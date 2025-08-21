#include "multithread.h"


void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
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

	//-- 需要考虑是否应移动至主线程进行 --
	//检查zip文档是否可被破解
	auto zip_entries_cnt = zip_get_num_entries(zip_archive.Get(), 0);
	if(zip_entries_cnt < 1) {
		fmt::println("-- Error: ZIP archive have no file --");
		return;
	}
	zip_uint64_t encrypted_file_index = -1;
	zip_stat_t file_stat;
	zip_stat_init(&file_stat);
	for(size_t i = 0; i < zip_entries_cnt; ++i) {
		zip_stat_index(zip_archive.Get(), i, 0, &file_stat);
		if(file_stat.valid & ZIP_STAT_ENCRYPTION_METHOD) {
			if(file_stat.encryption_method != ZIP_EM_NONE) {
				encrypted_file_index = i;
				break;
			}
		}
	}
	if(encrypted_file_index == -1) {
		fmt::println("-- Error: ZIP archive have no encrypted file --");
		return;
	}
	
	//初始化内存资源
	char * try_password = (char *)_malloca(options.maxPasswordLen + 1);
	if(try_password == nullptr) {
		return;
	}
	uint8_t * file_data = (uint8_t *)_malloca(file_stat.size);
	if(file_data == nullptr) {
		_freea(try_password);
		return;
	}

	std::ofstream out_file;
	out_file.open(fmt::format("data {}.csv", thread_id), std::ios::out | std::ios::trunc);
	out_file << "read_cnt," << std::endl;

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
				encrypted_file_index,
				0,
				try_password
			);
			if(file != nullptr) {
				auto read_cnt = zip_fread(file, file_data, file_stat.size);
				if(read_cnt > 0) {
					out_file << read_cnt << ',' << std::endl;
				}
				auto file_err_zip = zip_file_get_error(file)->zip_err;
				auto file_err_sys = zip_file_get_error(file)->sys_err;
				int file_err_code = file_err_zip + file_stat.size - read_cnt;
				zip_file_error_clear(file);
				zip_fclose(file);
				if(file_err_code != 0) {
					continue;
				}
				if(read_cnt < 1024) {
					if(!check_crc32(file_data, file_stat.crc, read_cnt)) {
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
					break;
				}
			}
			zip_error_clear(zip_archive.Get());
			if(index % 1000 == 0 && 0 == thread_id) {
				password_len_ob = password_len;
				index_ob = index;
			}
		}
	}
	if(!if_password_found) {
		index_ob = pow(char_set_len, options.maxPasswordLen);
	}
	out_file.close();
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
