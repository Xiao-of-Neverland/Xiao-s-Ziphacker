#include "multithread.h"

std::atomic<bool> flag_password_found(false);
std::string password;

void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
	Options & options
)
{
	if(flag_password_found.load()) {
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
	for(size_t i = 0; i < zip_entries_cnt; i++) {
		zip_stat_index(zip_archive.Get(), i, 0, &entry_stat);
		if(entry_stat.valid & ZIP_STAT_ENCRYPTION_METHOD) {
			if(entry_stat.encryption_method != ZIP_EM_NONE) {
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
	for(int password_len = options.min_password_len;
		password_len <= options.max_password_len;
		password_len++) {
		auto index_range = init_index_range(
			thread_id,
			thread_cnt,
			options.charSet.length(),
			password_len
		);
		auto start_index = index_range.first;
		auto end_index = index_range.second;
		for(uint64_t index = start_index; index < end_index; index++) {
			if(flag_password_found.load()) {
				break;
			}
			generate_password(index, options.charSet, password_len, try_password);
			auto entry = zip_fopen_index_encrypted(
				zip_archive.Get(),
				encrypted_entry_index,
				0,
				try_password
			);
			if(entry != nullptr) {
				zip_fclose(entry);
				if(try_password != nullptr) {
					password = try_password;
					flag_password_found.store(true);
				} else {
					fmt::println("-- Error: try_password is null --");
				}
				break;
			} else {

			}
		}
	}
	
}

void generate_password(
	uint64_t index,
	std::string & char_set,
	int password_len,
	char * current_password
)
{

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
