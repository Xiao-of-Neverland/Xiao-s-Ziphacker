#include "options.h"

std::string numbers = "0123456789";
std::string uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string lowers = "abcdefghijklmnopqrstuvwxyz";

Options init_options(int & argc, char * argv[])
{
	Options options;

	bool if_allocate_charset = false;
	bool if_allocate_path = false;
	bool if_allocate_range = false;

	bool if_have_numbers = false;
	bool if_have_uppers = false;
	bool if_have_lowers = false;

	for(size_t i = 1; i < argc; ++i) {
		std::string_view arg(argv[i]);
		if(arg == "-h" || arg == "-H") {
			print_help();
			return options;
		} if(arg == "-n" || arg == "-N") {
			if_have_numbers = true;
			if_allocate_charset = true;
		} else if(arg == "-u" || arg == "-U") {
			if_have_uppers = true;
			if_allocate_charset = true;
		} else if(arg == "-l" || arg == "-L") {
			if_have_lowers = true;
			if_allocate_charset = true;
		} else if(arg == "-t" || arg == "-T") {
			if(if_allocate_path) {
				fmt::println("-- Error: Multiple allocate target file path --");
				return options;
			}
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need target file path --");
				return options;
			}
			++i;
			std::string_view raw_path = argv[i];
			options.targetPath = init_target_path(raw_path);
			if(options.targetPath.generic_string().empty()) {
				return options;
			}
			if_allocate_path = true;
		} else if(arg == "-r" || arg == "-R") {
			if(if_allocate_range) {
				fmt::println("-- Error: Multiple allocate password len range --");
				return options;
			}
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need password length range --");
				return options;
			}
			++i;
			auto password_len_pair = init_password_len_range(i, argc, argv);
			if(password_len_pair.first < 1) {
				return options;
			}
			options.min_password_len = password_len_pair.first;
			options.max_password_len = password_len_pair.second;
			if_allocate_range = true;
		} else {
			fmt::println("-- Warnning: Ignore invalid parameter \"{}\"", arg);
		}
	}

	if(if_have_numbers) {
		options.charSet.append(numbers);
	}
	if(if_have_uppers) {
		options.charSet.append(uppers);
	}
	if(if_have_lowers) {
		options.charSet.append(lowers);
	}

	if(!if_allocate_charset) {
		fmt::println("-- Error: Need allocate password charset --");
	}
	if(!if_allocate_path) {
		fmt::println("-- Error: Need allocate target file path --");
	}
	if(!if_allocate_range) {
		fmt::println("-- Error: Need allocate password len range --");
	}

	if(if_allocate_charset && if_allocate_path && if_allocate_range) {
		options.isValid = true;
	}

	return options;
}

void print_help()
{
	fmt::println("usage: XZP [char set option(s)] [target path] [password len range]\n");

	fmt::println("char set option(s):");
	fmt::println("\t[-n | -N]\tadd all numbers to char set");
	fmt::println("\t[-u | -U]\tadd all upper case letters to char set");
	fmt::println("\t[-l | -L]\tadd all lower case letters to char set");
	fmt::println("these char set option(s) need at least one, repeated will be ignore\n");

	fmt::println("target path:");
	fmt::println("\t[-t | -T]\ttell prog to get target path");
	fmt::println("\t[PATH | \"PATH\"]\ttarget path, have to be exist, valid and end with \".zip\"");
	fmt::println("[-t | -T] must be followed with [PATH | \"PATH\"], both need one and only one\n");

	fmt::println("password len range:");
	fmt::println("\t[-r | -R]\ttell prog to get password len range");
	fmt::println("\t[RANGE]\tpassword len range, have to be two positove int, example:");
	fmt::println("\t\t\t[MIN,MAX] [MIN MAX] [MIN-MAX]");
	fmt::println("[-r | -R] must be followed with [RANGE], both need one and only one\n");
}

std::string gbk_to_utf8(const char * gbk_str)
{
	// 将 GBK 转换为 UTF-16
	auto size = MultiByteToWideChar(CP_ACP, 0, gbk_str, -1, nullptr, 0);
	if(size == 0) {
		return "";
	}
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, gbk_str, -1, wstr.data(), size);

	// 将 UTF-16 转换为 UTF-8
	auto utf8_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8_str(utf8_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, utf8_str.data(), utf8_size, nullptr, nullptr);
	return utf8_str;
}

std::filesystem::path init_target_path(std::string_view & raw_path)
{
	//编码转换
	auto utf8_path = gbk_to_utf8(raw_path.data());
	utf8_path.pop_back();

	//检查是否包含非法字符
	if(utf8_path.find_first_of("<>\"|?*\0") != std::string::npos) {
		fmt::println("-- Error: Invalid target file path --");
		return "";
	}

	//检查文件是否存在
	auto target_path = std::filesystem::u8path(utf8_path);
	if(!std::filesystem::exists(target_path)) {
		fmt::println("-- Error: Invalid target file path, not exist --");
		return "";
	}

	//检查文件拓展名
	auto path_extension = target_path.extension().generic_string();
	if(path_extension != ".zip" && path_extension != ".ZIP") {
		fmt::println("-- Error: Target file is not ZIP file --");
		return "";
	}

	return target_path;
}

std::pair<int, int> init_password_len_range(size_t & i, int & argc, char * argv[])
{
	int first, second;
	std::string_view arg1 = argv[i];
	auto pos_sep = arg1.find_first_of(",-");
	if(pos_sep != std::string_view::npos) {
		++i;
		auto result1 = std::from_chars(arg1.data(), arg1.data() + pos_sep, first);
		auto result2 = std::from_chars(arg1.data() + pos_sep + 1, arg1.data() + arg1.size(), second);
		if(result1.ec != std::errc{} || result2.ec != std::errc{}) {
			fmt::println("-- Error: Invalid password len range --");
			return{0, 0};
		}
	} else if(i + 2 <= argc) {
		std::string_view arg2 = argv[i + 1];
		i += 2;
		auto result1 = std::from_chars(arg1.data(), arg1.data() + arg1.size(), first);
		auto result2 = std::from_chars(arg2.data(), arg2.data() + arg2.size(), second);
		if(result1.ec != std::errc{} || result2.ec != std::errc{}) {
			fmt::println("-- Error: Invalid password len range, one input invalid --");
			return{0, 0};
		}
	} else {
		fmt::println("-- Error: Invalid password len range, need two input --");
		return{0, 0};
	}

	if(!(0 < first && first <= second && second <= UINT8_MAX)) {
		fmt::println("-- Error: Invalid password len range --");
		return{0, 0};
	}

	return {first, second};
}
