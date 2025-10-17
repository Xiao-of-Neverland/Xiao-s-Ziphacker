# CMakeList.txt: Xiao-s-Ziphacker 的 CMake 项目，在此处包括源代码并定义


# 项目逻辑部分

cmake_minimum_required (VERSION 3.21)

project ("XiaosZiphacker" LANGUAGES CXX)

include_directories(src)

set(CMAKE_CXX_STANDARD 17)

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libzip REQUIRED)
find_package(ICU REQUIRED COMPONENTS dt uc i18n)
find_package(unofficial-libmagic CONFIG REQUIRED)

add_executable (XiaosZiphacker
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
	XiaosZiphacker
		PRIVATE
			fmt::fmt
			ZLIB::ZLIB
			libzip::zip
			ICU::dt
			ICU::uc
			ICU::i18n
			unofficial::libmagic::libmagic
)

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/magic.mgc")
    configure_file(${unofficial-libmagic_DICTIONARY} ${CMAKE_CURRENT_BINARY_DIR}/magic.mgc COPYONLY)
endif()


# 项目打包部分

install(TARGETS XiaosZiphacker
    RUNTIME DESTINATION .
)

install(FILES "${unofficial-libmagic_DICTIONARY}"
    DESTINATION .
    RENAME "magic.mgc"
)

install(DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/"
	DESTINATION .
	FILES_MATCHING PATTERN "*.dll"
)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "1.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A ziphacker tool by Xiao")
set(CPACK_PACKAGE_VENDOR "Xiao")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")

set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

include(CPack)

if(CPACK_GENERATOR)
	add_custom_target(
		CreatePackage
			COMMAND ${CMAKE_CPACK_COMMAND} -C ${CMAKE_BINARY_DIR} --config CPackConfig.cmake
			DEPENDS XiaosZiphacker
			COMMENT "Generating package..."
	)
endif()

