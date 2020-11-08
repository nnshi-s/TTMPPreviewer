#pragma once

#include <string>
#include <vector>
#include <json.hpp>

namespace Tt
{
	enum class MplType
	{
		UNKNOWN,
		PAGED,	// mpl -> modpack pages -> groups -> options -> items
		SIMPLE, // mpl -> simple mod list -> items
		ITEM    // mpl -> items
	};

	struct MplGroup;
	struct MplOption;
	struct MplModdedItem;
	struct MplItem;

	class Mpl
	{
		friend struct MplGroup;
		friend struct MplOption;
	public:
		bool load(const std::wstring& mplPath);
		MplType getType() const { return mType; }
		std::vector<MplGroup> mGroups; // for type PAGED 
		std::vector<MplModdedItem> mModdedItems; // for type SIMPLE and ITEM
	private:
		void addAllGroups(const nlohmann::json& j);
		void addAllModdedItemsSimple(const nlohmann::json& j);
		void addAllModdedItems(const std::string& s);
		MplType mType = MplType::UNKNOWN;

		void clear()
		{
			mGroups.clear();
			mModdedItems.clear();
		}
	};


	struct MplGroup
	{
		std::string mGroupName;
		std::vector<MplOption> mOptions;

		MplGroup(const std::string& name) : mGroupName(name)
		{
		}

		void addAllOptions(const nlohmann::json& j);
	};

	struct MplOption
	{
		std::string mOptionName;
		std::vector<MplModdedItem> mModdedItems;

		MplOption(const std::string& name) : mOptionName(name)
		{
		}

		void addAllModdedItems(const nlohmann::json& j);
	};

	struct MplItem // this structure represents a basic json object in TTMPL.mpl
	{
		std::string mName;
		std::string mPath;
		unsigned mOffset = 0;
		unsigned mSize = 0;



		
		std::string getPathWithoutExt() const
		{
			auto b = mPath.find_last_of('/');
			auto e = mPath.find_last_of('.');
			return mPath.substr(b + 1, e - b - 1);
		}

		MplItem(const std::string& name, const std::string& path, unsigned offset, unsigned size) :
			mName(name), mPath(path), mOffset(offset), mSize(size)
		{
		}

		MplItem() = default;
	};

	struct MplModdedItem // this structure represents an ffxiv item with its model and textures
	{
		std::string mModdedItemName;
		std::array<MplItem, 4> mRelatedResourceItems; // offset 0: .mdl, 1: diffuse texture, 2: normal texture, 3: specular texture
		uint8_t mAvailableTextures;

		MplModdedItem(const std::string& name) : mModdedItemName(name), mRelatedResourceItems{}, mAvailableTextures(0)
		{
		}

		void addAllModdedItems(const nlohmann::json& j);

		std::tuple<unsigned, unsigned> getMdlOffSize() const;
		std::tuple<unsigned, unsigned> getDiffuseOffSize() const;
		std::tuple<unsigned, unsigned> getNormalOffSize() const;
		std::tuple<unsigned, unsigned> getSpecularOffSize() const;
	};
}
