#pragma once

#include "options.h"
#include "resources.h"
#include "multithread.h"
#include <chrono>
#include <iostream>


constexpr inline int progress_bar_width = 50;

typedef std::chrono::high_resolution_clock timer;
typedef std::chrono::steady_clock::time_point time_point;

//根据数据输出任务进度条
void print_progress(uint64_t try_cnt_ob, uint64_t try_cnt_max, time_point start_timer);

