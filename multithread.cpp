#include "multithread.h"

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
		fmt::println("Failed to create zip source from buffer: {}", zip_error_strerror(&err));
		return ZipArchive(nullptr);
	}

	//从zip资源对象打开zip文档对象
	zip_t * p_zip_archive = zip_open_from_source(
		p_zip_source,
		ZIP_RDONLY,
		&err
	);
	if(p_zip_archive == nullptr) {
		fmt::println("Failed to open zip archive from source: {} --", zip_error_strerror(&err));
		return ZipArchive(nullptr);
	}

	return ZipArchive(p_zip_archive);
}
