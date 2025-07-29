#pragma once

#include "resources.h"
#include <thread>

//从共享的内存映射资源创建并打开独立的zip文档对象
ZipArchive init_zip_archive(const SharedResources & shared_resources);

