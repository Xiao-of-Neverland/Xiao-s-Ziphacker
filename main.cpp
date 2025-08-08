#include "options.h"
#include "resources.h"
#include "multithread.h"

std::string password;
bool if_password_found = false;

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
		options.max_password_len = 4;
		options.isValid = true;
	}
	
	auto shared_resources = init_shared_resources(options.targetPath.generic_string());

	thread_worker_function(0, 1, shared_resources, options);

	if(if_password_found) {
		fmt::println("{}", password);
	}

	return 0;
}
