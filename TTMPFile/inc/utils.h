#pragma once
#include <vector>
#include <string>

// This header contains some types and functions that might be commonly used

namespace Tt
{
	/*struct BoneSet
	{
		std::vector<int16_t> boneIndices;
		int32_t boneIndexCount;
	};

	struct BoneDataBlock
	{
		std::vector<int> bonePathOffsetList;
	};*/



	struct MaterialDataBlock
	{
		std::vector<int> materialPathOffsetList;
	};

	struct Vec2
	{
		float x = 0.0;
		float y = 0.0;
	};

	struct Vec3
	{
		float x = 0.0;
		float y = 0.0;
		float z = 0.0;
	};

	struct Vec4
	{
		float x = 0.0;
		float y = 0.0;
		float z = 0.0;
		float w = 0.0;
	};

	std::ostream &operator<< (std::ostream &os, const Vec2 &v);
	std::ostream &operator<< (std::ostream &os, const Vec3 &v);
	std::ostream &operator<< (std::ostream &os, const Vec4 &v);


	struct Color4
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		uint8_t a = 0;
	};

	struct BoundingBox
	{
		std::vector<Vec4> pointList;
	};

	//typedef std::vector<Vec2> Vector2Collection;
	//typedef std::vector<Vec3> Vector3Collection;
	//typedef std::vector<Vec4> Vector4Collection;
	//typedef std::vector<Color> Color4Collection;

	// adopted from xivModdingFramework.Models.DataContainers.VertexData
	struct VertexData
	{
		std::vector<Vec3> positions;
		std::vector<std::vector<float>> boneWeights;
		std::vector<std::vector<uint8_t>> boneIndices;
		std::vector<Vec3> normals;
		std::vector<Vec3> binormals;
		std::vector<char> binormalHandedness;
		std::vector<Vec3> tangents;
		std::vector<Color4> colors;
		// std::vector<Color4> colors4; // ??
		std::vector<Vec2> textureCoordinates0;
		std::vector<Vec2> textureCoordinates1;
		std::vector<int> indices;
	};


	// adopted from xivModdingFramework.Models.Enums.VertexDataType
	enum class VertexDataType : uint8_t
	{
		Float1 = 0x0,
		Float2 = 0x1,
		Float3 = 0x2,
		Float4 = 0x3,
		Ubyte4 = 0x5,
		Short2 = 0x6,
		Short4 = 0x7,
		Ubyte4n = 0x8,
		Short2n = 0x9,
		Short4n = 0xA,
		Half2 = 0xD,
		Half4 = 0xE,
		Compress = 0xF
	};

	//std::map<uint8_t, VertexDataType> VertexTypeDictionary{
	//	{0x0, VertexDataType::Float1},
	//	{0x1, VertexDataType::Float2},
	//	{0x2, VertexDataType::Float3},
	//	{0x3, VertexDataType::Float4},
	//	{0x5, VertexDataType::Ubyte4},
	//	{0x6, VertexDataType::Short2},
	//	{0x7, VertexDataType::Short4},
	//	{0x8, VertexDataType::Ubyte4n},
	//	{0x9, VertexDataType::Short2n},
	//	{0xA, VertexDataType::Short4n},
	//	{0xD, VertexDataType::Half2},
	//	{0xE, VertexDataType::Half4},
	//	{0xF, VertexDataType::Compress}
	//};

	// adopted from xivModdingFramework.Models.Enums.VertexUsageType
	enum class VertexUsageType : uint8_t
	{
		Position = 0x0,
		BoneWeight = 0x1,
		BoneIndex = 0x2,
		Normal = 0x3,
		TextureCoordinate = 0x4,
		Tangent = 0x5,
		Binormal = 0x6,
		Color = 0x7
	};

	// adopted from xivModdingFramework.Models.DataContainers.VertexDataStruct
	struct VertexDataStruct
	{
		uint8_t dataBlock = 0;
		uint8_t dataOffset = 0;
		VertexDataType dataType = VertexDataType::Float3;
		VertexUsageType dataUsage = VertexUsageType::Position;
	};

	// adopted from xivModdingFramework.Models.DataContainers.MeshPart
	struct MeshPart
	{
		int32_t indexOffset = 0;
		int32_t indexCount = 0;
		uint32_t attributeBitmask = 0;
		int16_t boneStartOffset = 0;
		int16_t boneCount = 0;
	};

	// adopted from xivModdingFramework.Models.DataContainers.MeshDataInfo
	struct MeshDataInfo
	{
		int32_t vertexCount = 0;
		int32_t indexCount = 0;
		int16_t materialIndex = 0;
		int16_t meshPartIndex = 0;
		int16_t meshPartCount = 0;
		int16_t boneSetIndex = 0;
		int32_t indexDataOffset = 0;
		int32_t vertexDataOffset0 = 0;
		int32_t vertexDataOffset1 = 0;
		int32_t vertexDataOffset2 = 0;
		uint8_t vertexDataEntrySize0 = 0;
		uint8_t vertexDataEntrySize1 = 0;
		uint8_t vertexDataEntrySize2 = 0;
		uint8_t vertexDataBlockCount = 0;
	};

	struct MeshData
	{
		MeshDataInfo meshInfo;
		std::vector<MeshPart> meshPartList;
		std::vector<VertexDataStruct> vertexDataStructList;
		VertexData vertexData;
		bool isBody = false;
		std::vector<std::string> shapePathList;
	};

	struct LevelOfDetail
	{
		uint16_t meshOffset = 0;
		int16_t meshCount = 0;
		int32_t unknown0 = 0;
		int32_t unknown1 = 0;
		int16_t meshEnd = 0;
		int16_t extraMeshCount = 0;
		int16_t meshSum = 0;
		int16_t unknown2 = 0;
		int32_t unknown3 = 0;
		int32_t unknown4 = 0;
		int32_t unknown5 = 0;
		int32_t indexDataStart = 0;
		int32_t unknown6 = 0;
		int32_t unknown7 = 0;
		int32_t vertexDataSize = 0;
		int32_t indexDataSize = 0;
		int32_t vertexDataOffset = 0;
		int32_t indexDataOffset = 0;
		std::vector<MeshData> meshDataList;
	};

	// increase the pointer p until reaches 1 byte after the next byte of b
	static inline void goPastNextByteOf(char *&p, const char b)
	{
		while (*p++ != b);
	}
}
