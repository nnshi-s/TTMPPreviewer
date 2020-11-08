#include "Tex.h"
#include <iostream>
#include <iomanip>
#include "zlib.h"

namespace Tt
{
	/*const std::map<int32_t, TexFormat> Tex::mMapIntTexFormat{
	{4400, TexFormat::L8},
	{4401, TexFormat::A8},
	{5184, TexFormat::A4R4G4B4},
	{5185, TexFormat::A1R5G5B5},
	{5200, TexFormat::A8R8G8B8},
	{5201, TexFormat::X8R8G8B8},
	{8528, TexFormat::R32F},
	{8784, TexFormat::G16R16F},
	{8800, TexFormat::G32R32F},
	{9312, TexFormat::A16B16G16R16F},
	{9328, TexFormat::A32B32G32R32F},
	{13344, TexFormat::DXT1},
	{13360, TexFormat::DXT3},
	{13361, TexFormat::DXT5},
	{16704, TexFormat::D16}
};*/

// adopted from xivModdingFramework.SqPack.FileTypes.Dat::GetType4Data()
	bool Tex::loadFromRaw()
	{
#define READ(_type_, _member_) _member_##=*reinterpret_cast<##_type_##*>(currentPos),currentPos+=sizeof(##_type_##)
		char *currentPos = mpRawDataRaw;
		READ(int32_t, mHeaderLength);
		currentPos += sizeof(int32_t); // skip member fileType
		READ(int32_t, mUncompressedFileSize);
		currentPos += 2 * sizeof(int32_t); // skip ikd1 & ikd2
		READ(int32_t, mMipMapCount);

		const size_t endOfHeader = mHeaderLength;

		currentPos = mpRawDataRaw + endOfHeader + 4;

		READ(TexFormat, mTexFormat);
		READ(int16_t, mWidth);
		READ(int16_t, mHeight);
		READ(int16_t, mLayers);
		currentPos += sizeof(int16_t); // skip imageCount2

		mpDecompressedDataRaw = nullptr;

		mDcmpArrSize = mUncompressedFileSize * 1.05;
		// mUncompressedFileSize may not actually be big enough for decompressed data, may need to reallocate.
		mpDecompressedData = std::shared_ptr<char>(new char[mDcmpArrSize], [](char *p) {delete[] p; });
		mpDecompressedDataRaw = mpDecompressedData.get();
		char *currentDecompressedPos = mpDecompressedDataRaw;
		
		constexpr size_t mipMapInfoOffset = 24;
		for (int32_t i = 0, j = 0; i != mMipMapCount; ++i, j += 20)
		{
			currentPos = mpRawDataRaw + mipMapInfoOffset + j;
			int32_t offsetFromHeaderEnd = 0, mipMapParts = 0;
			READ(int32_t, offsetFromHeaderEnd);
			currentPos += 3 * sizeof(int32_t); // skip mipMapLength, mipMapSize, mipMapStart
			READ(int32_t, mipMapParts);

			//std::cout << "mipMap: " << i << "\t part: " << mipMapParts << std::endl;
			
			const size_t mipMapPartOffset = endOfHeader + offsetFromHeaderEnd;
			currentPos = mpRawDataRaw + mipMapPartOffset;

			currentPos += 8;
			int32_t compressedSize = 0, uncompressedSize = 0;
			READ(int32_t, compressedSize);
			READ(int32_t, uncompressedSize);
			// check if there is enough space left
			if(currentDecompressedPos + uncompressedSize - mpDecompressedDataRaw >= mDcmpArrSize)
			{
				addArrSize(mDcmpArrSize * 0.1, currentDecompressedPos);
			}

						
			// initialize zlib for decompression
			z_stream zLibStream;
			zLibStream.zalloc = Z_NULL;
			zLibStream.zfree = Z_NULL;

			if (mipMapParts > 1)
			{
				zLibStream.next_in = reinterpret_cast<unsigned char *>(currentPos);
				zLibStream.avail_in = compressedSize;
				zLibStream.next_out = reinterpret_cast<unsigned char *>(currentDecompressedPos);
				zLibStream.avail_out = uncompressedSize;

				zLibDecompressData(zLibStream); //

				currentPos += compressedSize;
				currentDecompressedPos += uncompressedSize;

				
				
				for (int32_t k = 0; k != mipMapParts; ++k)
				{
					/*
						char check = 0;
						READ(char, check);
						while (check != 0x10)
							READ(char, check);
							*/
				
					goPastNextByteOf(currentPos, 0x10);

					currentPos += 7;
					READ(int32_t, compressedSize);
					READ(int32_t, uncompressedSize);
					// check if there is enough space left
					if (currentDecompressedPos + uncompressedSize - mpDecompressedDataRaw >= mDcmpArrSize)
					{
						addArrSize(mDcmpArrSize * 0.1, currentDecompressedPos);
					}
					if (compressedSize != 32000)
					{
						zLibStream.next_in = reinterpret_cast<unsigned char *>(currentPos);
						zLibStream.avail_in = compressedSize;
						zLibStream.next_out = reinterpret_cast<unsigned char *>(currentDecompressedPos);
						zLibStream.avail_out = uncompressedSize;

						/*std::cout << "decmp space usage: "
							<< (double)(100.0 * (currentDecompressedPos - mpDecompressedDataRaw)) / mDcmpArrSize
							<< "c: " << compressedSize << "dc: " << uncompressedSize << std::endl;*/
						zLibDecompressData(zLibStream);
						currentPos += compressedSize;
						currentDecompressedPos += uncompressedSize;
					}
					else // not compressed
					{
						memcpy(currentDecompressedPos, currentPos, uncompressedSize);
						currentPos += uncompressedSize;
						currentDecompressedPos += uncompressedSize;
					}
				}
			}
			else
			{
				if (compressedSize != 32000)
				{
					zLibStream.next_in = reinterpret_cast<unsigned char *>(currentPos);
					zLibStream.avail_in = compressedSize;
					zLibStream.next_out = reinterpret_cast<unsigned char *>(currentDecompressedPos);
					zLibStream.avail_out = uncompressedSize;
					zLibDecompressData(zLibStream);

					currentPos += compressedSize;
					currentDecompressedPos += uncompressedSize;
				}
				else // not compressed
				{
					memcpy(currentDecompressedPos, currentPos, uncompressedSize);
					currentPos += uncompressedSize;
					currentDecompressedPos += uncompressedSize;
				}
			}
		}
		// fill the rest area with 0
		while (currentDecompressedPos - mpDecompressedDataRaw < mUncompressedFileSize)
		{
			*currentDecompressedPos++ = 0;
		}

		// mpDecompressedData = std::shared_ptr<char[]>(mpDecompressedDataRaw);

		return true;
#undef READ
	}

	void Tex::print() const
	{
#define COUT(x) std::cout<<std::right<<std::setw(28)<<##x##<<std::left<<std::setw(0)
		COUT("mHeaderLength: ") << mHeaderLength << '\n';
		COUT("mUncompressedFileSize: ") << mUncompressedFileSize << '\n';
		COUT("mMipMapCount: ") << mMipMapCount << '\n';
		COUT("mWidth: ") << mWidth << '\n';
		COUT("mHeight: ") << mHeight << '\n';
		COUT("mLayers: ") << mLayers << '\n';
		COUT("mDcmpArrSize: ") << mDcmpArrSize << '\n';

		std::map<TexFormat, std::string> texFormatMap{
			{TexFormat::L8, "L8"},
			{TexFormat::A8, "A8"},
			{TexFormat::A4R4G4B4, "A4R4G4B4"},
			{TexFormat::A1R5G5B5, "A1R5G5B5"},
			{TexFormat::A8R8G8B8, "A8R8G8B8"},
			{TexFormat::X8R8G8B8, "X8R8G8B8"},
			{TexFormat::R32F, "R32F"},
			{TexFormat::G16R16F, "G16R16F"},
			{TexFormat::G32R32F, "G32R32F"},
			{TexFormat::A16B16G16R16F, "A16B16G16R16F"},
			{TexFormat::A32B32G32R32F, "A32B32G32R32F"},
			{TexFormat::DXT1, "DXT1"},
			{TexFormat::DXT3, "DXT3"},
			{TexFormat::DXT5, "DXT5"},
			{TexFormat::DXT5, "DXT5"},
			{TexFormat::D16, "D16"},
			{TexFormat::INVALID, "INVALID"}
		};

		COUT("mTexFormat: ") << texFormatMap.at(mTexFormat) << '\n';

#undef COUT
	}

	void Tex::addArrSize(const size_t size, char *&ptrToMove)
	{
		if (!mpDecompressedDataRaw || ptrToMove < mpDecompressedDataRaw || ptrToMove - mpDecompressedDataRaw > mDcmpArrSize)
			return;
		const size_t offset = ptrToMove - mpDecompressedDataRaw;

		// char *temp = new char[mDcmpArrSize + size];
		std::shared_ptr<char> temp = std::shared_ptr<char>(new char[mDcmpArrSize + size], [](char *p) {delete[] p; });
		memcpy(temp.get(), mpDecompressedDataRaw, mDcmpArrSize);
		// delete[] mpDecompressedDataRaw;
		mpDecompressedDataRaw = temp.get();
		mpDecompressedData = temp;
		// temp = nullptr;

		mDcmpArrSize += size;
		ptrToMove = mpDecompressedDataRaw + offset;

		// std::cout << "ReAllocated: from " << mDcmpArrSize - size << " to " << mDcmpArrSize << std::endl;
	}

	void Tex::convertARGB8ToBGRA8()
	{
		for(auto *currentPos = mpDecompressedDataRaw; 
			currentPos != mpDecompressedDataRaw + mWidth * mHeight *4; currentPos += 4)
		{
			char a = *currentPos;
			char r = *(currentPos+1);
			char g = *(currentPos+2);
			char b = *(currentPos+3);

			*currentPos = b;
			*(currentPos + 1) = g;
			*(currentPos + 2) = r;
			*(currentPos + 3) = a;
		}
	}

	void Tex::convertARGB8ToRGBA8()
	{
		for (auto *currentPos = mpDecompressedDataRaw;
			currentPos != mpDecompressedDataRaw + mWidth * mHeight * 4; currentPos += 4)
		{
			char a = *currentPos;
			
			*currentPos = *(currentPos + 1);
			*(currentPos + 1) = *(currentPos + 2);
			*(currentPos + 2) = *(currentPos + 3);
			*(currentPos + 3) = a;
		}
	}

	void Tex::setAlphaForARGB8(unsigned char alpha)
	{
		for (auto *currentPos = mpDecompressedDataRaw;
			currentPos != mpDecompressedDataRaw + mWidth * mHeight * 4; currentPos += 4)
		{
			*reinterpret_cast<unsigned char *>(currentPos + 3) = alpha;
		}
	}
}
