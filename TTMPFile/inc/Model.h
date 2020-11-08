#pragma once
#include "Mdl.h"
#include <iomanip>
#include "utils.h"

namespace Tt
{
	class Model
	{
	public:

		bool loadFromMdl(const Mdl &mdl);
		std::vector<std::string> mMaterialList;
		std::vector<std::string> mShapeList;
		std::vector<LevelOfDetail> mLoDList;

		~Model();
	private:
		MaterialDataBlock mMatDataBLock;
		BoundingBox mBoundBox;
		uint8_t mPaddingSize = 0;
		char *mPaddingBytes = nullptr;
		int32_t mPathBlockSize = 0;
		int16_t mMeshCount = 0;
		int16_t mAttributeCount = 0;
		int16_t mMeshPartCount = 0;
		int16_t mMaterialCount = 0;
		int16_t mBoneCount = 0;
		int16_t mBoneListCount = 0;
		int16_t mShapeCount = 0;
		int16_t mShapePartCount = 0;
		uint16_t mShapeDataCount = 0;
		int16_t unknown1 = 0;
		int16_t unknown2 = 0;
		int16_t unknown3 = 0;
		int16_t unknown4 = 0;
		int16_t unknown5 = 0;
		int16_t unknown6 = 0;
		int16_t unknown7 = 0;
		int16_t unknown8 = 0;
		int16_t unknown9 = 0;
		char unknown10a = 0;
		char unknown10b = 0;
		int16_t unknown11 = 0;
		int16_t unknown12 = 0;
		int16_t unknown13 = 0;
		int16_t unknown14 = 0;
		int16_t unknown15 = 0;
		int16_t unknown16 = 0;
		int16_t unknown17 = 0;

		static const VertexDataStruct *getVertexDataStructFor(const MeshData &md, VertexUsageType usageType);
		static void setVertexData(const MeshData &meshData, const VertexDataStruct *pDataStruct, int32_t &vertexDataOffset, int32_t &vertexDataSize);

	public:
		Model() = default;
		Model(const Model &) = delete;
		Model(Model &&) noexcept = delete;
		Model &operator=(const Model &) = delete;
		Model &operator=(Model &&) noexcept = delete;

	};
}

