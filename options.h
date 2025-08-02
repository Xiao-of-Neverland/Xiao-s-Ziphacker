#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <charconv>
#include <filesystem>
#include <fmt/core.h>
#include <Windows.h>


//结构体：用户输入参数
struct Options
{
	bool isValid = false;
	std::filesystem::path targetPath;
	std::string charSet;
	int min_password_len = 0;
	int max_password_len = 0;
};


//初始化输入参数
Options init_options(int & argc, char * argv[]);

//编码转换
std::string gbk_to_utf8(const char * gbk_str);

//初始化目标文件路径。适用于windows
std::filesystem::path init_target_path(std::string_view & raw_path);

//初始化密码长度范围
std::pair<int, int> init_password_len_range(size_t & i, int & argc, char * argv[]);

