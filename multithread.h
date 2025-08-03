#pragma once

#include "resources.h"
#include "options.h"
#include <thread>

//线程工作函数
void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
	Options & options
);

void generate_password(uint64_t index, int password_len, char * password);