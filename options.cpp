#include "options.h"

std::string numbers = "0123456789";
std::string uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string lowers = "abcdefghijklmnopqrstuvwxyz";

Options init_options(int & argc, char * argv[])
{
	Options options;

	bool if_have_numbers = false;
	bool if_have_uppers = false;
	bool if_have_lowers = false;
	std::string utf8_path;

	for(size_t i = 1; i < argc; i++) {
		std::string_view arg(argv[i]);
		if(arg == "-n" || arg == "-N") {
			if_have_numbers = true;
		} else if(arg == "-u" || arg == "-U") {
			if_have_uppers = true;
		} else if(arg == "-l" || arg == "-L") {
			if_have_lowers = true;
		} else if(arg == "-t" || arg == "-T") {
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need target file path --");
				return{};
			}
			i++;
			std::string_view raw_path = argv[i];
			options.targetPath = init_target_path(raw_path);
		} else if(arg == "-r" || arg == "-R") {
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need password length range --");
				return{};
			}
		}
	}

	if(options.targetPath.u8string().empty()) {
		return{};
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

	options.isValid = true;
	return options;
}

std::string gbk_to_utf8(const char * gbk_str)
{
	// 将 GBK 转换为 UTF-16
	int size = MultiByteToWideChar(CP_ACP, 0, gbk_str, -1, nullptr, 0);
	if(size == 0) {
		return "";
	}
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, gbk_str, -1, wstr.data(), size);

	// 将 UTF-16 转换为 UTF-8
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8_str(utf8_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, utf8_str.data(), utf8_size, nullptr, nullptr);
	return utf8_str;
}

std::filesystem::path init_target_path(std::string_view & raw_path)
{
	auto utf8_path = gbk_to_utf8(raw_path.data());
	if(utf8_path.size() > 1 && utf8_path.front() == '"' && utf8_path.back() == '"') {
		utf8_path = utf8_path.substr(1, utf8_path.size() - 2);
	}
	if(utf8_path.find_first_of("<>:\"|?*\0") != std::string::npos) {
		fmt::println("-- Error: Invalid target file path --");
		return "";
	}
	auto target_path = std::filesystem::u8path(utf8_path);
	if(!std::filesystem::exists(target_path)) {
		fmt::println("-- Error: Invalid target file path --");
		return "";
	}
	auto path_extension = target_path.extension().u8string();
	if(path_extension != ".zip" && path_extension != ".ZIP") {
		fmt::println("-- Error: Target file is not ZIP file --");
		return "";
	}

	return target_path;
}
