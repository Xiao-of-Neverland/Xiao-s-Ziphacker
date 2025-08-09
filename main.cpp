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
			return 1;
		}
	} else {
		options.targetPath = std::filesystem::u8path("D:\\VS2022\\Xiao's Ziphacker\\test.zip");
		options.charSet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		options.min_password_len = 1;
		options.max_password_len = 2;
		options.isValid = true;
	}
	
	auto timer_start = std::chrono::high_resolution_clock::now();

	auto shared_resources = init_shared_resources(options.targetPath.generic_string());

	thread_worker_function(0, 1, shared_resources, options);

	auto timer_end = std::chrono::high_resolution_clock::now();

	auto time_cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		timer_end - timer_start
	).count();

	if(if_password_found) {
		fmt::println("Password: {}", password);
		fmt::println("Time cost: {} ms", time_cost_ms);
		uint64_t try_cnt = index_when_found - start_index_when_found;
		for(size_t i = options.min_password_len; i < password.length(); i++) {
			try_cnt += pow(options.charSet.length(), i);
		}
		fmt::println("Count of try passwords: {}", try_cnt);
		if(time_cost_ms >= 1000) {
			fmt::println("count of trys per sec: {}", try_cnt / (time_cost_ms / 1000));
		}
	} else {
		fmt::println("Password not found");
	}

	return 0;
}
