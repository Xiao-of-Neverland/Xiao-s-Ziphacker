#include "main.h"


int main(int argc, char * argv[])
{
	//初始化参数
	Options options;
	if(argc > 1) {
		options = init_options(argc, argv);
		fmt::println("Isvalid: {}", options.ifValid);
		if(options.ifDirMode) {
			fmt::println("Mode: directory");
			fmt::println("Directory: {}", options.dirPath.generic_u8string());
		} else {
			fmt::println("Mode: archive");
			fmt::println("Archive: {}", options.targetPath.generic_u8string());
		}
		fmt::println("Charset: {}", options.charSet);
		fmt::println("Len range: {} - {}", options.minPasswordLen, options.maxPasswordLen);
		fmt::println("Thread cnt: {} (\"0\" means use prog setting)", options.threadCnt);
		if(!options.ifValid) {
			fmt::println("Invalid options, use '-h' to get help info");
			return 1;
		}
	} else {
		fmt::println("Need options, use '-h' to get help info");
		return 1;
		//debug options
		options.targetPath = std::filesystem::u8path("D:/VS2022/Xiao-s-Ziphacker/test2.zip");
		options.targetPath = std::filesystem::u8path("D:/VS2022/Xiao-s-Ziphacker/中文测试.zip");
		options.dirPath = std::filesystem::u8path("D:/VS2022/Xiao-s-Ziphacker/test");
		options.ifDirMode = false;
		options.charSet.append(numbers).append(uppers).append(lowers);
		options.minPasswordLen = 1;
		options.maxPasswordLen = 4;
		options.threadCnt = 10;
		options.ifValid = true;

		if(std::filesystem::exists(options.targetPath)) {
			fmt::println("Debug path exist");
		} else {
			fmt::println("Debug path NOT exist");
		}
	}

	//检查libmagic数据库文件是否存在
	try {
		auto prog_path = std::filesystem::current_path();
		auto magic_db_path = prog_path / "magic.mgc";
		if(!std::filesystem::exists(magic_db_path)) {
			fmt::println("-- Error: can't find magic database file: magic.mgc --");
			return 1;
		}
	} catch(const std::filesystem::filesystem_error & err) {
		std::cerr << "-- File system error: " << err.what() << " --" << std::endl;
		return 1;
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

	std::vector<std::filesystem::path> zip_path_vector;
	if(options.ifDirMode) {
		try {
			for(const auto & file_entry : std::filesystem::directory_iterator(options.dirPath)) {
				if(file_entry.is_regular_file()) {
					const std::filesystem::path & file_path = file_entry.path();
					auto extension = file_path.extension().generic_u8string();
					if(extension == ".zip" || extension == ".ZIP") {
						zip_path_vector.push_back(file_path);
					}
				}
			}
		} catch(const std::filesystem::filesystem_error & err) {
			std::cerr << "-- File system error: " << err.what() << " --" << std::endl;
			return 1;
		}
	} else {
		zip_path_vector.push_back(options.targetPath);
	}

	std::ofstream output_file;
	if(options.ifDirMode) {
		output_file.open(options.dirPath / output_file_name);
	}
	for(size_t i = 0; i < zip_path_vector.size(); ++i) {
		//当前zip文档信息
		auto zip_path = zip_path_vector[i];
		auto zip_path_u8str = zip_path_vector[i].generic_u8string();
		fmt::println("\nCurrent: {} of {} archive(s)", i + 1, zip_path_vector.size());
		fmt::println("Zip path: {}", zip_path_u8str);
		output_file << "Zip:" << zip_path_u8str << std::endl;

		//初始化线程共享资源
		auto shared_resources = init_shared_resources(zip_path.generic_string());
		if(!shared_resources.ifValid) {
			fmt::println("-- Error: Failed to init shared resources. Please check last error --");
			return 1;
		}

		auto file_index = get_file_index(shared_resources);
		if(file_index < 0) {
			return 1;
		}

		//记录启动时间
		auto start_time = timer::now();

		//创建线程
		std::vector<std::thread> worker_thread_vector;
		for(int thread_id = 0; thread_id < thread_cnt; ++thread_id) {
			worker_thread_vector.emplace_back(
				thread_worker_function,
				thread_id,
				thread_cnt,
				shared_resources,
				file_index,
				options
			);
		}

		wait_worker(options, shared_resources, start_time, worker_thread_vector);

		print_result_info(options, start_time);
		if(options.ifDirMode) {
			output_file << "Password:" << password << '\n' << std::endl;
		}
	}
	if(options.ifDirMode) {
		output_file.close();
		fmt::print("\nAll archive(s) done, see passwords in file: ");
		fmt::println("{}", (options.dirPath / output_file_name).generic_u8string());
	}

	return 0;
}

int get_file_index(SharedResources shared_resources)
{
	//初始化zip文档
	auto zip_archive = pre_init_zip_archive(shared_resources);
	if(!zip_archive.IfValid()) {
		fmt::println("-- Error: Failed to open ZIP archive --");
		return -1;
	}

	//检查zip文档内文件条目
	auto file_cnt = zip_get_num_entries(zip_archive.Get(), 0);
	if(file_cnt < 1) {
		fmt::println("-- Error: ZIP archive have no file --");
		return -1;
	}

	zip_uint64_t encrypted_file_index = -1;
	zip_stat_t file_stat;
	zip_stat_init(&file_stat);
	for(size_t i = 0; i < file_cnt; ++i) {
		zip_stat_index(zip_archive.Get(), i, 0, &file_stat);
		if(file_stat.valid & ZIP_STAT_ENCRYPTION_METHOD) {
			if(file_stat.encryption_method != ZIP_EM_NONE) {
				fmt::println(
					"File name: {}, Comp method: {}, Encryp method: {}",
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
		return -1;
	}

	return encrypted_file_index;
}

void wait_worker(
	Options & options,
	SharedResources & shared_resources,
	time_point & start_time,
	std::vector<std::thread> & worker_thread_vector
)
{
	uint64_t try_cnt_max = 0;
	for(size_t i = options.minPasswordLen; i <= options.maxPasswordLen; ++i) {
		try_cnt_max += pow(options.charSet.length(), i);
	}
	while(!if_password_found) {
		uint64_t try_cnt_ob = index_ob;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		for(size_t i = options.minPasswordLen; i < password_len_ob; ++i) {
			try_cnt_ob += pow(options.charSet.length(), i);
		}
		print_progress(try_cnt_ob, try_cnt_max, start_time);
		if(running_thread_cnt < 1) {
			print_progress(try_cnt_ob, try_cnt_max, start_time);
			break;
		}
	}
	fmt::print("\n");

	//等待线程终止
	for(auto & worker_thread : worker_thread_vector) {
		worker_thread.join();
	}
}

void print_progress(uint64_t try_cnt_ob, uint64_t try_cnt_max, time_point start_time)
{
	float percentage = (float)try_cnt_ob / try_cnt_max * 100;
	if(percentage > 100.0 || running_thread_cnt < 1) {
		percentage = 100.0;
	}

	auto current_time = timer::now();
	auto time_cost_sec = std::chrono::duration_cast<std::chrono::seconds>(
		current_time - start_time
	).count();
	long long time_need_sec = (float)time_cost_sec / percentage * (100 - percentage);

	auto time_cost_hour = time_cost_sec / 3600;
	time_cost_sec %= 3600;
	auto time_cost_minute = time_cost_sec / 60;
	time_cost_sec %= 60;

	auto time_need_hour = time_need_sec / 3600;
	time_need_sec %= 3600;
	auto time_need_minute = time_need_sec / 60;
	time_need_sec %= 60;

	std::string time_cost_str(" ");
	if(time_cost_hour > 0) {
		time_cost_str += fmt::format("{}h", time_cost_hour);
	}
	if(time_cost_minute > 0) {
		time_cost_str += fmt::format("{}m", time_cost_minute);
	}
	if(time_cost_sec > 0) {
		time_cost_str += fmt::format("{}s", time_cost_sec);
	}

	std::string time_need_str(" ");
	if(percentage >= 100.0) {
		time_need_str.append("Done");
	} else if(time_need_hour >= 24){
		time_need_str.append("More than one day");
	} else {
		if(time_need_hour > 0) {
			time_need_str += fmt::format("{}h", time_need_hour);
		}
		if(time_need_minute > 0) {
			time_need_str += fmt::format("{}m", time_need_minute);
		}
		if(time_need_sec > 0) {
			time_need_str += fmt::format("{}s", time_need_sec);
		}
	}

	int bar_filled = progress_bar_width * percentage / 100;
	std::string bar(progress_bar_width, ' ');
	for(size_t i = 0; i < progress_bar_width; ++i) {
		if(i < bar_filled) {
			bar.at(i) = '=';
		}
	}
	std::cout << fmt::format(
		"\r[{}] {:.0f}% | Time cost:{} | Time need:{}",
		bar,
		percentage,
		time_cost_str,
		time_need_str
	);
	std::cout.flush();
}

void print_result_info(Options & options, time_point start_time)
{
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
}
