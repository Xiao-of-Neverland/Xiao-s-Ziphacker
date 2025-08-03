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
}
