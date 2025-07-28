#include <zlib.h>
#include <zip.h>
#include <string>
#include <fmt/core.h>
#include <iostream>
#include <Windows.h>

void start_hello();

using namespace std;

zip_t * open_target_file(string target_path)
{
	//创建目标文件句柄
	auto handle_target_file = CreateFileA(
		target_path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr
	);
	if(handle_target_file == INVALID_HANDLE_VALUE) {
		fmt::println("-- Failed to create file handle: {} --", GetLastError());
		return nullptr;
	}

	//从内存映射对象创建zip资源对象
	zip_error_t err;
	zip_error_init(&err);
	auto src_target_file = zip_source_win32handle_create(
		handle_target_file,
		0,
		0,
		&err
	);
	if(src_target_file == nullptr) {
		fmt::println("-- Failed to create zip source: {}", zip_error_strerror(&err));
		return nullptr;
	}
	
	//从zip资源对象打开目标文件
	auto p_target_file = zip_open_from_source(
		src_target_file,
		ZIP_RDONLY,
		&err
	);
	if(p_target_file == nullptr) {
		fmt::println("-- Failed to open file: {} --", zip_error_strerror(&err));
		return nullptr;
	}

	return p_target_file;
}

int main()
{
	string target_path = "D:\\VS2022\\Xiao's Ziphacker\\test.zip";
	int err;

	auto p_target = open_target_file(target_path.c_str());

	//auto p_target = zip_open(target_path.c_str(), 0, &err);
	if(p_target != nullptr) {
		auto cnt_entries = zip_get_num_entries(p_target, 0);
		zip_uint64_t index_smallest_file = -1;
		zip_uint64_t size_smallest_file = ZIP_UINT64_MAX;
		cout << cnt_entries << endl;
		for(size_t i = 0; i < (size_t)cnt_entries; i++) {
			zip_stat_t file_stat;
			if(zip_stat_index(p_target, i, 0, &file_stat) == 0) {
				if(file_stat.size != 0 && file_stat.size < size_smallest_file) {
					index_smallest_file = i;
					size_smallest_file = file_stat.size;
				}
				cout << file_stat.name << ' ' << file_stat.size;
				if(file_stat.valid & ZIP_STAT_ENCRYPTION_METHOD) {
					cout << ' ' << "Encryption method: ";
					switch(file_stat.encryption_method) {
						case ZIP_EM_NONE:
							cout << "Not encrypted";
							break;
						case ZIP_EM_TRAD_PKWARE:
							cout << "PKWARE";
							break;
						case ZIP_EM_AES_128:
							cout << "AES_128";
							break;
						case ZIP_EM_AES_192:
							cout << "AES_192";
							break;
						case ZIP_EM_AES_256:
							cout << "AES_256";
							break;
					}
					
				}
				cout << endl;
			}
		}
		zip_close(p_target);
	}
	
	cout << "zlib version: " << zlibVersion() << endl;
	cout << "libzip version: " << LIBZIP_VERSION << endl;

	start_hello();

	return 0;
}
