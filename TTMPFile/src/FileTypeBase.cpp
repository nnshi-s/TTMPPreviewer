#include "FileTypeBase.h"

#include <iostream>

namespace Tt
{
	bool FileTypeBase::readRawFromDisk(std::ifstream& ifs, const std::wstring& mpdPath, size_t offset, size_t size)
	{
		if (ifs.is_open()) ifs.close();
		ifs.open(mpdPath, std::ios::binary | std::ios::in);
		if (!ifs.good()) return false;
		ifs.seekg(offset);
		/*delete[] mpRawDataRaw;
		mpRawDataRaw = new char[size];*/
		mpRawData = std::shared_ptr<char>(new char[size], [](char *p) {delete[] p; });
		mpRawDataRaw = mpRawData.get();
		ifs.read(mpRawDataRaw, size);
		ifs.close();
		return true;
	}

	bool FileTypeBase::zLibDecompressData(z_stream& zLibStream, size_t currBlockNo)
	{
		if (inflateInit2(&zLibStream, -15) != Z_OK) // set zlib to use raw inflate.
		{
			std::cerr << "Error initializing zlib stream when decompressing block" << currBlockNo + 1
				<< ", " << zLibStream.msg << std::endl;
			return false;
		}
		if (inflate(&zLibStream, Z_FINISH) != Z_STREAM_END)
		{
			std::cerr << "Error decompressing block" << currBlockNo + 1
				<< ", " << zLibStream.msg << std::endl;
			return false;
		}
		if (inflateEnd(&zLibStream) != Z_OK)
		{
			std::cerr << "Error freeing zlib stream after decompressing block" << currBlockNo + 1
				<< ", " << zLibStream.msg << std::endl;
			return false;
		}
		return true;
	}

	bool FileTypeBase::zLibDecompressData(z_stream& zLibStream)
	{
		if (inflateInit2(&zLibStream, -15) != Z_OK) // set zlib to use raw inflate.
		{
			std::cerr << "Error initializing zlib stream: " << zLibStream.msg << std::endl;
			return false;
		}
		if (inflate(&zLibStream, Z_FINISH) != Z_STREAM_END)
		{
			std::cerr << "Error decompressing data" << zLibStream.msg << std::endl;
			return false;
		}
		if (inflateEnd(&zLibStream) != Z_OK)
		{
			std::cerr << "Error freeing zlib stream after decompressing" << zLibStream.msg << std::endl;
			return false;
		}
		return true;

	}
}
