#pragma once

#include "options.h"
#include "resources.h"
#include "multithread.h"
#include <chrono>
#include <iostream>

constexpr inline int progress_bar_width = 50;

typedef std::chrono::high_resolution_clock timer;
typedef std::chrono::steady_clock::time_point time_point;

//获取zip文档内用于破解的文件索引，同时预处理多格式文件
int get_file_index(SharedResources shared_resources);

//等待子线程任务进行，打印进度条
void wait_worker(
	Options & options,
	SharedResources & shared_resources,
	time_point & start_time,
	std::vector<std::thread> & worker_threads
);

//根据数据输出任务进度条
void print_progress(uint64_t try_cnt_ob, uint64_t try_cnt_max, time_point start_time);

//输出任务结果信息
void print_result_info(Options & options, time_point start_time);

