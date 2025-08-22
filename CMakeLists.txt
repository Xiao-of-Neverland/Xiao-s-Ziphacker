# CMakeList.txt: Xiao-s-Ziphacker 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.20)

project ("Xiao-s-Ziphacker" LANGUAGES CXX CUDA)

enable_language(CUDA)

include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

set(CMAKE_CUDA_RUNTIME_LIBRARY)

find_package(fmt REQUIRED )
find_package(ZLIB REQUIRED)
find_package(libzip REQUIRED)
find_library(MAGIC_LIBRARY NAMES magic libmagic)
find_path(MAGIC_INCLUDE_DIR NAMES magic.h)

if(NOT MAGIC_LIBRARY OR NOT MAGIC_INCLUDE_DIR)
    message(FATAL_ERROR "libmagic not found. Ensure it is installed via vcpkg.")
endif()

add_executable (XZP
	"main.h"
	"main.cpp"
	"PreValidation.cu"
	"resources.h"
	"resources.cpp"
	"multithread.h"
	"multithread.cpp"
	"options.h"
	"options.cpp"
)

set_source_files_properties("PreValidation.cu" PROPERTIES LANGUAGE CUDA)

target_include_directories(XZP PRIVATE ${MAGIC_INCLUDE_DIR})

target_link_libraries(XZP
	PRIVATE
		fmt::fmt
		ZLIB::ZLIB
		libzip::zip
		${MAGIC_LIBRARY}
)