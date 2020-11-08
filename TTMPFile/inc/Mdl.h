#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include "FileTypeBase.h"

namespace Tt
{
	class Mdl final : public FileTypeBase
	{
	public:
		int32_t mHeaderLength			= 0;
		int32_t mFileType				= 0;
		int32_t mDecompressedSize		= 0;
		int32_t mBuffer1					= 0;
		int32_t mBuffer2					= 0;
		int16_t mParts					= 0;
		int32_t mChunkUncompSizes[11]	= { 0 };
		int32_t mChunkLengths[11]		= { 0 };
		int32_t mChunkOffsets[11]		= { 0 };
		uint16_t mChunkBlockStart[11]	= { 0 };
		uint16_t mChunkNumBlocks[11]		= { 0 };
		uint16_t mMeshCount				= 0;
		uint16_t mMaterialCount			= 0;
		uint16_t *mpBlockSizes			= nullptr;
		char *mpDecompressedData		= nullptr;
	
		bool loadFromRaw() override;
		void print() const;
		size_t getActualDecompressedSize() const;
		void dumpDecompressedData() const;
		~Mdl() override;
	private:
		size_t mTotalNumBlocks				= 0;
		size_t mDcmpArrSize = 0;
		void addArrSize(const size_t size, char *&ptrToMove);
	public:
		Mdl() = default;
		Mdl(const Mdl &) = delete;
		Mdl(Mdl &&) = delete;
		Mdl &operator=(const Mdl &) = delete;
		Mdl &operator=(Mdl &&) = delete;
	};
}