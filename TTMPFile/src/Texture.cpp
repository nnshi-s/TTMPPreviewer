//#include "Texture.h"
//#include <algorithm>
//
//namespace Tt
//{
//	// adopted from xivModdingFramework.Textures.FileTypes::GetImageData()
//	bool Texture::loadFromTex(const Tex &tex)
//	{
//		mWidth = tex.mWidth;
//		mHeight = tex.mHeight;
//		mLayers = tex.mLayers == 0 ? 1 : tex.mLayers;
//		mpTextureData = tex.mpDecompressedData;
//		mTextureDataArrSize = tex.getDcmpArrSize();
//		return true;
//	}
//
//
//	Texture::Texture(const Tex& tex)
//		: mWidth(tex.mWidth), mHeight(tex.mHeight),
//			mLayers(tex.mLayers == 0 ? 1 : tex.mLayers),
//			mpTextureData(tex.mpDecompressedData),
//			mTextureDataArrSize(tex.getDcmpArrSize())
//	{
//		
//	}
//}
