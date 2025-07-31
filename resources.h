#pragma once

#include <Windows.h>
#include <zip.h>
#include <string>
#include <fmt/core.h>
#include <utility>
#include <memory>

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
			if(this->IsValid()) {
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
	bool IsValid() const;
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
			if(this->IsValid()) {
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

	bool IsValid() const;
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
			if(this->IsValid()) {
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

	bool IsValid() const;
};


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

	ZipArchive & operator=(ZipArchive && other)noexcept
	{
		if(this != &other) {
			if(this->IsValid()) {
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

	bool IsValid() const;
};


//结构体：线程共享资源
struct SharedResources
{
	bool isValid = false;
	std::shared_ptr<FileHandle> pFileHandle;
	std::shared_ptr<FileMappingHandle> pFileMapHandle;
	std::shared_ptr<MapView> pMapView;
	zip_uint64_t fileSize = 0;
};


//初始化线程共享资源
SharedResources init_shared_resources(std::string target_path);

//从共享的内存映射资源创建并打开独立的zip文档对象
ZipArchive init_zip_archive(const SharedResources & shared_resources);

