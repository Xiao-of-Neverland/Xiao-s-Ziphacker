#include "multithread.h"


void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
	zip_uint64_t file_index,
	Options options
)
{
	fmt::println("Thread id: {} running...", thread_id);

	if(if_password_found) {
		return;
	}

	auto zip_archive = init_zip_archive(shared_resources);
	if(!zip_archive.IsValid()) {
		fmt::println("-- Error: Failed to open ZIP archive --");
		return;
	}

	zip_stat_t file_stat;
	zip_stat_init(&file_stat);
	zip_stat_index(zip_archive.Get(), file_index, 0, &file_stat);
	auto file_type = get_expected_file_type(file_stat.name);
	bool if_need_check_crc = false;
	bool if_need_check_magic = false;
	if(file_stat.size <= read_cnt_max) {
		if_need_check_crc = true;
		read_cnt_max = file_stat.size;
	} else {
		if(file_stat.comp_method == ZIP_CM_STORE) {
			if(file_type == FileType::UNSUPPORTED) {
				if_need_check_crc = true;
				read_cnt_max = file_stat.size;
			} else {
				if_need_check_magic = true;
			}
		}
	}
	
	//检查系统内存
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(MEMORYSTATUSEX);
	if(GlobalMemoryStatusEx(&mem_info)) {
		if(mem_info.ullTotalPhys < thread_cnt * read_cnt_max * 3 / 4) {
			fmt::println("-- Error: Pyhsical memory not enough for read file in zip --");
			return;
		}
	} else {
		fmt::println("-- Warnning: Cant get physical memory --");
	}

	//初始化内存资源
	char * try_password = (char *)_malloca(options.maxPasswordLen + 1);
	if(try_password == nullptr) {
		return;
	}
	uint8_t * file_data = (uint8_t *)_malloca(read_cnt_max);
	if(file_data == nullptr) {
		_freea(try_password);
		return;
	}

	//初始化libmagic
	magic_t magic_cookie = nullptr;
	if(if_need_check_magic) {
		magic_cookie = magic_open(MAGIC_MIME | MAGIC_ERROR);
		if(magic_cookie == nullptr) {
			fmt::println("Error: Failed to init libmagic");
			return;
		}
		if(magic_load(magic_cookie, nullptr) != 0) {
			auto magic_err = magic_error(magic_cookie);
			fmt::println("Error: Failed to load magic database: {}", magic_err);
			magic_close(magic_cookie);
			return;
		}
	}

	//遍历密码长度，尝试密码组合
	auto char_set_len = options.charSet.length();
	for(int password_len = options.minPasswordLen;
		password_len <= options.maxPasswordLen;
		++password_len) {
		if(if_password_found) {
			break;
		}
		uint64_t index_max = pow(char_set_len, password_len);
		for(uint64_t index = thread_id; index < index_max; index += thread_cnt) {
			if(if_password_found) {
				break;
			}
			generate_password(index, options.charSet, char_set_len, password_len, try_password);
			auto file = zip_fopen_index_encrypted(
				zip_archive.Get(),
				file_index,
				0,
				try_password
			);
			if(file != nullptr) {
				auto read_cnt = zip_fread(file, file_data, read_cnt_max);
				auto file_err_zip = zip_file_get_error(file)->zip_err;
				auto file_err_sys = zip_file_get_error(file)->sys_err;
				zip_file_error_clear(file);
				zip_fclose(file);
				if(file_err_zip + file_err_sys != 0) {
					continue;
				}
				if(read_cnt < read_cnt_max) {
					continue;
				}
				if(if_need_check_crc) {
					if(!check_crc32(file_data, file_stat.crc, read_cnt)) {
						continue;
					}
				}
				if(if_need_check_magic) {
					if(!check_magic(magic_cookie, file_data, read_cnt, file_type)) {
						continue;
					}
				}
				if(try_password != nullptr) {
					password = try_password;
					index_when_found = index;
					if_password_found = true;
					break;
				} else {
					fmt::println("-- Error: try_password is null --");
					break;
				}
			} else {
				auto zip_archive_err = zip_get_error(zip_archive.Get());
				if(zip_error_code_zip(zip_archive_err) != ZIP_ER_WRONGPASSWD) {
					fmt::println("-- Error: Unknown error while trying password --");
					fmt::println("{}, {}", zip_archive_err->zip_err, zip_archive_err->sys_err);
					break;
				}
			}
			zip_error_clear(zip_archive.Get());
			if(0 == thread_id) {
				password_len_ob = password_len;
				index_ob = index;
			}
		}
	}
	if(!if_password_found) {
		index_ob = pow(char_set_len, options.maxPasswordLen);
	}
	_freea(try_password);
	_freea(file_data);
}

void generate_password(
	uint64_t index,
	const std::string & char_set,
	const size_t & char_set_len,
	const int & password_len,
	char * try_password
)
{
	int i = password_len - 1;
	while(index >= char_set_len) {
		try_password[i] = char_set[index % char_set_len];
		index /= char_set_len;
		--i;
	}
	try_password[i] = char_set[index];
	--i;
	while(i >= 0) {
		try_password[i] = char_set[0];
		--i;
	}
	try_password[password_len] = '\0';
}

bool check_crc32(const uint8_t * file_data, zip_uint32_t crc, zip_uint64_t data_len)
{
	uint32_t data_crc = crc32(0L, Z_NULL, 0);
	data_crc = crc32(data_crc, file_data, data_len);
	return data_crc == crc;
}

FileType get_expected_file_type(const char * file_name)
{
	std::string_view name_str(file_name);
	auto dot_pos = name_str.find_last_of('.');
	if(dot_pos == std::string_view::npos || dot_pos == name_str.length() - 1) {
		return FileType::UNSUPPORTED;
	}
	std::string extension_str(name_str.substr(dot_pos + 1));
	std::transform(extension_str.begin(), extension_str.end(), extension_str.begin(), ::tolower);

	auto type_it = extension_to_type.find(extension_str);
	if(type_it == extension_to_type.end()) {
		return FileType::UNSUPPORTED;
	} else {
		return type_it->second;
	}
}

bool check_magic(
	magic_t magic,
	const uint8_t * file_data,
	zip_uint64_t data_len,
	FileType expected_type
)
{
	const char * mime = magic_buffer(magic, file_data, data_len);
	if(mime == nullptr) {
		magic_close(magic);
		return false;
	}

	std::string mime_str(mime);
	auto semicolon_pos = mime_str.find(';');
	if(semicolon_pos != std::string::npos) {
		mime_str = mime_str.substr(0, semicolon_pos);
	}
	//out_file << mime_str << ',';
	auto type_it = mime_to_type.find(mime_str);
	if(type_it == mime_to_type.end()) {
		return false;
	}

	if(type_it->second == expected_type) {
		return true;
	} else {
		return false;
	}
}
