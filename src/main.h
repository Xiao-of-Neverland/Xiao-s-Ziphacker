#pragma once

#include "options.h"
#include "resources.h"
#include "multithread.h"
#include <chrono>
#include <iostream>


//根据数据输出任务进度条
void show_progress(uint64_t try_cnt_ob, uint64_t try_cnt_max, int bar_width = 50);

