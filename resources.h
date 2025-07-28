#pragma once

#include <Windows.h>
#include <zip.h>
#include <string.h>
#include <fmt/core.h>
#include <utility>

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
			hFile = std::exchange(other.hFile, INVALID_HANDLE_VALUE);
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
			hFileMap = std::exchange(other.hFileMap, nullptr);
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
			lpBaseAddress = std::exchange(other.lpBaseAddress, nullptr);
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