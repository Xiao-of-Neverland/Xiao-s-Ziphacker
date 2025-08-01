#include "options.h"
#include "resources.h"
#include "multithread.h"

int main(int argc, char * argv[])
{
	Options options;
	if(argc > 2) {
		options = init_options(argc, argv);
	} else {
		options.targetPath = std::filesystem::u8path("D:\\VS2022\\Xiao's Ziphacker\\test.zip");
		options.isValid = true;
	}
	
	auto shared_resources = init_shared_resources(options.targetPath.u8string());

	auto p_zip_archive = init_zip_archive(shared_resources);

	return 0;
}
