# CMakeList.txt: Xiao's Ziphacker 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.20)

project ("Xiao's Ziphacker" LANGUAGES CXX CUDA)

enable_language(CUDA)

include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libzip REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

add_executable (main "main.cpp" "PreValidation.cu" "resources.h" "resources.cpp")

set_source_files_properties("PreValidation.cu" PROPERTIES LANGUAGE CUDA)

target_link_libraries(main
	PRIVATE
		fmt::fmt
		ZLIB::ZLIB
		libzip::zip
)