#include "multithread.h"

extern bool if_password_found;
extern std::string password;

extern uint64_t start_index_when_found;
extern uint64_t end_index_when_found;
extern uint64_t index_when_found;

void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
	Options & options
)
{
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
	zip_uint64_t encrypted_entry_index = -1;
	zip_stat_t entry_stat;
	zip_stat_init(&entry_stat);
	for(size_t i = 0; i < zip_entries_cnt; ++i) {
		zip_stat_index(zip_archive.Get(), i, 0, &entry_stat);
		if(entry_stat.valid & ZIP_STAT_ENCRYPTION_METHOD) {
			if(entry_stat.encryption_method != ZIP_EM_NONE) {
				fmt::println("Try entry: {}", entry_stat.name);
				encrypted_entry_index = i;
				break;
			}
		}
	}
	if(encrypted_entry_index == -1) {
		fmt::println("-- Error: ZIP archive have no encrypted file --");
		return;
	}

	char * try_password = (char *)_malloca(sizeof(char) * options.max_password_len);
	auto char_set_len = options.charSet.length();
	for(int password_len = options.min_password_len;
		password_len <= options.max_password_len;
		++password_len) {
		if(if_password_found) {
			return;
		}
		auto index_range = init_index_range(
			thread_id,
			thread_cnt,
			char_set_len,
			password_len
		);
		auto start_index = index_range.first;
		auto end_index = index_range.second;
		for(uint64_t index = start_index; index < end_index; ++index) {
			if(if_password_found) {
				return;
			}
			generate_password(index, options.charSet, char_set_len, password_len, try_password);
			fmt::println("Trying password: {}", try_password);
			auto entry = zip_fopen_index_encrypted(
				zip_archive.Get(),
				encrypted_entry_index,
				0,
				try_password
			);
			auto archive_err = zip_get_error(zip_archive.Get());
			fmt::println("Zip err: {}, {}", archive_err->zip_err, archive_err->sys_err);
			if(entry != nullptr) {
				char temp_read[1024];
				auto bytes_read_cnt = zip_fread(entry, temp_read, 1024);

				auto entry_err = zip_file_get_error(entry);
				zip_file_error_clear(entry);
				zip_fclose(entry);
				fmt::println("File err: {}, {}", zip_error_code_zip(entry_err), zip_error_code_system(entry_err));
				if(zip_error_code_zip(archive_err) != ZIP_ER_OK || zip_error_code_zip(entry_err) != ZIP_ER_OK) {
					continue;
				}
				if(bytes_read_cnt > 0) {
					if(try_password != nullptr) {
						password = try_password;
						start_index_when_found = start_index;
						end_index_when_found = end_index;
						index_when_found = index;
						if_password_found = true;
						return;
					} else {
						fmt::println("-- Error: try_password is null --");
						return;
					}
				}
			} else {
				auto zip_archive_err = zip_get_error(zip_archive.Get());
				if(zip_error_code_zip(zip_archive_err) != ZIP_ER_WRONGPASSWD) {
					fmt::println("-- Error: Unknown error while trying password --");
					break;
				}
			}
			zip_error_clear(zip_archive.Get());
		}
	}
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

std::pair<uint64_t, uint64_t> init_index_range(
	int thread_id,
	int thread_cnt,
	int char_set_len,
	int password_len
)
{
	uint64_t max_index = pow(char_set_len, password_len);
	uint64_t first = max_index * thread_id / thread_cnt;
	uint64_t second = max_index;
	if(thread_id + 1 < thread_cnt) {
		second = max_index * (thread_id + 1) / thread_cnt;
	}
	return {first, second};
}
