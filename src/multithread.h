#pragma once

#include "resources.h"
#include "options.h"
#include <thread>
#include <cmath>
#include <zlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <magic.h>


//定义全局变量
inline int running_thread_cnt = 0;
inline uint64_t expected_read_cnt = 8 * 1024;

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
	//未知或不支持的类型
	UNSUPPORTED = 0,
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
	JSON, XML,
};

//拓展名到文件类型的映射
inline const std::map<std::string, FileType> extension_to_type = {
    {"jpg", FileType::JPEG}, {"jpeg", FileType::JPEG},
    {"png", FileType::PNG},
    {"gif", FileType::GIF},
    {"bmp", FileType::BMP},
    {"tif", FileType::TIFF}, {"tiff", FileType::TIFF},
    {"webp", FileType::WEBP},
    {"mp3", FileType::MP3},
    {"wav", FileType::WAV},
    {"flac", FileType::FLAC},
    {"ogg", FileType::OGG},
    {"mp4", FileType::MP4},
    {"avi", FileType::AVI},
    {"mkv", FileType::MKV},
    {"webm", FileType::WEBM},
    {"pdf", FileType::PDF},
    {"docx", FileType::DOCX},
    {"xlsx", FileType::XLSX},
    {"pptx", FileType::PPTX},
    {"rtf", FileType::RTF},
    {"txt", FileType::TXT},
    {"zip", FileType::ZIP},
    {"gz", FileType::GZIP},
    {"tar", FileType::TAR},
    {"rar", FileType::RAR},
    {"7z", FileType::SEVENZ},
    {"elf", FileType::ELF},
    {"exe", FileType::PE}, {"dll", FileType::PE},
    {"macho", FileType::MACHO},
    {"json", FileType::JSON},
    {"xml", FileType::XML},
};

// libmagic MIME 类型到文件类型的映射
static const std::map<std::string, FileType> mime_to_type = {
    {"image/jpeg", FileType::JPEG},
    {"image/png", FileType::PNG},
    {"image/gif", FileType::GIF},
    {"image/bmp", FileType::BMP},
    {"image/tiff", FileType::TIFF},
    {"image/webp", FileType::WEBP},
    {"audio/mpeg", FileType::MP3},
    {"audio/wav", FileType::WAV},
    {"audio/flac", FileType::FLAC},
    {"audio/ogg", FileType::OGG},
    {"video/mp4", FileType::MP4},
    {"video/x-msvideo", FileType::AVI},
    {"video/x-matroska", FileType::MKV},
    {"video/webm", FileType::WEBM},
    {"application/pdf", FileType::PDF},
    {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", FileType::DOCX},
    {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", FileType::XLSX},
    {"application/vnd.openxmlformats-officedocument.presentationml.presentation", FileType::PPTX},
    {"application/rtf", FileType::RTF},
    {"text/plain", FileType::TXT},
    {"application/zip", FileType::ZIP},
    {"application/gzip", FileType::GZIP},
    {"application/x-tar", FileType::TAR},
    {"application/x-rar", FileType::RAR},
    {"application/x-7z-compressed", FileType::SEVENZ},
    {"application/x-elf", FileType::ELF},
    {"application/x-dosexec", FileType::PE},
    {"application/x-mach-binary", FileType::MACHO},
    {"application/json", FileType::JSON},
    {"text/xml", FileType::XML},
};

//线程工作函数
void thread_worker_function(
    int thread_id,
    int thread_cnt,
    SharedResources shared_resources,
    zip_uint64_t file_index,
	Options options
);

//检查系统内存
inline bool check_memory(const int & thread_cnt);

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

//根据文件拓展名判断预期类型
FileType get_expected_file_type(const char * file_name);

//对于以store方式存储的特定类型的较大文件，验证解密后数据的类型对应特征
bool check_magic(
    magic_t magic,
    const uint8_t * file_data,
    zip_uint64_t data_len,
    FileType expected_type
);

