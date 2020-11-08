#include "Model.h"
#include "half.hpp"


namespace Tt
{
#define READ(_type_, _member_) _member_##=*reinterpret_cast<##_type_##*>(currentPos),currentPos+=sizeof(##_type_##)
#define POS_OFFSET currentPos-mdl.mpDecompressedData
#define PRINTCURRPOS std::cout<< "currentPosOffset: 0x"<<std::hex<< \
	POS_OFFSET<<'('<<std::dec<<POS_OFFSET<<')'<<std::endl
	

	std::ostream &operator<<(std::ostream &os, const Vec4 &v)
	{
		os << std::fixed << std::setprecision(2);
		os << '('
			<< v.x << ", "
			<< v.y << ", "
			<< v.z << ", "
			<< v.w << ')';
		os.unsetf(std::ios::fixed);
		os.precision(6);
		return os;
	}

	std::ostream &operator<<(std::ostream &os, const Vec3 &v)
	{
		os << std::fixed << std::setprecision(2);
		os << '('
			<< v.x << ", "
			<< v.y << ", "
			<< v.z << ")";
		os.unsetf(std::ios::fixed);
		os.precision(6);
		return os;
	}

	std::ostream &operator<<(std::ostream &os, const Vec2 &v)
	{
		os << std::fixed << std::setprecision(2);
		os << '('
			<< v.x << ", "
			<< v.y << ")";
		os.unsetf(std::ios::fixed);
		os.precision(6);
		return os;
	}

	bool Model::loadFromMdl(const Mdl &mdl)
	{
		if (!mdl.mpDecompressedData) return false;
		char *currentPos = mdl.mpDecompressedData;
		// skip vertex data structures
		// in the source of textools, this is 64 + 136 * meshCount + 4.
		// probably means in textools there is a extra 68-byte header.
		currentPos += 136 * static_cast<size_t>(mdl.mMeshCount);

		// skip pathCount(int32)
		currentPos += sizeof(int32_t);
		READ(int32_t, mPathBlockSize);

		// skip pathBlock and the unknown(int32) after it for now
		currentPos += mPathBlockSize + sizeof(int32_t);
		READ(int16_t, mMeshCount);
		READ(int16_t, mAttributeCount);
		READ(int16_t, mMeshPartCount);
		READ(int16_t, mMaterialCount);
		READ(int16_t, mBoneCount);
		READ(int16_t, mBoneListCount);
		READ(int16_t, mShapeCount);
		READ(int16_t, mShapePartCount);
		READ(uint16_t, mShapeDataCount);

		// reposition to and read the pathBlock
		char *savePos = currentPos;
		currentPos = mdl.mpDecompressedData + 136 * static_cast<size_t>(mdl.mMeshCount) + 2 * sizeof(int32_t);
		// skip attributes
		for (int16_t i = 0; i != mAttributeCount; ++i)
			goPastNextByteOf(currentPos, '\0');

		// skip Bones
		for (int16_t i = 0; i != mBoneCount; ++i)
			goPastNextByteOf(currentPos, '\0');

		// read material names
		mMaterialList.reserve(static_cast<size_t>(mMaterialCount) + 1);
		mShapeList.reserve(static_cast<size_t>(mMaterialCount) + mShapeCount);
		for (int16_t i = 0; i != mMaterialCount; ++i)
		{
			std::string materialName(currentPos);
			if (materialName.rfind("shp_", 0) == 0) // if material name starts with "shp_"
				mShapeList.push_back(std::move(materialName));
			else
				mMaterialList.push_back(std::move(materialName));
			goPastNextByteOf(currentPos, '\0');
		}

		// read shape names
		for (int16_t i = 0; i != mShapeCount; ++i)
		{
			mShapeList.emplace_back(currentPos);
			goPastNextByteOf(currentPos, '\0');
		}

		// skip remaining pathData
		// restore position and continue reading
		currentPos = savePos;
		// read unknown1-17
		READ(int16_t, unknown1);
		// unknown 1-17 from xivModdingFramework.Models.FileTypes.Mdl.cs.GetRawMdlData().mdlModelData
		READ(int16_t, unknown2);
		READ(int16_t, unknown3);
		READ(int16_t, unknown4);
		READ(int16_t, unknown5);
		READ(int16_t, unknown6);
		READ(int16_t, unknown7);
		READ(int16_t, unknown8);
		READ(int16_t, unknown9);
		READ(char, unknown10a);
		READ(char, unknown10b);
		READ(int16_t, unknown11);
		READ(int16_t, unknown12);
		READ(int16_t, unknown13);
		READ(int16_t, unknown14);
		READ(int16_t, unknown15);
		READ(int16_t, unknown16);
		READ(int16_t, unknown17);

		// skip UnknownData0
		currentPos += 32 * static_cast<size_t>(unknown2);
		// from xivModdingFramework.Models.FileTypes.Mdl.cs.GetRawMdlData().unkData0

		// read LODs
		mLoDList.reserve(4);
		for (size_t i = 0; i != 3; ++i)
		{
			LevelOfDetail lod;
			READ(uint16_t, lod.meshOffset);
			READ(int16_t, lod.meshCount);
			READ(int32_t, lod.unknown0);
			READ(int32_t, lod.unknown1);
			READ(int16_t, lod.meshEnd);
			READ(int16_t, lod.extraMeshCount);
			READ(int16_t, lod.meshSum);
			READ(int16_t, lod.unknown2);
			READ(int32_t, lod.unknown3);
			READ(int32_t, lod.unknown4);
			READ(int32_t, lod.unknown5);
			READ(int32_t, lod.indexDataStart);
			READ(int32_t, lod.unknown6);
			READ(int32_t, lod.unknown7);
			READ(int32_t, lod.vertexDataSize);
			READ(int32_t, lod.indexDataSize);
			READ(int32_t, lod.vertexDataOffset);
			READ(int32_t, lod.indexDataOffset);
			mLoDList.push_back(std::move(lod));
		}

		// now we have the lod data, go back to read Vertex Data Structures
		savePos = currentPos;
		currentPos = mdl.mpDecompressedData;
		const uint32_t vertexDataStructureSize = 136;
		// for each mesh in each lod
		uint32_t lodStructPos = 0;
		for (uint16_t i = 0; i != mLoDList.size(); ++i)
		{
			uint16_t totalMeshCount = static_cast<uint16_t>(mLoDList[i].meshCount) + static_cast<uint16_t>(mLoDList[i].
				extraMeshCount);
			mLoDList[i].meshDataList.reserve(totalMeshCount + 1);
			for (uint16_t j = 0; j != totalMeshCount; ++j)
			{
				currentPos = mdl.mpDecompressedData + static_cast<uint64_t>(j) * vertexDataStructureSize + lodStructPos;
				MeshData meshData;
				meshData.vertexDataStructList.reserve(18);
				while (*reinterpret_cast<uint8_t *>(currentPos) != 255)
				{
					VertexDataStruct vds;
					READ(uint8_t, vds.dataBlock);
					READ(uint8_t, vds.dataOffset);
					READ(VertexDataType, vds.dataType);
					READ(VertexUsageType, vds.dataUsage);

					meshData.vertexDataStructList.push_back(vds);
					currentPos += 4;
				}
				mLoDList[i].meshDataList.push_back(std::move(meshData));
			}
			lodStructPos += 136 * mLoDList[i].meshCount;
		}

		// finished reading vertex data. go back to saved position
		currentPos = savePos;
		// PRINTCURRPOS;
		uint32_t meshNum = 0; // not sure the function of this variable
		for (auto &lod : mLoDList)
		{
			uint16_t totalMeshCount = static_cast<uint16_t>(lod.meshCount) + static_cast<uint16_t>(lod.extraMeshCount);
			for (uint16_t i = 0; i != totalMeshCount; ++i)
			{
				READ(int32_t, lod.meshDataList[i].meshInfo.vertexCount);
				READ(int32_t, lod.meshDataList[i].meshInfo.indexCount);

				READ(int16_t, lod.meshDataList[i].meshInfo.materialIndex);
				READ(int16_t, lod.meshDataList[i].meshInfo.meshPartIndex);
				READ(int16_t, lod.meshDataList[i].meshInfo.meshPartCount);
				READ(int16_t, lod.meshDataList[i].meshInfo.boneSetIndex);

				READ(int32_t, lod.meshDataList[i].meshInfo.indexDataOffset);
				READ(int32_t, lod.meshDataList[i].meshInfo.vertexDataOffset0);
				READ(int32_t, lod.meshDataList[i].meshInfo.vertexDataOffset1);
				READ(int32_t, lod.meshDataList[i].meshInfo.vertexDataOffset2);

				READ(uint8_t, lod.meshDataList[i].meshInfo.vertexDataEntrySize0);
				READ(uint8_t, lod.meshDataList[i].meshInfo.vertexDataEntrySize1);
				READ(uint8_t, lod.meshDataList[i].meshInfo.vertexDataEntrySize2);
				READ(uint8_t, lod.meshDataList[i].meshInfo.vertexDataBlockCount);

				// In the event we have a null material reference, set it to material 0 to be safe.
				if (static_cast<size_t>(lod.meshDataList[i].meshInfo.materialIndex) >= mMaterialList.size())
					lod.meshDataList[i].meshInfo.materialIndex = 0;

				std::string materialString = mMaterialList[lod.meshDataList[i].meshInfo.materialIndex];

				try
				{
					if (materialString[4] == 'c' && materialString[9] == 'b')
						lod.meshDataList[i].isBody = true;
				}
				catch (const std::exception &)
				{
				}

				++meshNum;
			}
		}
		// skip attribute data blocks
		currentPos += mAttributeCount * sizeof(int32_t);

		// read MeshParts
		for (auto &lod : mLoDList)
		{
			for (auto &meshData : lod.meshDataList)
			{
				meshData.meshPartList.reserve(meshData.meshInfo.meshPartCount + 1);
				MeshPart mp;
				for (auto i = 0; i != meshData.meshInfo.meshPartCount; ++i)
				{
					READ(int32_t, mp.indexOffset);
					READ(int32_t, mp.indexCount);
					READ(uint32_t, mp.attributeBitmask);
					READ(int16_t, mp.boneStartOffset);
					READ(int16_t, mp.boneCount);

					meshData.meshPartList.push_back(mp);
				}
			}
		}

		// skip UnkData2
		currentPos += 12 * static_cast<size_t>(unknown9);

		// Data block for materials
		// Currently unknown usage
		mMatDataBLock.materialPathOffsetList.reserve(mMaterialCount + 1);
		for (int16_t i = 0; i != mMaterialCount; ++i)
		{
			int32_t a;
			READ(int32_t, a);
			mMatDataBLock.materialPathOffsetList.push_back(a);
		}
		// skip data block for bones
		currentPos += mBoneCount * sizeof(int32_t);
		// skip bone lists info
		currentPos += mBoneListCount * (64 * sizeof(int16_t) + sizeof(int32_t));
		// skip shape info
		currentPos += mShapeCount * (sizeof(int32_t) + 2 * mLoDList.size() * sizeof(uint16_t));
		currentPos += mShapePartCount * (3 * sizeof(int32_t)); // shape index info
		currentPos += mShapeDataCount * (2 * sizeof(uint16_t));

		// std::cout << "bone index: " << std::hex << currentPos - mdl.mpDecompressedData << std::endl;
		// skip bone index for parts
		int32_t boneIndexCount;
		READ(int32_t, boneIndexCount);
		currentPos += static_cast<size_t>(boneIndexCount / 2) * sizeof(int16_t);
		// std::cout << "padding: " << std::hex << currentPos - mdl.mpDecompressedData << std::endl;
		// padding
		READ(uint8_t, mPaddingSize);
		mPaddingBytes = new char[mPaddingSize];
		for (uint8_t i = 0; i != mPaddingSize; ++i)
			READ(char, mPaddingBytes[i]);
		// std::cout << "bounding box: " << std::hex << currentPos - mdl.mpDecompressedData << std::endl;
		// bounding box
		mBoundBox.pointList.reserve(9);
		for (uint8_t i = 0; i != 8; ++i)
		{
			Vec4 point;
			READ(float, point.x);
			READ(float, point.y);
			READ(float, point.z);
			READ(float, point.w);
			mBoundBox.pointList.push_back(point);
			// std::cout << point << std::endl;
		}

		// skip bone transform data
		int16_t transformCount = mBoneCount;
		if (transformCount == 0)
			transformCount = unknown8;

		currentPos += static_cast<size_t>(transformCount) * 2 * 4 * sizeof(float);

		savePos = currentPos;

		// start reading actual useful model data
		// vertex position
		int32_t lodNum = 0, totalMeshNum = 0;
		for (auto &lod : mLoDList)
		{
			if (lod.meshCount == 0) continue;
			auto *meshDataList = &lod.meshDataList;
			if (lod.meshOffset != totalMeshNum)
				meshDataList = &(mLoDList[lodNum + 1].meshDataList);
			for (auto &meshData : *meshDataList)
			{
				VertexData vd;
				int32_t vertexDataOffset = 0;
				int32_t vertexDataSize = 0;
				// Get the Vertex Data Structure for positions
				// get the vertexDataStruct for position
				const VertexDataStruct *pPosDataStruct = getVertexDataStructFor(meshData, VertexUsageType::Position);
				if (pPosDataStruct)
				{
					setVertexData(meshData, pPosDataStruct, vertexDataOffset, vertexDataSize);
					meshData.vertexData.positions.reserve(meshData.meshInfo.vertexCount + 1);
					for (int32_t i = 0; i != meshData.meshInfo.vertexCount; ++i)
					{
						int32_t positionOffset = lod.vertexDataOffset + vertexDataOffset + pPosDataStruct->dataOffset + vertexDataSize * i;
						// the decompressed data of textools has a header of 68 Bytes, here the decompressed data does not.
						positionOffset -= 68;
						currentPos = mdl.mpDecompressedData + positionOffset;

						Vec3 positionVector;
						// pPosDataStruct->dataType = VertexDataType::Half4; // for debug purpose
						if (pPosDataStruct->dataType == VertexDataType::Half4)
						{
							uint16_t temp;
							READ(uint16_t, temp);
							half_float::half coord(half_float::detail::binary_t{}, temp);
							positionVector.x = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							positionVector.y = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							positionVector.z = static_cast<float>(coord);

							READ(uint16_t, temp); // read w coordinate without storing it
						}
						else
						{
							READ(float, positionVector.x);
							READ(float, positionVector.y);
							READ(float, positionVector.z);
						}
						meshData.vertexData.positions.push_back(positionVector);
					}
				}

				// skip bone weights data & bone indices data

				// read normals
				// get the vertexDataStruct for normals
				const VertexDataStruct *pNormDataStruct = getVertexDataStructFor(meshData, VertexUsageType::Normal);
				if(pNormDataStruct)
				{
					setVertexData(meshData, pNormDataStruct, vertexDataOffset, vertexDataSize);
					// There is always one set of normals per vertex
					meshData.vertexData.normals.reserve(meshData.meshInfo.vertexCount + 1);
					for(int32_t i = 0; i != meshData.meshInfo.vertexCount; ++i)
					{
						int32_t normOffset = lod.vertexDataOffset + vertexDataOffset + pNormDataStruct->dataOffset + vertexDataSize * i;
						normOffset -= 68;
						currentPos = mdl.mpDecompressedData + normOffset;

						Vec3 normalVector;
						if(pNormDataStruct->dataType == VertexDataType::Half4)
						{
							uint16_t temp;
							READ(uint16_t, temp);
							half_float::half coord(half_float::detail::binary_t{}, temp);
							normalVector.x = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							normalVector.y = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							normalVector.z = static_cast<float>(coord);

							READ(uint16_t, temp); // read w coordinate without storing it
						}
						else
						{
							READ(float, normalVector.x);
							READ(float, normalVector.y);
							READ(float, normalVector.z);
						}
						meshData.vertexData.normals.push_back(normalVector);
					}
				}

				// read binormals
				const VertexDataStruct *pBinormDataStruct = getVertexDataStructFor(meshData, VertexUsageType::Binormal);
				if(pBinormDataStruct)
				{
					setVertexData(meshData, pBinormDataStruct, vertexDataOffset, vertexDataSize);
					meshData.vertexData.binormals.reserve(meshData.meshInfo.vertexCount + 1);
					meshData.vertexData.binormalHandedness.reserve(meshData.meshInfo.vertexCount + 1);
					for(int32_t i = 0; i != meshData.meshInfo.vertexCount; ++i)
					{
						int32_t binormOffset = lod.vertexDataOffset + vertexDataOffset + pBinormDataStruct->dataOffset + vertexDataSize * i;
						binormOffset -= 68;
						currentPos = mdl.mpDecompressedData + binormOffset;

						Vec3 binormVector;
						char temp;
						READ(char, temp);
						binormVector.x = temp * 2 / 255.0f - 1.0f;
						READ(char, temp);
						binormVector.y = temp * 2 / 255.0f - 1.0f;
						READ(char, temp);
						binormVector.z = temp * 2 / 255.0f - 1.0f;
						READ(char, temp);

						meshData.vertexData.binormals.push_back(binormVector);
						meshData.vertexData.binormalHandedness.push_back(temp);
					}
				}

				// read tangents
				const VertexDataStruct *pTangentDataStruct = getVertexDataStructFor(meshData, VertexUsageType::Tangent);
				if(pTangentDataStruct)
				{
					setVertexData(meshData, pTangentDataStruct, vertexDataOffset, vertexDataSize);
					meshData.vertexData.tangents.reserve(meshData.meshInfo.vertexCount + 1);
					for(int32_t i = 0; i != meshData.meshInfo.vertexCount; ++i)
					{
						int32_t tangentOffset = lod.vertexDataOffset + vertexDataOffset + pTangentDataStruct->dataOffset + vertexDataSize * i;
						tangentOffset -= 68;
						currentPos = mdl.mpDecompressedData + tangentOffset;

						Vec3 tangentVector;
						char temp;
						READ(char, temp);
						tangentVector.x = temp * 2 / 255.0f - 1.0f;
						READ(char, temp);
						tangentVector.y = temp * 2 / 255.0f - 1.0f;
						READ(char, temp);
						tangentVector.z = temp * 2 / 255.0f - 1.0f;
						READ(char, temp);

						meshData.vertexData.tangents.push_back(tangentVector);
					}
				}

				// read vertex color
				const VertexDataStruct *pVertexColorStruct = getVertexDataStructFor(meshData, VertexUsageType::Color);
				if(pVertexColorStruct)
				{
					setVertexData(meshData, pVertexColorStruct, vertexDataOffset, vertexDataSize);
					meshData.vertexData.colors.reserve(meshData.meshInfo.vertexCount + 1);
					for(int32_t i = 0; i != meshData.meshInfo.vertexCount; ++i)
					{
						int32_t colorOffset = lod.vertexDataOffset + vertexDataOffset + pVertexColorStruct->dataOffset + vertexDataSize * i;
						colorOffset -= 68;
						currentPos = mdl.mpDecompressedData + colorOffset;

						Color4 colorVector;
						READ(uint8_t, colorVector.r);
						READ(uint8_t, colorVector.g);
						READ(uint8_t, colorVector.b);
						READ(uint8_t, colorVector.a);

						meshData.vertexData.colors.push_back(colorVector);
					}
				}

				// read texture coordinates
				const VertexDataStruct *pTcDataStruct = getVertexDataStructFor(meshData, VertexUsageType::TextureCoordinate);
				if(pVertexColorStruct)
				{
					setVertexData(meshData, pTcDataStruct, vertexDataOffset, vertexDataSize);
					meshData.vertexData.textureCoordinates0.reserve(meshData.meshInfo.vertexCount + 1);
					meshData.vertexData.textureCoordinates1.reserve(meshData.meshInfo.vertexCount + 1);
					for(int32_t i = 0; i != meshData.meshInfo.vertexCount; ++i)
					{
						int32_t tcOffset = lod.vertexDataOffset + vertexDataOffset + pTcDataStruct->dataOffset + vertexDataSize * i;
						tcOffset -= 68;
						currentPos = mdl.mpDecompressedData + tcOffset;

						Vec2 tcVector1;
						Vec2 tcVector2;
						// Normal data is either stored in half-floats or singles
						if(pTcDataStruct->dataType == VertexDataType::Half4)
						{
							uint16_t temp;
							READ(uint16_t, temp);
							half_float::half coord(half_float::detail::binary_t(), temp);
							tcVector1.x = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							tcVector1.y = static_cast<float>(coord);
							
							READ(uint16_t, temp);
							coord = temp;
							tcVector2.x = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							tcVector2.y = static_cast<float>(coord);

							meshData.vertexData.textureCoordinates0.push_back(tcVector1);
							meshData.vertexData.textureCoordinates1.push_back(tcVector2);
						}
						else if (pTcDataStruct->dataType == VertexDataType::Half2)
						{
							uint16_t temp;
							READ(uint16_t, temp);
							half_float::half coord(half_float::detail::binary_t(), temp);
							tcVector1.x = static_cast<float>(coord);

							READ(uint16_t, temp);
							coord = temp;
							tcVector1.y = static_cast<float>(coord);
							
							meshData.vertexData.textureCoordinates0.push_back(tcVector1);
						}
						else if(pTcDataStruct->dataType == VertexDataType::Float2)
						{
							READ(float, tcVector1.x);
							READ(float, tcVector1.y);
							meshData.vertexData.textureCoordinates0.push_back(tcVector1);
						}
						else if(pTcDataStruct->dataType == VertexDataType::Float4)
						{
							READ(float, tcVector1.x);
							READ(float, tcVector1.y);
							READ(float, tcVector2.x);
							READ(float, tcVector2.y);

							meshData.vertexData.textureCoordinates0.push_back(tcVector1);
							meshData.vertexData.textureCoordinates1.push_back(tcVector2);
						}
					}
				}

				// read indices
				int32_t indexOffset = lod.indexDataOffset + meshData.meshInfo.indexDataOffset * 2;
				indexOffset -= 68;
				currentPos = mdl.mpDecompressedData + indexOffset;

				meshData.vertexData.indices.reserve(meshData.meshInfo.indexCount + 1);
				for(int32_t i = 0; i != meshData.meshInfo.indexCount; ++i)
				{
					uint16_t index;
					READ(uint16_t, index);
					meshData.vertexData.indices.push_back(index);
				}

				// update totalMeshNum
				++totalMeshNum;
			}
		}



		return true;
	}

	Model::~Model()
	{
		delete[] mPaddingBytes;
	}

	const VertexDataStruct *Model::getVertexDataStructFor(const MeshData &md, VertexUsageType usageType)
	{
		// get the vertexDataStruct for normals
		for (const auto &vds : md.vertexDataStructList)
		{
			if (vds.dataUsage == usageType)
				return &vds;
		}
		return nullptr;
	}

	void Model::setVertexData(const MeshData &meshData, const VertexDataStruct *pDataStruct, int32_t &vertexDataOffset, int32_t &vertexDataSize)
	{
		switch (pDataStruct->dataBlock)
		{
		case 0:
			vertexDataOffset = meshData.meshInfo.vertexDataOffset0;
			vertexDataSize = meshData.meshInfo.vertexDataEntrySize0;
			break;
		case 1:
			vertexDataOffset = meshData.meshInfo.vertexDataOffset1;
			vertexDataSize = meshData.meshInfo.vertexDataEntrySize1;
			break;
		default:
			vertexDataOffset = meshData.meshInfo.vertexDataOffset2;
			vertexDataSize = meshData.meshInfo.vertexDataEntrySize2;
			break;
		}
	}
}
