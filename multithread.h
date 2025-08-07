#pragma once

#include "resources.h"
#include "options.h"
#include <thread>
#include <cmath>

//线程工作函数
void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
	Options & options
);

//将index转化为对应密码字符串
void generate_password(
	uint64_t index,
	std::string & char_set,
	int password_len,
	char * current_password
);

//初始化对应密码长度和线程id的index范围（左闭右开区间）
std::pair<uint64_t, uint64_t> init_index_range(
	int thread_id,
	int thread_cnt,
	int char_set_len,
	int password_len
);

