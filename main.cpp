#include "options.h"
#include "resources.h"
#include "multithread.h"
#include <chrono>

std::string password;
bool if_password_found = false;

uint64_t start_index_when_found;
uint64_t end_index_when_found;
uint64_t index_when_found;

int main(int argc, char * argv[])
{
	Options options;
	if(argc > 1) {
		options = init_options(argc, argv);
		fmt::println("Isvalid: {}", options.isValid);
		fmt::println("Path: {}", options.targetPath.u8string());
		fmt::println("Charset: {}", options.charSet);
		fmt::println("Len range: {} - {}", options.min_password_len, options.max_password_len);
		if(!options.isValid) {
			fmt::println("Invalid options, use '-h' to get help info");
			return 1;
		}
	} else {
		fmt::println("Need options, use '-h' to get help info");
		options.targetPath = std::filesystem::u8path("D:\\VS2022\\Xiao's Ziphacker\\test.zip");
		options.charSet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		options.min_password_len = 1;
		options.max_password_len = 4;
		options.isValid = true;
	}
	
	auto timer_start = std::chrono::high_resolution_clock::now();

	int thread_cnt = 0;
	auto logical_processor_cnt = std::thread::hardware_concurrency();
	if(logical_processor_cnt > 0) {
		thread_cnt = (logical_processor_cnt + 1) / 2;
	} else {
		thread_cnt = 1;
		fmt::println("Warnning: Can't get count of logical processor. Using only ONE thread");
	}

	auto shared_resources = init_shared_resources(options.targetPath.generic_string());

	for(int thread_id = 0; thread_id < thread_cnt; ++thread_id) {
		std::thread worker_thread(
			thread_worker_function,
			thread_id,
			thread_cnt,
			shared_resources,
			options
		);
		worker_thread.join();
	}

	auto timer_end = std::chrono::high_resolution_clock::now();

	auto time_cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		timer_end - timer_start
	).count();

	if(if_password_found) {
		fmt::println("Worker thread cnt: {}", thread_cnt);
		fmt::println("Password: {}", password);
		fmt::println("Time cost: {} ms", time_cost_ms);
		uint64_t try_cnt = (index_when_found - start_index_when_found) * thread_cnt;
		for(size_t i = options.min_password_len; i < password.length(); i++) {
			try_cnt += pow(options.charSet.length(), i);
		}
		fmt::println("Count of try passwords: {}", try_cnt);
		double trys_per_sec = try_cnt / ((double)time_cost_ms / 1000);
		if(time_cost_ms >= 1000) {
			fmt::println("Count of trys per sec: {:.0f}", trys_per_sec);
		}
	} else {
		fmt::println("Password not found");
	}

	return 0;
}
