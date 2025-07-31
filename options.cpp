#include "options.h"

Options init_options(int & argc, char * argv[])
{
	Options options;

	bool if_have_numbers = false;
	bool if_have_uppers = false;
	bool if_have_lowers = false;
	std::filesystem::path target_path;

	for(size_t i = 0; i < argc; i++) {
		std::string_view arg(argv[i]);
		if(arg == "-n" || arg == "-N") {
			if_have_numbers = true;
		} else if(arg == "-u" || arg == "-U") {
			if_have_uppers = true;
		} else if(arg == "-l" || arg == "-L") {
			if_have_lowers = true;
		} else if(arg == "-t" || arg == "-T") {
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need target file path! --");
				return{};
			}
			i++;
			std::string_view raw_path = argv[i];
		}
	}

	return options;
}
