#include "resources.h"

//windows API
#if defined(_WIN32)

//FileHandle部分

FileHandle::~FileHandle()
{
	if(IfValid()) {
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
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

bool FileHandle::IfValid() const
{
	return hFile != INVALID_HANDLE_VALUE && hFile != nullptr;
}


//FileMappingHandle部分

FileMappingHandle::~FileMappingHandle()
{
	if(IfValid()) {
		CloseHandle(hFileMap);
		hFileMap = nullptr;
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

bool FileMappingHandle::IfValid() const
{
	return hFileMap != nullptr;
}


//MapView部分

MapView::~MapView()
{
	if(IfValid()) {
		UnmapViewOfFile(lpBaseAddress);
		lpBaseAddress = nullptr;
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

bool MapView::IfValid() const
{
	return lpBaseAddress != nullptr;
}


//ZipArchive部分

ZipArchive::~ZipArchive()
{
	if(IfValid()) {
		zip_close(archive);
		archive = nullptr;
	}
}

zip_t * ZipArchive::Get() const
{
	return archive;
}

bool ZipArchive::IfValid() const
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
	if(!p_file_handle->IfValid()) {
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
	if(!p_file_map_handle->IfValid()) {
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
	if(!p_map_view->IfValid()) {
		fmt::println("-- Failed to map view of file: {} --", GetLastError());
		return {};
	}
	shared_resources.pMapView = std::move(p_map_view);

	shared_resources.ifValid = true;
	return shared_resources;
}

#endif


//linux API
#if defined(__linux__)


//FileDescriptor部分

FileDescriptor::~FileDescriptor()
{
	if(IfValid()) {
		close(fileDescriptor);
		fileDescriptor = -1;
	}
}

int FileDescriptor::Get() const
{
	return fileDescriptor;
}

int FileDescriptor::Release()
{
	return std::exchange(fileDescriptor, -1);
}

bool FileDescriptor::IfValid() const
{
	if(fileDescriptor >= 0) {
		return true;
	} else {
		return false;
	}
}


//FileMap部分

FileMap::~FileMap()
{
	if(IfValid()) {
		munmap(mapAddr, length);
		mapAddr = MAP_FAILED;
	}
}

void * FileMap::GetAddr() const
{
	return mapAddr;
}

int FileMap::GetLen() const
{
	return length;
}

FileMap FileMap::Release()
{
	return FileMap(std::exchange(mapAddr, nullptr), std::exchange(length, 0));
}

bool FileMap::IfValid() const
{
	return mapAddr != MAP_FAILED && length > 0;
}

SharedResources init_shared_resources(std::string target_path)
{
	SharedResources shared_resources;

	//打开文件，获取文件描述符
	std::shared_ptr<FileDescriptor> p_file_descriptor = std::make_shared<FileDescriptor>(
		open(target_path.c_str(), O_RDONLY)
	);
	if(!p_file_descriptor->IfValid()) {
		fmt::println("-- Failed to open file --");
		return {};
	}
	shared_resources.pFileDescriptor = std::move(p_file_descriptor);

	//获取文件状态信息（文件大小）
	struct stat file_stat;
	if(fstat(shared_resources.pFileDescriptor->Get(), &file_stat) == -1 ){
		fmt::println("-- Failed to get file stat --");
		return {};
	}
	shared_resources.fileSize = file_stat.st_size;

	//将文件映射到虚拟地址空间
	std::shared_ptr<FileMap> p_file_map = std::make_shared<FileMap>(
		mmap(
			NULL,
			shared_resources.fileSize,
			PROT_READ,
			MAP_PRIVATE,
			shared_resources.pFileDescriptor->Get(),
			0
		)
	);
	if(!p_file_map->IfValid()) {
		fmt::println("-- Failed to map file --");
		return {};
	}
	shared_resources.pFileMap = std::move(p_file_map);

	shared_resources.ifValid = true;
	return shared_resources;
}

#endif


ZipArchive init_zip_archive(const SharedResources & shared_resources)
{
	zip_error_t err;
	zip_error_init(&err);

	LPVOID p_data_view = nullptr;

	//windows
#if defined(_WIN32)
	p_data_view = shared_resources.pMapView->Get();
#endif

	//linux
#if defined(__linux__)
	p_data_view = shared_resources.pFileMap->GetAddr();
#endif

	if(shared_resources.ifUseZipDataPtr) {
		p_data_view = shared_resources.pZipData;
	}

	//从内存缓冲区创建zip资源对象
	zip_source_t * p_zip_source = zip_source_buffer_create(
		p_data_view,
		shared_resources.fileSize,
		0,
		&err
	);
	if(p_zip_source == nullptr) {
		fmt::println("-- Failed to create zip source from buffer: {} --", zip_error_strerror(&err));
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

ZipArchive pre_init_zip_archive(SharedResources & shared_resources)
{
	zip_error_t err;
	zip_error_init(&err);

	void * p_file_map = nullptr;

	//windows
#if defined(_WIN32)
	p_file_map = shared_resources.pMapView->Get();
#endif

	//linux
#if defined(__linux__)
	p_file_map = shared_resources.pFileMap->GetAddr();
#endif

	//从内存缓冲区创建zip资源对象
	zip_source_t * p_zip_source = zip_source_buffer_create(
		p_file_map,
		shared_resources.fileSize,
		0,
		&err
	);
	if(p_zip_source == nullptr) {
		fmt::println("-- Failed to create zip source from buffer: {} --", zip_error_strerror(&err));
		return ZipArchive(nullptr);
	}

	//从zip资源对象打开zip文档对象
	zip_t * p_zip_archive = zip_open_from_source(
		p_zip_source,
		ZIP_RDONLY,
		&err
	);
	if(p_zip_archive == nullptr) {
		if(err.zip_err == ZIP_ER_NOZIP) {
			if(find_zip_data(shared_resources)) {
				return init_zip_archive(shared_resources);
			} else {
				fmt::println("-- Failed to find zip data --");
				return ZipArchive(nullptr);
			}
		} else {
			fmt::println("-- Failed to open zip archive from source: {} --", zip_error_strerror(&err));
			return ZipArchive(nullptr);
		}
	}

	return ZipArchive(p_zip_archive);
}

bool find_zip_data(SharedResources & shared_resources)
{
	//windows
#if defined(_WIN32)
	const char * p_char_data = static_cast<const char *>(shared_resources.pMapView->Get());
#endif

	//linux
#if defined(__linux__)
	const char * p_char_data = static_cast<const char *>(shared_resources.pFileMap->GetAddr());
#endif

	const char * zip_header = "\x50\x4B\x03\x04"; // 0x04034B50
	const char * zip_eocdr = "\x50\x4B\x05\x06"; // 0x06054B50
	std::string_view data_view(p_char_data, shared_resources.fileSize);

	//获取zip header位置
	auto header_pos = data_view.find(zip_header);
	if(header_pos == std::string_view::npos) {
		fmt::println("Failed to find header_pos");
		return false;
	}

	//获取zip eocdr位置
	auto eocdr_pos = data_view.rfind(zip_eocdr);
	if(eocdr_pos == std::string_view::npos) {
		fmt::println("Failed to find eocdr_pos");
		return false;
	}

	//检查eocdr注释长度段，获取注释长度
	if(eocdr_pos + 20 >= shared_resources.fileSize) {
		return false;
	}
	uint16_t comment_len = *(uint16_t *)(p_char_data + eocdr_pos + 20);

	//计算zip数据长度并验证
	zip_uint64_t zip_size = (eocdr_pos - header_pos) + 22 + comment_len;
	if(header_pos + zip_size > shared_resources.fileSize) {
		fmt::println("Invalid zip size when find zip data");
		return false;
	}

	shared_resources.pZipData = (LPVOID)(p_char_data + header_pos);
	shared_resources.fileSize = zip_size;
	shared_resources.ifUseZipDataPtr = true;
	return true;
}