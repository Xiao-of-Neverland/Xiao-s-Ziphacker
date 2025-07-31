#pragma once

#include "resources.h"
#include <thread>

//线程工作函数
void thread_worker_function(int thread_id, SharedResources shared_resources);

