#include "multithread.h"

std::atomic<bool> flag_password_found(false);

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

	char * current_password = (char *)_alloca(sizeof(char) * options.max_password_len);

}
