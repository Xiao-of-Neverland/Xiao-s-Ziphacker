#include "options.h"
#include "resources.h"
#include "multithread.h"

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
		options.isValid = true;
	}
	
	auto shared_resources = init_shared_resources(options.targetPath.u8string());

	auto zip_archive = init_zip_archive(shared_resources);

	for(size_t i = 0; i < 10; i++) {
		auto index_range = init_index_range(i, 10, 27, 4);
		fmt::println("{}: [{}, {}]", i, index_range.first, index_range.second);
	}
	fmt::println("Total: {}", pow(27, 4));

	return 0;
}
