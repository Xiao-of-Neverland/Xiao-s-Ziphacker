#pragma once

#include <zip.h>
#include <string>
#include <fmt/core.h>
#include <utility>
#include <memory>


//windows API
#if defined(_WIN32)

#include <Windows.h>

//封装管理文件句柄
class FileHandle
{
private:
	HANDLE hFile;
public:
	//构造函数：接收一个HANDLE。必须显式调用
	explicit FileHandle(HANDLE h = INVALID_HANDLE_VALUE):
		hFile(h)
	{}

	//禁用拷贝构造和拷贝赋值
	FileHandle(const FileHandle &) = delete;
	FileHandle & operator=(const FileHandle &) = delete;

	//移动构造
	FileHandle(FileHandle && other) noexcept:
		hFile(std::exchange(other.hFile, INVALID_HANDLE_VALUE))
	{}

	//移动赋值
	FileHandle & operator=(FileHandle && other)noexcept
	{
		if(this != &other) {
			if(this->IfValid()) {
				CloseHandle(hFile);
			}
			this->hFile = std::exchange(other.hFile, INVALID_HANDLE_VALUE);
		}
		return *this;
	}

	//析构函数：关闭HANDLE
	~FileHandle();

	//可隐式转换为HANDLE
	operator HANDLE() const
	{
		return hFile;
	}

	//获取HANDLE
	HANDLE Get() const;

	//释放HANDLE
	HANDLE Release();

	//检查HANDLE是否有效
	bool IfValid() const;
};


//封装管理文件映射句柄
class FileMappingHandle
{
private:
	HANDLE hFileMap;
public:
	explicit FileMappingHandle(HANDLE h = nullptr):
		hFileMap(h)
	{}

	FileMappingHandle(const FileMappingHandle &) = delete;
	FileMappingHandle & operator=(const FileMappingHandle &) = delete;

	FileMappingHandle(FileMappingHandle && other) noexcept:
		hFileMap(std::exchange(other.hFileMap, nullptr))
	{}

	FileMappingHandle & operator=(FileMappingHandle && other) noexcept
	{
		if(this != &other) {
			if(this->IfValid()) {
				CloseHandle(hFileMap);
			}
			this->hFileMap = std::exchange(other.hFileMap, nullptr);
		}
		return *this;
	}

	~FileMappingHandle();

	operator HANDLE() const
	{
		return hFileMap;
	}

	HANDLE Get() const;

	HANDLE Release();

	bool IfValid() const;
};
typedef FileMappingHandle MapHandle;


//封装管理内存视图
class MapView
{
private:
	LPVOID lpBaseAddress;
public:
	explicit MapView(LPVOID addr = nullptr):
		lpBaseAddress(addr)
	{}

	MapView(const MapView &) = delete;
	MapView & operator=(const MapView &) = delete;

	MapView(MapView && other) noexcept:
		lpBaseAddress(std::exchange(other.lpBaseAddress, nullptr))
	{}

	MapView & operator=(MapView && other) noexcept
	{
		if(this != &other) {
			if(this->IfValid()) {
				UnmapViewOfFile(lpBaseAddress);
			}
			this->lpBaseAddress = std::exchange(other.lpBaseAddress, nullptr);
		}
		return *this;
	}

	~MapView();

	operator LPVOID() const
	{
		return lpBaseAddress;
	}

	LPVOID Get() const;

	LPVOID Release();

	bool IfValid() const;
};

//结构体：线程共享资源
struct SharedResources
{
	bool ifValid = false;
	bool ifUseZipDataPtr = false; // 为true时使用pZipData（zip数据指针）代替pMapView（内存视图指针）
	LPVOID pZipData = nullptr;
	std::shared_ptr<FileHandle> pFileHandle;
	std::shared_ptr<FileMappingHandle> pFileMapHandle;
	std::shared_ptr<MapView> pMapView;
	zip_uint64_t fileSize = 0;
};

#endif


//linux API
#if defined(__linux__)

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


//封装管理Linux文件描述符
class FileDescriptor
{
private:
	int fileDescriptor;
public:
	explicit FileDescriptor(int fd = -1):
		fileDescriptor(fd)
	{}

	FileDescriptor(const FileDescriptor &) = delete;
	FileDescriptor & operator=(const FileDescriptor &) = delete;

	FileDescriptor(FileDescriptor && other) noexcept:
		fileDescriptor(std::exchange(other.fileDescriptor, -1))
	{}

	FileDescriptor & operator=(FileDescriptor && other) noexcept
	{
		if(this != &other) {
			if(this->IfValid()) {
				close(fileDescriptor);
			}this->fileDescriptor = std::exchange(other.fileDescriptor, -1);
		}
	}

	~FileDescriptor();

	operator int() const
	{
		return fileDescriptor;
	}

	int Get() const;

	int Release();

	bool IfValid() const;
};


//封装管理Linux文件映射
class FileMap
{
private:
	void * mapAddr;
	size_t length;
public:
	explicit FileMap(void * addr = MAP_FAILED, size_t len = 0):
		mapAddr(addr),
		length(len)
	{}

	FileMap(const FileMap &) = delete;
	FileMap & operator=(const FileMap &) = delete;

	FileMap(FileMap && other) noexcept:
		mapAddr(std::exchange(other.mapAddr, MAP_FAILED)),
		length(std::exchange(other.length, 0))
	{}

	FileMap & operator=(FileMap && other) noexcept
	{
		if(this != &other) {
			if(this->IfValid()) {
				munmap(mapAddr, length);
			}
		}
	}

	~FileMap();

	void * GetAddr() const;

	int GetLen() const;

	FileMap Release();

	bool IfValid() const;
};


//结构体：线程共享资源
struct SharedResources
{
	bool ifValid = false;
	bool ifUseZipDataPtr = false; // 为true时使用pZipData（zip数据指针）代替pMapView（内存视图指针）
	void * pZipData = nullptr;
	std::shared_ptr<FileDescriptor> pFileDescriptor;
	std::shared_ptr<FileMap> pFileMap;
	zip_uint64_t fileSize = 0;
};

#endif


//封装管理zip_t*
class ZipArchive
{
private:
	zip_t * archive;
public:
	explicit ZipArchive(zip_t * z = nullptr):
		archive(z)
	{}

	ZipArchive(const ZipArchive &) = delete;
	ZipArchive & operator=(const ZipArchive &) = delete;

	ZipArchive(ZipArchive && other) noexcept:
		archive(std::exchange(other.archive, nullptr))
	{}

	ZipArchive & operator=(ZipArchive && other) noexcept
	{
		if(this != &other) {
			if(this->IfValid()) {
				zip_close(archive);
			}
			this->archive = std::exchange(other.archive, nullptr);
		}
	}

	~ZipArchive();

	operator zip_t * () const
	{
		return archive;
	}

	zip_t * Get() const;

	bool IfValid() const;
};


//初始化线程共享资源
SharedResources init_shared_resources(std::string zip_path);

//从共享的内存映射资源创建并打开独立的zip文档对象
ZipArchive init_zip_archive(const SharedResources & shared_resources);

//检查并尝试整理文件数据，随后调用init_zip_archive。仅主线程使用
ZipArchive pre_init_zip_archive(SharedResources & shared_resources);

//查找zip数据的起始位置和大小。用于应对多格式文件
bool find_zip_data(SharedResources & shared_resources);

