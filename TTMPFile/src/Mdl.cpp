#include "Mdl.h"
#include "zlib.h"

namespace Tt
{
	
	//bool Mdl::readFromFile(const std::string &filePath)
	//{
	//#define STREAM_READ(_stream_,_member_) _stream_##.read(reinterpret_cast<char*>(&##_member_##),sizeof(##_member_##))
	//	std::ifstream mdlStream(filePath, std::ios::in | std::ios::binary);
	//	mdlStream.seekg(0);
	//	if (!mdlStream.good())
	//		return false;
	//
	//	STREAM_READ(mdlStream, mHeaderLength);
	//	STREAM_READ(mdlStream, mFileType);
	//	STREAM_READ(mdlStream, mDecompressedSize);
	//	STREAM_READ(mdlStream, mBuffer1);
	//	STREAM_READ(mdlStream, mBuffer2);
	//	STREAM_READ(mdlStream, mParts);
	//
	//	int16_t temp;
	//	STREAM_READ(mdlStream, temp);// skip 2 bytes
	//
	//	for (size_t i = 0; i != 11; ++i)
	//		STREAM_READ(mdlStream, mChunkUncompSizes[i]);
	//
	//	for (size_t i = 0; i != 11; ++i)
	//		STREAM_READ(mdlStream, mChunkLengths[i]);
	//
	//	for (size_t i = 0; i != 11; ++i)
	//		STREAM_READ(mdlStream, mChunkOffsets[i]);
	//
	//	for (size_t i = 0; i != 11; ++i)
	//		STREAM_READ(mdlStream, mChunkBlockStart[i]);
	//
	//	mTotalNumBlocks = 0;
	//	for (size_t i = 0; i != 11; ++i)
	//	{
	//		STREAM_READ(mdlStream, mChunkNumBlocks[i]);
	//		mTotalNumBlocks += mChunkNumBlocks[i];
	//	}
	//
	//
	//	STREAM_READ(mdlStream, mMeshCount);
	//	STREAM_READ(mdlStream, mMaterialCount);
	//
	//	STREAM_READ(mdlStream, temp); STREAM_READ(mdlStream, temp);// skip 4 bytes
	//
	//	mpBlockSizes = new uint16_t[mTotalNumBlocks];
	//	for (size_t i = 0; i != mTotalNumBlocks; ++i)
	//		STREAM_READ(mdlStream, mpBlockSizes[i]);
	//
	//	return true;
	//
	//#undef STREAM_READ
	//
	//}
	
	//bool Mdl::readRawFromDisk(std::ifstream &ifs, const std::wstring &mpdPath, size_t offset, size_t size)
	//{
	//	if (ifs.is_open()) ifs.close();
	//	ifs.open(mpdPath, std::ios::binary | std::ios::in);
	//	if (!ifs.good()) return false;
	//	ifs.seekg(offset);
	//	delete[] mpRawDataRaw;
	//	mpRawDataRaw = new char[size];
	//	ifs.read(mpRawDataRaw, size);
	//	ifs.close();
	//	return true;
	//}
	
	bool Mdl::loadFromRaw()
	{
		if (!mpRawDataRaw) return false;
		char *currentPos = &mpRawDataRaw[0];
	#define READ(_type_, _member_) _member_##=*reinterpret_cast<##_type_##*>(currentPos),currentPos+=sizeof(##_type_##)
		READ(int32_t, mHeaderLength);
		READ(int32_t, mFileType);
		READ(int32_t, mDecompressedSize);
		mDcmpArrSize = mDecompressedSize;
		READ(int32_t, mBuffer1);
		READ(int32_t, mBuffer2);
		READ(int16_t, mParts);
		currentPos += 0x2;
		for (size_t i = 0; i != 11; ++i) // macro expanded to 2 statements, don't forget {}
		{
			READ(int32_t, mChunkUncompSizes[i]);
		}
	
		for (size_t i = 0; i != 11; ++i)
		{
			READ(int32_t, mChunkLengths[i]);
		}
	
		for (size_t i = 0; i != 11; ++i)
		{
			READ(int32_t, mChunkOffsets[i]);
		}
	
		for (size_t i = 0; i != 11; ++i)
		{
			READ(uint16_t, mChunkBlockStart[i]);
		}
	
		mTotalNumBlocks = 0;
		for (size_t i = 0; i != 11; ++i)
		{
			READ(uint16_t, mChunkNumBlocks[i]);
			mTotalNumBlocks += mChunkNumBlocks[i];
		}
	
		READ(uint16_t, mMeshCount);
		READ(uint16_t, mMaterialCount);
	
		currentPos += 0x4;
	
		// each block contains compressed data
		// need to decompress all to one array
		delete[] mpBlockSizes;
		mpBlockSizes = new uint16_t[mTotalNumBlocks];
		for (size_t i = 0; i != mTotalNumBlocks; ++i)
		{
			READ(uint16_t, mpBlockSizes[i]);
		}
	
		currentPos = mpRawDataRaw + mHeaderLength + mChunkOffsets[0];
		mpDecompressedData = new char[mDecompressedSize];
	
		// initialize zlib stream
		z_stream zLibStream;
		zLibStream.zalloc = Z_NULL;
		zLibStream.zfree = Z_NULL; // todo : should wrap the decompression to a class
		
	
	
		char *pDst = mpDecompressedData;
		for (size_t i = 0; i != mTotalNumBlocks; ++i)
		{
			const int32_t partCompressedSize = *reinterpret_cast<int32_t *>(currentPos + 8);
			const int32_t partDecompressedSize = *reinterpret_cast<int32_t *>(currentPos + 8 + sizeof(int32_t));

			// check if there is enough space left
			if(pDst + partDecompressedSize - mpDecompressedData >= mDcmpArrSize )
			{
				addArrSize(mDcmpArrSize * 0.1, pDst);
			}
			
			char *pBlockStart = currentPos + 8 + 2 * sizeof(int32_t);
	
			if (partCompressedSize == 32000)
			{
				// data not compressed
				memcpy(pDst, pBlockStart, partDecompressedSize);
			}
			else
			{
				// decompress data
				zLibStream.next_in = reinterpret_cast<unsigned char *>(pBlockStart);
				zLibStream.avail_in = partCompressedSize;
				zLibStream.next_out = reinterpret_cast<unsigned char *>(pDst);
				zLibStream.avail_out = partDecompressedSize;
				zLibDecompressData(zLibStream, i);
			}
	
			pDst += partDecompressedSize;
			currentPos += mpBlockSizes[i];
		}
	
		return true;
	#undef READ
	}
	
	
	void Mdl::print() const
	{
	#define COUT(x) std::cout<<std::right<<std::setw(28)<<##x##<<std::left<<std::setw(0)
		COUT("mHeaderLength: ") << mHeaderLength << '\n';
		COUT("mFileType: ") << mFileType << '\n';
		COUT("mDecompressedSize: ") << mDecompressedSize << "(actual: "
			<< getActualDecompressedSize() << ")\n";
		COUT("mBuffer1: ") << mBuffer1 << '\n';
		COUT("mBuffer2: ") << mBuffer2 << '\n';
		COUT("mParts: ") << mParts << '\n';
	
		COUT("mChunkUncompSizes[11]: ") << "[";
		for (auto n : mChunkUncompSizes)
			std::cout << n << ", ";
		std::cout << "\b\b]\n";
	
		COUT("mChunkLengths[11]: ") << "[";
		for (auto n : mChunkLengths)
			std::cout << n << ", ";
		std::cout << "\b\b]\n";
	
		COUT("mChunkOffsets[11]: ") << "[";
		for (auto n : mChunkOffsets)
			std::cout << n << ", ";
		std::cout << "\b\b]\n";
	
		COUT("mChunkBlockStart[11]: ") << "[";
		for (auto n : mChunkBlockStart)
			std::cout << n << ", ";
		std::cout << "\b\b]\n";
	
		COUT("mChunkNumBlocks[11]: ") << "[";
		for (auto n : mChunkNumBlocks)
			std::cout << n << ", ";
		std::cout << "\b\b]\n";
	
		COUT("mMeshCount: ") << mMeshCount << '\n';
		COUT("mMaterialCount: ") << mMaterialCount << '\n';
	
		std::stringstream tempStream;
		tempStream << "mpBlockSizes[" << mTotalNumBlocks << "]: ";
		COUT(tempStream.str()) << "[";
		for (size_t i = 0; i != mTotalNumBlocks; ++i)
			std::cout << mpBlockSizes[i] << ", ";
		std::cout << "\b\b]\n";
	
		std::cout << std::endl;
	
	#undef COUT
	
	}
	
	// sum(partDecompressedSize of every block) != mDecompressedSize
	size_t Mdl::getActualDecompressedSize() const
	{
		if (!mpBlockSizes) return 0;
		char *pBlockStart = mpRawDataRaw + mHeaderLength + mChunkOffsets[0];
		size_t actualDecompressedSize = 0;
		for (size_t i = 0; i != mTotalNumBlocks; ++i)
		{
			actualDecompressedSize += *reinterpret_cast<int32_t *>(pBlockStart + 8 + sizeof(int32_t));
			pBlockStart += mpBlockSizes[i];
		}
		return actualDecompressedSize;
	}
	
	void Mdl::dumpDecompressedData() const
	{
		if (!mpDecompressedData) return;
		std::ofstream ofs("mpDecompressedData.dump", std::ios::binary | std::ios::out);
		ofs.write(mpDecompressedData, mDecompressedSize);
		ofs.close();
	}
	
	Mdl::~Mdl()
	{
		// delete[] mpRawDataRaw;
		delete[] mpDecompressedData;
	}

	void Mdl::addArrSize(const size_t size, char*& ptrToMove)
	{
		if (!mpDecompressedData || ptrToMove < mpDecompressedData || ptrToMove - mpDecompressedData > mDcmpArrSize)
			return;
		const size_t offset = ptrToMove - mpDecompressedData;

		char *temp = new char[mDcmpArrSize + size];
		memcpy(temp, mpDecompressedData, mDcmpArrSize);
		delete[] mpDecompressedData;
		mpDecompressedData = temp;
		temp = nullptr;

		mDcmpArrSize += size;
		ptrToMove = mpDecompressedData + offset;
	}
}

