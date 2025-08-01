#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <fmt/core.h>
#include <Windows.h>


struct Options
{
	bool isValid = false;
	bool ifMultipleTarget = false;
	std::filesystem::path targetPath;
	std::string charSet;
};


Options init_options(int & argc, char * argv[]);

std::string gbk_to_utf8(const char * gbk_str);

std::filesystem::path init_target_path(std::string_view & raw_path);

std::pair<int, int> init_password_length_range(std::string_view & raw);