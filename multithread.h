#pragma once

#include "resources.h"
#include "options.h"
#include <thread>
#include <cmath>
#include <zlib.h>
#include <fstream>


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


//枚举类，表示特定文件类型
enum class FileType
{
	UNKNOWN,
	//图片文件
	JPEG, PNG, GIF, BMP, TIFF, WEBP,
	//音频文件
	MP3, WAV, FLAC, OGG,
	//视频文件
	MP4, AVI, MKV, WEBM,
	//文档和文本文件
	PDF, DOCX, XLSX, PPTX, RTF, TXT,
	//压缩文件
	ZIP, GZIP, TAR, RAR, SEVENZ,
	//可执行和二进制文件
	ELF, PE, MACHO,
	//其他
	JSON, XML, SQLITE
};


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

//对于以store方式存储的特定类型的较大文件，验证解密后数据的类型对应特征
bool check_magic(const uint8_t * file_data, zip_uint64_t data_len);

