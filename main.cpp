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
		options.targetPath = std::filesystem::u8path("D:\\VS2022\\Xiao's Ziphacker\\test.zip");
		options.charSet.append(numbers).append(uppers).append(lowers);
		options.minPasswordLen = 1;
		options.maxPasswordLen = 4;
		options.threadCnt = 10;
		options.isValid = true;
	}
	
	//记录启动时间
	auto timer_start = std::chrono::high_resolution_clock::now();
	int thread_cnt = options.threadCnt;
	if(thread_cnt == 0) {
		auto logical_processor_cnt = std::thread::hardware_concurrency();
		if(logical_processor_cnt > 0) {
			thread_cnt = (logical_processor_cnt + 1) / 2;
		} else {
			thread_cnt = 1;
			fmt::println("Warnning: Can't get count of logical processor. Using only ONE thread");
		}
	}

	//初始化线程共享资源
	auto shared_resources = init_shared_resources(options.targetPath.generic_string());

	//创建线程
	std::vector<std::thread> worker_threads;
	for(int thread_id = 0; thread_id < thread_cnt; ++thread_id) {
		worker_threads.emplace_back(
			thread_worker_function,
			thread_id,
			thread_cnt,
			shared_resources,
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
		}
		show_progress(try_cnt_ob, try_cnt_max);
		if(try_cnt_ob >= try_cnt_max) {
			break;
		}
	} while(!if_password_found);
	fmt::print("\n");

	//等待线程终止
	for(auto & worker_thread : worker_threads) {
		worker_thread.join();
	}

	//记录终止时间
	auto timer_end = std::chrono::high_resolution_clock::now();
	auto time_cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		timer_end - timer_start
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

void show_progress(uint64_t try_cnt_ob, uint64_t try_cnt_max, int bar_width)
{
	float percentage = (float)try_cnt_ob / try_cnt_max * 100;
	if(percentage > 100.0) {
		percentage = 100;
	}
	int bar_filled = bar_width * try_cnt_ob / try_cnt_max;
	std::string bar(bar_width, ' ');
	for(size_t i = 0; i < bar_width; ++i) {
		if(i < bar_filled) {
			bar.at(i) = '=';
		}
	}
	//no bug with cout
	std::cout << "\r[" << bar << "] " << int(percentage) << '%';
	//bug with fmt
	//std::cout << fmt::format("\r[{}] {:.0f}%\n", bar, percentage);
	std::cout.flush();
}