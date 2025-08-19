#pragma once

#include "resources.h"
#include "options.h"
#include <thread>
#include <cmath>
#include <zlib.h>


//定义全局变量

//找到的密码
inline std::string password("");
//是否找到密码
inline bool if_password_found = false;
//观察当前尝试密码长度，用于在主线程中生成进度条
inline int password_len_ob = 0;
//观察当前尝试索引，用于在主线程中生成进度条
inline uint64_t index_ob = 0;
//成功找到密码时的当前尝试索引，用于在主线程中统计性能
inline uint64_t index_when_found = 0; 


//线程工作函数
void thread_worker_function(
	int thread_id,
	int thread_cnt,
	SharedResources shared_resources,
	Options options
);

//-- 需要极限优化性能 --
//将index转化为对应密码字符串
void generate_password(
	uint64_t index,
	const std::string & char_set,
	const size_t & char_set_len,
	const int & password_len,
	char * try_password
);

//对于极小文件，验证解密后数据的CRC32，判断尝试的密码是否正确
bool check_crc32(const uint8_t * file_data, zip_uint32_t crc, zip_uint64_t data_len);

