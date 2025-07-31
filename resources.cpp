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


//外部函数部分

SharedResources init_shared_resources(std::string target_path)
{
	SharedResources shared_resources;

	//打开目标文件，获取文件句柄
	auto p_file_handle = std::make_shared<FileHandle>(CreateFileA(
		target_path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	));
	if(!p_file_handle->IsValid()) {
		fmt::println("-- Failed to open file: {} --", GetLastError());
		return {};
	}
	shared_resources.pFileHandle = std::move(p_file_handle);

	//获取文件大小
	LARGE_INTEGER file_size;
	if(!GetFileSizeEx(shared_resources.pFileHandle->Get(), &file_size)) {
		fmt::println("-- Failed to get file size: {} --", GetLastError());
		return {};
	}
	shared_resources.fileSize = file_size.QuadPart;

	//为文件创建内存映射对象，获取句柄
	auto p_file_map_handle = std::make_shared<FileMappingHandle>(CreateFileMappingA(
		shared_resources.pFileHandle->Get(),
		nullptr,
		PAGE_READONLY,
		file_size.HighPart,
		file_size.LowPart,
		nullptr
	));
	if(!p_file_map_handle->IsValid()) {
		fmt::println("-- Failed to create file mapping: {} --", GetLastError());
		return {};
	}
	shared_resources.pFileMapHandle = std::move(p_file_map_handle);

	//将文件映射的视图映射到进程的地址空间
	auto p_map_view = std::make_shared<MapView>(MapViewOfFile(
		shared_resources.pFileMapHandle->Get(),
		FILE_MAP_READ,
		0,
		0,
		0
	));
	if(!p_map_view->IsValid()) {
		fmt::println("-- Failed to map view of file: {} --", GetLastError());
		return {};
	}
	shared_resources.pMapView = std::move(p_map_view);

	shared_resources.isValid = true;
	return shared_resources;
}

ZipArchive init_zip_archive(const SharedResources & shared_resources)
{
	zip_error_t err;
	zip_error_init(&err);

	//从内存缓冲区创建zip资源对象
	zip_source_t * p_zip_source = zip_source_buffer_create(
		shared_resources.pMapView->Get(),
		shared_resources.fileSize,
		0,
		&err
	);
	if(p_zip_source == nullptr) {
		fmt::println("-- Failed to create zip source from buffer: {}", zip_error_strerror(&err));
		return ZipArchive(nullptr);
	}

	//从zip资源对象打开zip文档对象
	zip_t * p_zip_archive = zip_open_from_source(
		p_zip_source,
		ZIP_RDONLY,
		&err
	);
	if(p_zip_archive == nullptr) {
		fmt::println("-- Failed to open zip archive from source: {} --", zip_error_strerror(&err));
		return ZipArchive(nullptr);
	}

	return ZipArchive(p_zip_archive);
}