#pragma once
#include <fstream>
#include <string>
#include <memory>
#include "zlib.h"
namespace Tt
{
	class FileTypeBase
	{
	public:
		bool readRawFromDisk(std::ifstream &ifs, const std::wstring& mpdPath, size_t offset, size_t size);
		virtual bool loadFromRaw() = 0;
		static bool zLibDecompressData(z_stream &zLibStream, size_t currBlockNo);
		static bool zLibDecompressData(z_stream &zLibStream);
		
		virtual ~FileTypeBase() = default;
	protected:
		char *mpRawDataRaw = nullptr;
		std::shared_ptr<char> mpRawData;
	public:
		FileTypeBase() = default;
		// FileTypeBase(const FileTypeBase &) = delete;
		// FileTypeBase(FileTypeBase &&) = delete;
		// FileTypeBase &operator=(const FileTypeBase &) = delete;
		// FileTypeBase &operator=(FileTypeBase &&) = delete;
	};
}
