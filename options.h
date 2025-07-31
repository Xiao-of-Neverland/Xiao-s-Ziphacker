#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <fmt/core.h>


std::string numbers = "0123456789";
std::string uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string lowers = "abcdefghijklmnopqrstuvwxyz";


struct Options
{
	bool isValid = false;
	bool ifMultipleTarget = false;
	std::filesystem::path targetPath;
	std::string charSet;
};


Options init_options(int & argc, char * argv[]);

