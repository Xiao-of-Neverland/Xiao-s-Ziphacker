#include "resources.h"

//FileHandle部分

FileHandle::~FileHandle()
{
	if(this->IsValid()) {
		CloseHandle(hFile);
	}
}

HANDLE FileHandle::Get() const
{
	return hFile;
}

HANDLE FileHandle::Release()
{
	return std::exchange(hFile, INVALID_HANDLE_VALUE);
}

bool FileHandle::IsValid() const
{
	return hFile != INVALID_HANDLE_VALUE && hFile != nullptr;
}


//FileMappingHandle部分

FileMappingHandle::~FileMappingHandle()
{
	if(this->IsValid()) {
		CloseHandle(hFileMap);
	}
}

HANDLE FileMappingHandle::Get() const
{
	return hFileMap;
}

HANDLE FileMappingHandle::Release()
{
	return std::exchange(hFileMap, nullptr);
}

bool FileMappingHandle::IsValid() const
{
	return hFileMap != nullptr;
}


//MapView部分

MapView::~MapView()
{
	if(this->IsValid()) {
		UnmapViewOfFile(lpBaseAddress);
	}
}

LPVOID MapView::Get() const
{
	return lpBaseAddress;
}

LPVOID MapView::Release()
{
	return std::exchange(lpBaseAddress, nullptr);
}

bool MapView::IsValid() const
{
	return lpBaseAddress != nullptr;
}


//ZipArchive部分

ZipArchive::~ZipArchive()
{
	if(IsValid()) {
		zip_close(archive);
	}
}

zip_t * ZipArchive::Get() const
{
	return archive;
}

bool ZipArchive::IsValid() const
{
	return archive != nullptr;
}