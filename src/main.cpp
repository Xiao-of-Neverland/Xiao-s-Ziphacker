#include "main.h"


int main(int argc, char * argv[])
{
	//初始化参数
	Options options;
	if(argc > 1) {
		options = init_options(argc, argv);
		fmt::println("Isvalid: {}", options.isValid);
		fmt::println("Path: {}", options.targetPath.u8string());
		fmt::println("Charset: {}", options.charSet);
		fmt::println("Len range: {} - {}", options.minPasswordLen, options.maxPasswordLen);
		fmt::println("Thread cnt: {} (\"0\" means use prog setting)", options.threadCnt);
		if(!options.isValid) {
			fmt::println("Invalid options, use '-h' to get help info");
			return 1;
		}
	} else {
		fmt::println("Need options, use '-h' to get help info");
		//return 1;
		//debug options
		options.targetPath = std::filesystem::u8path("D:\\VS2022\\Xiao-s-Ziphacker\\test.zip");
		options.charSet.append(numbers).append(uppers).append(lowers);
		options.minPasswordLen = 1;
		options.maxPasswordLen = 4;
		options.threadCnt = 10;
		options.isValid = true;
	}

	//初始化线程数量
	int thread_cnt = options.threadCnt;
	if(thread_cnt == 0) {
		auto logical_processor_cnt = std::thread::hardware_concurrency();
		if(logical_processor_cnt > 0) {
			thread_cnt = (logical_processor_cnt + 1) / 2;
		} else {
			thread_cnt = 1;
			fmt::println("-- Warnning: Can't get count of logical processor --");
		}
	}

	//初始化线程共享资源
	auto shared_resources = init_shared_resources(options.targetPath.generic_string());

	//初始化zip文档
	auto zip_archive = init_zip_archive(shared_resources);
	if(!zip_archive.IsValid()) {
		fmt::println("-- Error: Failed to open ZIP archive --");
		return 1;
	}

	//检查zip文档内文件条目
	auto zip_entries_cnt = zip_get_num_entries(zip_archive.Get(), 0);
	if(zip_entries_cnt < 1) {
		fmt::println("-- Error: ZIP archive have no file --");
		return 1;
	}
	zip_uint64_t encrypted_file_index = -1;
	zip_stat_t file_stat;
	zip_stat_init(&file_stat);
	for(size_t i = 0; i < zip_entries_cnt; ++i) {
		zip_stat_index(zip_archive.Get(), i, 0, &file_stat);
		if(file_stat.valid & ZIP_STAT_ENCRYPTION_METHOD) {
			if(file_stat.encryption_method != ZIP_EM_NONE) {
				fmt::println(
					"File name: {}, Comp method:{}, Encryp method: {}",
					file_stat.name,
					file_stat.comp_method,
					file_stat.encryption_method
				);
				encrypted_file_index = i;
				break;
			}
		}
	}
	if(encrypted_file_index == -1) {
		fmt::println("-- Error: ZIP archive have no encrypted file --");
		return 1;
	}

	//记录启动时间
	auto start_time = timer::now();

	//创建线程
	std::vector<std::thread> worker_threads;
	for(int thread_id = 0; thread_id < thread_cnt; ++thread_id) {
		worker_threads.emplace_back(
			thread_worker_function,
			thread_id,
			thread_cnt,
			shared_resources,
			encrypted_file_index,
			options
		);
	}

	//打印进度条。包含任务终止判定
	uint64_t try_cnt_max = 0;
	for(size_t i = options.minPasswordLen; i <= options.maxPasswordLen; ++i) {
		try_cnt_max += pow(options.charSet.length(), i);
	}
	do {
		uint64_t try_cnt_ob = index_ob;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		for(size_t i = options.minPasswordLen; i < password_len_ob; ++i) {
			try_cnt_ob += pow(options.charSet.length(), i);
		}if(shared_resources.pMapView.use_count() <= 1) {
			try_cnt_ob = try_cnt_max;
		}
		print_progress(try_cnt_ob, try_cnt_max, start_time);
		if(shared_resources.pMapView.use_count() <= 1) {
			break;
		}
	} while(!if_password_found);
	fmt::print("\n");

	//等待线程终止
	for(auto & worker_thread : worker_threads) {
		worker_thread.join();
	}

	//记录终止时间
	auto end_time = timer::now();
	auto time_cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		end_time - start_time
	).count();

	//计算破解速度
	int password_len_all_try = options.maxPasswordLen;
	uint64_t try_cnt = index_when_found;
	if(if_password_found) {
		++try_cnt;
		password_len_all_try = password.length() - 1;
	}
	for(size_t i = options.minPasswordLen; i <= password_len_all_try; ++i) {
		try_cnt += pow(options.charSet.length(), i);
	}

	//输出破解速度
	fmt::println("Time cost: {} ms", time_cost_ms);
	fmt::println("Count of try passwords: {}", try_cnt);
	double trys_per_sec = try_cnt / ((double)time_cost_ms / 1000);
	fmt::println("Count of trys per sec: {:.0f}", trys_per_sec);

	//输出是否破解成功及破解密码
	if(if_password_found) {
		fmt::println("Password found: \"{}\"", password);
	} else {
		fmt::println("Password not found");
	}

	return 0;
}

void print_progress(uint64_t try_cnt_ob, uint64_t try_cnt_max, time_point start_time)
{
	auto current_time = timer::now();
	auto time_cost_sec = std::chrono::duration_cast<std::chrono::seconds>(
		current_time - start_time
	).count();
	float percentage = (float)try_cnt_ob / try_cnt_max * 100;
	if(percentage > 100.0) {
		percentage = 100.0;
	}
	int bar_filled = progress_bar_width * try_cnt_ob / try_cnt_max;
	std::string bar(progress_bar_width, ' ');
	for(size_t i = 0; i < progress_bar_width; ++i) {
		if(i < bar_filled) {
			bar.at(i) = '=';
		}
	}
	std::cout << fmt::format("\r[{}] {:.0f}%", bar, percentage);
	std::cout.flush();
}