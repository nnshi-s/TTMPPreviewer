//#pragma once
//#include <cstdint>
//#include <memory>
//
//namespace Tt
//{
//	// adopted from https://stackoverflow.com/questions/19016099/lookup-table-with-constexpr
//	template <int AnyNumber>
//	struct LookupTables
//	{
//		constexpr LookupTables() : rb5To8(), g5To8()
//		{
//			for (unsigned i = 0; i != 32; ++i)
//				rb5To8[i] = static_cast<uint8_t>(i * 255.0 / 31.0 + 0.5);
//			for (unsigned i = 0; i != 64; ++i)
//				g5To8[i] = static_cast<uint8_t>(i * 255.0 / 63.0 + 0.5);
//		}
//
//		uint8_t rb5To8[32];
//		uint8_t g5To8[64];
//	};
//
//	// adopted from xivModdingFramework.Helpers.DxtUtil
//	class DxtUtil
//	{
//	public:
//		static std::unique_ptr<char[]> decompressDxt1(const char *pSrc, int32_t width, int32_t height);
//		static DxtUtil& getSingleton();
//
//		static void convertRgb565ToRgb888(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b)
//		{
//			// RGB565 format uses 16 bits to store rgb color
//			// 15 14 13 12 11    10 09 08 07 06 05    04 03 02 01 00
//			// R  R  R  R  R 	 G  G  G  G  G  G	  B  B  B  B  B
//			// RGB888 format use 24 bits to store rgb color
//			// 23 22 21 20 19 18 17 16    15 14 13 12 11 10 09 08    07 06 05 04 03 02 01 00
//			// B  B  B  B  B  B  B  B     G  G  G  G  G  G  G  G     R  R  R  R  R  R  R  R
//			// However, color value in rgb565 has only 32 different values for r & b, 64 different values for g.
//			// so we need to scale r & b by 255/31, scale g by 255/63.
//			// we use a lookup table to speed this up.
//
//			r = color >> 11;
//			g = (color & 0x07E0) >> 5;
//			b = color & 0x001F;
//			r = LOOKUP_TABLE.rb5To8[r];
//			g = LOOKUP_TABLE.g5To8[g];
//			b = LOOKUP_TABLE.rb5To8[b];
//		}
//
//		// static void convertRgb565ToRgb888Dynamic(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b);
//		static constexpr auto LOOKUP_TABLE = LookupTables<1>();
//	private:
//		DxtUtil() = default;
//		~DxtUtil() = default;
//	public:
//		DxtUtil(const DxtUtil &) = delete;
//		DxtUtil(DxtUtil &&) = delete;
//		DxtUtil& operator=(const DxtUtil &) = delete;
//		DxtUtil& operator=(DxtUtil &&) = delete;
//	};
//}
