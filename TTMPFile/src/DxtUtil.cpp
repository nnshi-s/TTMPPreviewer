//#include "DxtUtil.h"
//
//#define READ(_type_) *reinterpret_cast<##_type_##*>(currentPos);currentPos+=sizeof(##_type_##)
//
//namespace Tt
//{
//
//
//	std::unique_ptr<char[]> DxtUtil::decompressDxt1(const char *pSrc, int32_t width, int32_t height)
//	{
//		char *decompressed = new char[static_cast<size_t>(width) * height * 4];
//		char *currentPos = const_cast<char *>(pSrc);
//
//		// divide image into 4x4 blocks
//		const int blockCountX = (width + 3) / 4, blockCountY = (height + 3) / 4;
//		for (int y = 0; y != blockCountY; ++y)
//		{
//			for (int x = 0; x != blockCountX; ++x)
//			{
//				// Decompress 1 block, c0 & c1 are in 
//				uint16_t c0 = READ(uint16_t);
//				uint16_t c1 = READ(uint16_t);
//
//				uint8_t r0, g0, b0;
//				uint8_t r1, g1, b1;
//				convertRgb565ToRgb888(c0, r0, g0, b0);
//				convertRgb565ToRgb888(c1, r1, g1, b1);
//
//				uint32_t lookupTable = READ(uint32_t);
//				// every 2 bits in lookupTable forms an index.
//				// 16 indices in total.
//				// the value of index decides how to combine r0/g0/b0 and r1/g1/b1 to get the final r/g/b
//
//				// +-------+-------------------+-------------------+
//				// | index |  r(when c0 > c1)  | r(when c0 <= c1)  |
//				// +-------+-------------------+-------------------+
//				// |    00 | r0                | r0                |
//				// |    01 | r1                | r1                |
//				// |    10 | (2/3)r0 + (1/3)r1 | (1/2)r0 + (1/2)r1 |
//				// |    11 | (1/3)r0 + (2/3)r1 | 0                 |
//				// +-------+-------------------+-------------------+
//
//
//
//				// for every pixel in the block todo: 是否可以提速？（将rgba放在一个32位整数中一起操作）
//				for (int pixelY = 0; pixelY != 4; ++pixelY)
//				{
//					for (int pixelX = 0; pixelX != 4; ++pixelX)
//					{
//						uint8_t r = 0, g = 0, b = 0, a = 255;
//						const uint8_t index = (lookupTable >> 2 * (4 * pixelY + pixelX)) & 0x03;
//						switch (index)
//						{
//						case 0:
//							r = r0;
//							g = g0;
//							b = b0;
//							break;
//						case 1:
//							r = r1;
//							g = g1;
//							b = b1;
//							break;
//						case 2:
//							r = c0 > c1 ? (2 * r0 + r1) / 3 : (r0 + r1) / 2;
//							g = c0 > c1 ? (2 * g0 + g1) / 3 : (g0 + g1) / 2;
//							b = c0 > c1 ? (2 * b0 + b1) / 3 : (b0 + b1) / 2;
//							break;
//						case 3:
//							r = c0 > c1 ? (r0 + 2 * r1) / 3 : 0;
//							g = c0 > c1 ? (g0 + 2 * g1) / 3 : 0;
//							b = c0 > c1 ? (b0 + 2 * b1) / 3 : 0;
//							break;
//						default: break;
//						}
//
//						const int px = 4 * x + pixelX;
//						const int py = 4 * y + pixelY;
//						if (px < width && py < height)
//						{
//							const int offset = 4 * ((py * width) + px);
//							decompressed[offset] = r;
//							decompressed[offset + 1] = g;
//							decompressed[offset + 2] = b;
//							decompressed[offset + 3] = a;
//						}
//					}
//				}
//
//			}
//		}
//
//		return std::unique_ptr<char[]>(decompressed);
//	}
//
//	DxtUtil &DxtUtil::getSingleton()
//	{
//		static DxtUtil singleton;
//		return singleton;
//	}
//
//
//	//void DxtUtil::convertRgb565ToRgb888Dynamic(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b)
//	//{
//	//	r = (color >> 11) * 255.0 / 31.0;
//	//	g = ((color & 0x07E0) >> 5) * 255.0 / 63.0;
//	//	b = (color & 0x001F) * 255.0 / 31.0;
//	//}
//}
//
//#undef READ
