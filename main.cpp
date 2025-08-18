#include "options.h"
#include "resources.h"
#include "multithread.h"
#include <chrono>


int main(int argc, char * argv[])
{
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
		return 1;
		//开发调试用
		//options.targetPath = std::filesystem::u8path("D:\\VS2022\\Xiao's Ziphacker\\test.zip");
		//options.charSet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		//options.minPasswordLen = 1;
		//options.maxPasswordLen = 4;
		//options.threadCnt = 4;
		//options.isValid = true;
	}
	
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

	auto shared_resources = init_shared_resources(options.targetPath.generic_string());

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

	for(auto & worker_thread : worker_threads) {
		worker_thread.join();
	}

	auto timer_end = std::chrono::high_resolution_clock::now();

	auto time_cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		timer_end - timer_start
	).count();

	fmt::println("Worker thread cnt: {}", thread_cnt);

	int password_len_all_try = options.maxPasswordLen;
	uint64_t try_cnt = index_when_found;
	if(if_password_found) {
		++try_cnt;
		password_len_all_try = password.length() - 1;
	}
	for(size_t i = options.minPasswordLen; i <= password_len_all_try; ++i) {
		try_cnt += pow(options.charSet.length(), i);
	}

	fmt::println("Time cost: {} ms", time_cost_ms);
	fmt::println("Count of try passwords: {}", try_cnt);
	double trys_per_sec = try_cnt / ((double)time_cost_ms / 1000);
	fmt::println("Count of trys per sec: {:.0f}", trys_per_sec);

	if(if_password_found) {
		fmt::println("Password found: \"{}\"", password);
	} else {
		fmt::println("Password not found");
	}

	return 0;
}

void show_progress()
{
	float percentage = (float)(1);
}