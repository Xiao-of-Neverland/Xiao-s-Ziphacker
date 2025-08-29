# CMakeList.txt: Xiao-s-Ziphacker 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.20)

project ("Xiao-s-Ziphacker" LANGUAGES CXX CUDA)

enable_language(CUDA)

include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
include_directories(src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

set(CMAKE_CUDA_RUNTIME_LIBRARY)

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libzip REQUIRED)
find_package(unofficial-libmagic CONFIG REQUIRED)

add_executable (XZP
	"src/main.h"
	"src/main.cpp"
	"src/resources.h"
	"src/resources.cpp"
	"src/multithread.h"
	"src/multithread.cpp"
	"src/options.h"
	"src/options.cpp"
)

target_link_libraries(
	XZP
		PRIVATE
			fmt::fmt
			ZLIB::ZLIB
			libzip::zip
			unofficial::libmagic::libmagic
)

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/magic.mgc")
    configure_file(${unofficial-libmagic_DICTIONARY} ${CMAKE_CURRENT_BINARY_DIR}/magic.mgc COPYONLY)
endif()
string(REPLACE "\\" "/" MAGIC_DB_PATH "${CMAKE_CURRENT_BINARY_DIR}/magic.mgc")
add_definitions(-DMAGIC_DB_PATH="${MAGIC_DB_PATH}")

