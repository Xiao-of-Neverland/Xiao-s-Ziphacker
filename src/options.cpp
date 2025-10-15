#include "options.h"


Options init_options(int & argc, char * argv[])
{
	Options options;

	bool if_allocate_charset = false;
	bool if_allocate_archive_path = false;
	bool if_allocate_dir_path = false;
	bool if_allocate_range = false;
	bool if_allocate_thread = false;

	bool if_have_numbers = false;
	bool if_have_uppers = false;
	bool if_have_lowers = false;
	bool if_have_signs = false;

	for(size_t i = 1; i < argc; ++i) {
		std::string_view arg(argv[i]);
		if(arg == "-h" || arg == "-H") {
			print_help();
			return options;
		} else if(arg == "-n" || arg == "-N") {
			if_have_numbers = true;
			if_allocate_charset = true;
		} else if(arg == "-u" || arg == "-U") {
			if_have_uppers = true;
			if_allocate_charset = true;
		} else if(arg == "-l" || arg == "-L") {
			if_have_lowers = true;
			if_allocate_charset = true;
		} else if(arg == "-s" || arg == "-S") {
			if_have_signs = true;
			if_allocate_charset = true;
		} else if(arg == "-a" || arg == "-A") {
			if(if_allocate_archive_path) {
				fmt::println("-- Error: Multiple allocate archive path --");
				return options;
			}
			if(if_allocate_dir_path) {
				fmt::println("-- Error: [archive path] is disabled when [dir path] was given --");
				return options;
			}
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need archive path --");
				return options;
			}
			++i;
			std::string_view raw_path = argv[i];
			options.archivePath = init_archive_path(raw_path);
			if(options.archivePath.generic_u8string().empty()) {
				return options;
			}
			if_allocate_archive_path = true;
		} else if(arg == "-d" || arg == "-D") {
			if(if_allocate_dir_path) {
				fmt::println("-- Error: Multiple allocate dir path --");
				return options;
			}
			if(if_allocate_archive_path) {
				fmt::println("-- Error: [dir path] is disabled when [archive path] was given --");
				return options;
			}
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need dir path --");
				return options;
			}
			++i;
			std::string_view raw_path = argv[i];
			options.dirPath = init_dir_path(raw_path);
			if(options.dirPath.generic_u8string().empty()) {
				return options;
			}
			if_allocate_dir_path = true;
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
			options.minPasswordLen = password_len_pair.first;
			options.maxPasswordLen = password_len_pair.second;
			if_allocate_range = true;
		} else if(arg == "-m" || arg == "-M") {
			if(if_allocate_thread) {
				fmt::println("-- Error: Multiple allocate multithread cnt --");
				return options;
			}
			if(i + 1 >= argc) {
				fmt::println("-- Error: Invalid arguments, need multithread cnt --");
				return options;
			}
			++i;
			std::string_view arg1 = argv[i];
			int cnt = 0;
			auto result1 = std::from_chars(arg1.data(), arg1.data() + arg1.size(), cnt);
			if(result1.ec != std::errc{} || cnt <= 0) {
				fmt::println("-- Error: Invalid multithread cnt --");
				return options;
			} else {
				options.threadCnt = cnt;
				if_allocate_thread = true;
			}
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
	if(!(if_allocate_archive_path || if_allocate_dir_path)) {
		fmt::println("-- Error: Need allocate archive or dir path --");
	}
	if(!if_allocate_range) {
		fmt::println("-- Error: Need allocate password len range --");
	}

	if(if_allocate_charset && if_allocate_archive_path && if_allocate_range) {
		options.ifValid = true;
	}

	return options;
}

void print_help()
{
	fmt::print("usage: XiaosZiphacker.exe [char set option(s)] [path]");
	fmt::println(" [password len range] [multithread cnt](optional)\n");

	fmt::println("char set option(s):");
	fmt::println("\t[-n | -N]\tadd all numbers to char set");
	fmt::println("\t[-u | -U]\tadd all upper case letters to char set");
	fmt::println("\t[-l | -L]\tadd all lower case letters to char set");
	fmt::println("\t[-s | -S]\tadd all other printable ASCII signs to char set");
	fmt::println("these char set option(s) need at least one, repeated will be ignore\n");

	fmt::println("path([archive path] OR [dir path]):");
	fmt::println("\tuse [archive path] to hack single archive, invalid with [dir path]");
	fmt::println("\tuse [dir path] to hack all zip archive in dir, invalid with [archive path]");

	fmt::println("archive path:");
	fmt::println("\t[-a | -A]\ttell prog to get archive path");
	fmt::println("\t[PATH | \"PATH\"]\tarchive path, have to be exist, valid and end with \".zip\"");
	fmt::println("[-a | -A] must be followed by [PATH | \"PATH\"], both need one and only one");

	fmt::println("dir path:");
	fmt::println("\t[-d | -D]\ttell prog to get dir path");
	fmt::println("\t[PATH | \"PATH\"]\tdir path, have to be exist and valid");
	fmt::println("[-d | -D] must be followed by [PATH | \"PATH\"], both need one and only one\n");

	fmt::println("password len range:");
	fmt::println("\t[-r | -R]\ttell prog to get password len range");
	fmt::println("\t[RANGE]\tpassword len range, have to be two positove int, example:");
	fmt::println("\t\t\t[MIN,MAX] [MIN MAX] [MIN-MAX]");
	fmt::println("[-r | -R] must be followed by [RANGE], both need one and only one\n");

	fmt::println("multithread cnt:");
	fmt::println("\t[-m | -M]\ttell prog to get thread cnt");
	fmt::println("\t[CNT]\tworker thread cnt, have to be positove int");
	fmt::println("[-m | -M] must be followed by [CNT], IF GIVED, both need one and only one");
	fmt::println("[multithread cnt] is optional, if not gived, prog will set it automatically\n");

	fmt::println("full example: XiaosZiphacker.exe -t \"D:/test.zip\" -n -u -l -r 1,4");
}

Encoding detect_encoding(const char * raw_cstr)
{
	size_t len = strlen(raw_cstr);
	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector * detector = ucsdet_open(&status);
	if(U_FAILURE(status)) {
		fmt::println("-- Failed to open detector: {} --", u_errorName(status));
		return {"", 0};
	}
	
	ucsdet_setText(detector, raw_cstr, len, &status);
	if(U_FAILURE(status)) {
		fmt::println("-- Failed to set text: {} --", u_errorName(status));
		ucsdet_close(detector);
		return {"", 0};
	}

	const UCharsetMatch * match = ucsdet_detect(detector, &status);
	if(U_FAILURE(status) || !match) {
		fmt::println("-- Detection failed: {} --", u_errorName(status));
		ucsdet_close(detector);
		return {"", 0};
	}

	const char * encoding = ucsdet_getName(match, &status);
	auto confidence = ucsdet_getConfidence(match, &status);
	ucsdet_close(detector);
	if(!encoding || !U_SUCCESS(status)) {
		return {"", 0};
	}

	return {encoding, confidence};
}

std::string convert_to_utf8(const char * raw_cstr)
{
	auto raw_len = strlen(raw_cstr);
	auto encoding = detect_encoding(raw_cstr);

	UErrorCode status = U_ZERO_ERROR;
	UConverter * cnv = ucnv_open(encoding.first.c_str(), &status);
	if(U_FAILURE(status)) {
		fmt::println("-- Failed to open converter --");
		return "";
	}

	int32_t uchar_len = ucnv_toUChars(cnv, nullptr, 0, raw_cstr, raw_len, &status);
	if(status != U_BUFFER_OVERFLOW_ERROR) {
		fmt::println("-- Failed to get Unicode len --");
		return "";
	}

	status = U_ZERO_ERROR;
	std::vector<UChar> buffer(uchar_len + 1);
	ucnv_toUChars(cnv, buffer.data(), uchar_len + 1, raw_cstr, raw_len, &status);
	if(U_FAILURE(status)) {
		fmt::println("-- Failed to convert to Unicode: {} --", u_errorName(status));
		ucnv_close(cnv);
		return "";
	}

	icu::UnicodeString ustr(buffer.data());
	std::string utf8_str;
	ustr.toUTF8String(utf8_str);
	ucnv_close(cnv);

	return utf8_str;
}

std::string gbk_to_utf8(const char * gbk_cstr)
{
	// 将 GBK 转换为 UTF-16
	auto size = MultiByteToWideChar(CP_ACP, 0, gbk_cstr, -1, nullptr, 0);
	if(size == 0) {
		return "";
	}
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, gbk_cstr, -1, wstr.data(), size);

	// 将 UTF-16 转换为 UTF-8
	auto utf8_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8_str(utf8_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, utf8_str.data(), utf8_size, nullptr, nullptr);
	utf8_str.pop_back();
	return utf8_str;
}

bool check_path(std::string & utf8_path)
{
	//检查是否包含非法字符
	if(utf8_path.find_first_of("<>\"|?*\0") != std::string::npos) {
		fmt::println("-- Error: Invalid archive path --");
		return false;
	}

	//检查路径是否存在
	try {
		auto fs_path = std::filesystem::u8path(utf8_path);
		if(!std::filesystem::exists(fs_path)) {
			fmt::println("-- Error: Invalid archive path, not exist --");
			return false;
		}
	} catch(const std::filesystem::filesystem_error & err) {
		std::cerr << "-- File system error: " << err.what() << " --" << std::endl;
		return false;
	}

	return true;
}

std::filesystem::path init_archive_path(std::string_view & raw_path)
{
	//编码转换
	auto utf8_path = gbk_to_utf8(raw_path.data());

	if(!check_path(utf8_path)) {
		return "";
	}

	//检查文件拓展名
	auto archive_path = std::filesystem::u8path(utf8_path);
	auto extension = archive_path.extension().generic_u8string();
	if(extension != ".zip" && extension != ".ZIP") {
		fmt::println("-- Error: archive is not ZIP --");
		return "";
	}

	return archive_path;
}

std::filesystem::path init_dir_path(std::string_view & raw_path)
{
	auto utf8_path = gbk_to_utf8(raw_path.data());

	if(!check_path(utf8_path)) {
		return "";
	}

	auto dir_path = std::filesystem::path(utf8_path);
	if(!std::filesystem::is_directory(dir_path)) {
		fmt::println("-- Error: Dir path is not a FOLDER --");
		return "";
	}

	return std::filesystem::path();
}

std::pair<int, int> init_password_len_range(size_t & i, int & argc, char * argv[])
{
	int first, second;
	std::string_view arg1 = argv[i];
	auto pos_sep = arg1.find_first_of(",-");
	if(pos_sep != std::string_view::npos) {
		auto result1 = std::from_chars(arg1.data(), arg1.data() + pos_sep, first);
		auto result2 = std::from_chars(arg1.data() + pos_sep + 1, arg1.data() + arg1.size(), second);
		if(result1.ec != std::errc{} || result2.ec != std::errc{}) {
			fmt::println("-- Error: Invalid password len range --");
			return{0, 0};
		}
	} else if(i + 2 <= argc) {
		std::string_view arg2 = argv[i + 1];
		++i;
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
