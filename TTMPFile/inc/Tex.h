#pragma once
#include <fstream>
#include <string>
#include "FileTypeBase.h"
#include <map>
#include "utils.h"
#include <memory>

// related files & functions in xivModdingFramework
// class XivTex, class Tex, class Dat
// Dat.cs.GetType4Data() defines how to read .tex files from disk
namespace Tt
{
	enum class TexFormat : int32_t
	{
		L8 = 4400,
		A8 = 4401,
		A4R4G4B4 = 5184,
		A1R5G5B5 = 5185,
		A8R8G8B8 = 5200,
		X8R8G8B8 = 5201,
		R32F = 8528,
		G16R16F = 8784,
		G32R32F = 8800,
		A16B16G16R16F = 9312,
		A32B32G32R32F = 9328,
		DXT1 = 13344,
		DXT3 = 13360,
		DXT5 = 13361,
		D16 = 16704,
		INVALID = 0
	};


	class Tex final : public FileTypeBase
	{
	public:
		int32_t mHeaderLength = 0;
		int32_t mUncompressedFileSize = 0;
		int32_t mMipMapCount = 0;
		TexFormat mTexFormat = TexFormat::INVALID;
		int16_t mWidth = 0;
		int16_t mHeight = 0;
		int16_t mLayers = 0;
		char* mpDecompressedDataRaw = nullptr;
		std::shared_ptr<char> mpDecompressedData;

		bool loadFromRaw() override;
		size_t getDcmpArrSize() const { return mDcmpArrSize; }

		size_t getMip0TextureSize() const { return mPixelSizeMap.at(mTexFormat) * mWidth * mHeight / 8; }

		void print() const;
	private:
		// static const std::map<int32_t, TexFormat> mMapIntTexFormat;
		size_t mDcmpArrSize = 0;
		void addArrSize(const size_t size, char*& ptrToMove);
		static inline std::map<TexFormat, size_t> mPixelSizeMap{
			{TexFormat::L8, 8},
			{TexFormat::A8, 8},
			{TexFormat::A4R4G4B4, 16},
			{TexFormat::A1R5G5B5, 16},
			{TexFormat::A8R8G8B8, 32},
			{TexFormat::X8R8G8B8, 32},
			{TexFormat::G16R16F, 32},
			{TexFormat::G32R32F, 64},
			{TexFormat::A16B16G16R16F, 64},
			{TexFormat::A32B32G32R32F, 128},
			{TexFormat::DXT1, 4},
			{TexFormat::DXT3, 8},
			{TexFormat::DXT5, 8},
			{TexFormat::D16, 16},
			{TexFormat::INVALID, 32}
		};

	public:
		void convertARGB8ToBGRA8();
		void convertARGB8ToRGBA8();
		void setAlphaForARGB8(unsigned char alpha);
		Tex() = default;
		// Tex(const Tex&) = delete;
		// Tex(Tex&&) = delete;
		// Tex& operator=(const Tex&) = delete;
		// Tex& operator=(Tex&&) = delete;
	};
}
