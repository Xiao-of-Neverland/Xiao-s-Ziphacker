#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <charconv>
#include <filesystem>
#include <fmt/core.h>
#include <Windows.h>
#include <iostream>

//字符集
inline std::string numbers = "0123456789";
inline std::string uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
inline std::string lowers = "abcdefghijklmnopqrstuvwxyz";
inline std::string signs = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

//结构体：用户输入参数
struct Options
{
	bool ifValid = false;
	bool ifDirMode = false;
	std::filesystem::path targetPath;
	std::filesystem::path dirPath;
	std::string charSet;
	int minPasswordLen = 0;
	int maxPasswordLen = 0;
	int threadCnt = 0;
};


//初始化输入参数
Options init_options(int & argc, char * argv[]);

//输出帮助信息
void print_help();

//编码转换
std::string gbk_to_utf8(const char * gbk_str);

//检查路径
bool check_path(std::string & utf8_path);

//初始化目标文件路径。适用于windows
std::filesystem::path init_target_path(std::string_view & raw_path);

//初始化目标目录
std::filesystem::path init_dir_path(std::string_view & raw_path);

//初始化密码长度范围
std::pair<int, int> init_password_len_range(size_t & i, int & argc, char * argv[]);

