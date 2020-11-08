//#pragma once
//#include "Tex.h"
//#include <memory>
//
//namespace Tt
//{
//	// the data that class Tex contains is already in hardware compressed format.(DXT1 .etc)
//	// OGRE naturally support all those texture formats. no need to do further decompression.
//	// just do some simple copy here.
//	class Texture
//	{
//	public:
//		int16_t mWidth = 0;
//		int16_t mHeight = 0;
//		int16_t mLayers = 0;
//		TexFormat mTextureFormat = TexFormat::INVALID;
//		std::shared_ptr<char[]> mpTextureData;
//		size_t mTextureDataArrSize = 0;
//		
//		bool loadFromTex(const Tex &tex);
//
//		Texture() = default;
//		Texture(const Tex &tex);
//		~Texture() = default;
//		Texture(const Texture &) = delete;
//		Texture(Texture &&) = delete;
//		Texture &operator=(const Texture &) = delete;
//		Texture &operator=(Texture &&) = delete;
//	};
//}
