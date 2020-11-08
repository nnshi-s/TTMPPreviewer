#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Mpl.h"

namespace Tt
{
	static inline bool has(const std::string& src, const std::string& tofind)
	{
		return src.find(tofind) != std::string::npos;
	}

	
	static std::vector<MplModdedItem> genModdedItemVec(const std::vector<MplItem>& allItems)
	{
		std::vector<MplModdedItem> moddedItems;

		// get all mdls
		for (const auto& item : allItems)
		{
			if (has(item.mPath, ".mdl"))
			{
				MplModdedItem m(item.mName);
				m.mRelatedResourceItems[0] = item;
				moddedItems.push_back(std::move(m));
			}
		}

		// get all textures for each mdl
		for (const auto& item : allItems)
		{
			if (has(item.mPath, ".tex")) // is a texture
			{
				for (auto& mi : moddedItems)
				{
					if (has(item.mPath, mi.mRelatedResourceItems[0].getPathWithoutExt()))
					{
						if (has(item.mPath, "_d.tex"))
						{
							mi.mRelatedResourceItems[1] = item; // diffuse map
							mi.mAvailableTextures++;
						}
						else if (has(item.mPath, "_n.tex"))
						{
							mi.mRelatedResourceItems[2] = item; // normal map
							mi.mAvailableTextures++;
						}
						else if (has(item.mPath, "_s.tex"))
						{
							mi.mRelatedResourceItems[3] = item; // specular map
							mi.mAvailableTextures++;
						}
					}
				}
			}
		}

		return moddedItems;
	}

	bool Mpl::load(const std::wstring& mplPath)
	{
		std::ifstream ifs(mplPath.c_str(), std::ios::in);
		if (!ifs.good())
		{
			std::cerr << "could not open .mpl" << std::endl;
			return false;
		}
		std::stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();

		clear();

		mType = MplType::UNKNOWN;


		// detect the type of mpl
		nlohmann::json j;
		try
		{
			j = nlohmann::json::parse(ss.str());

			if (j.contains("ModPackPages") && !j["ModPackPages"].is_null())
			{
				mType = MplType::PAGED;
			}
			else if (j.contains("SimpleModsList") && !j["SimpleModsList"].is_null())
			{
				mType = MplType::SIMPLE;
			}
			else
			{
				mType = MplType::UNKNOWN;
			}
		}
		catch (nlohmann::detail::parse_error& err)
		{
			mType = MplType::ITEM;
		}


		if (mType == MplType::PAGED)
		{
			addAllGroups(j);
		}
		else if (mType == MplType::SIMPLE)
		{
			addAllModdedItemsSimple(j);
		}
		else if (mType == MplType::ITEM)
		{
			addAllModdedItems(ss.str());
		}
		else
		{
			return false;
		}


		return true;
	}


	void Mpl::addAllGroups(const nlohmann::json& j)
	{
		for (const nlohmann::json& page : j["ModPackPages"])
		{
			for (const nlohmann::json& group : page["ModGroups"])
			{
				MplGroup g(group["GroupName"]);
				g.addAllOptions(group);
				mGroups.push_back(std::move(g));
			}
		}
	}


	void Mpl::addAllModdedItemsSimple(const nlohmann::json& j)
	{
		std::vector<MplItem> allItems;
		for (const nlohmann::json& item : j["SimpleModsList"])
		{
			MplItem i(
				item["Name"],
				item["FullPath"],
				item["ModOffset"],
				item["ModSize"]
			);
			i.mPath = i.mPath.substr(i.mPath.find_last_of('/') + 1);
			allItems.push_back(std::move(i));
		}

		// grab the resources related to the same ffxiv item
		std::vector<MplModdedItem> moddedItems = genModdedItemVec(allItems);

		// add those items with more than 1 textures available
		for (const auto &mi : moddedItems)
		{
			if (mi.mAvailableTextures >= 1)
				mModdedItems.push_back(mi);
		}
	}

	void Mpl::addAllModdedItems(const std::string& s)
	{
		std::vector<MplItem> allItems;
		
		auto b = std::find(s.cbegin(), s.cend(), '{');
		auto e = std::find(s.cbegin(), s.cend(), '}');
		while (b != s.cend() && e != s.cend())
		{
			std::string strJson = s.substr(b - s.cbegin(), e - b + 1);

			const auto j = nlohmann::json::parse(strJson);
			MplItem i(
				j["Name"],
				j["FullPath"],
				j["ModOffset"],
				j["ModSize"]
			);

			i.mPath = i.mPath.substr(i.mPath.find_last_of('/') + 1);
			allItems.push_back(std::move(i));

			b = std::find(e + 1, s.cend(), '{');
			e = std::find(e + 1, s.cend(), '}');
		}

		// grab the resources related to the same ffxiv item
		std::vector<MplModdedItem> moddedItems = genModdedItemVec(allItems);

		// add those items with more than 1 textures available
		for (const auto &mi : moddedItems)
		{
			if (mi.mAvailableTextures >= 1)
				mModdedItems.push_back(mi);
		}
	}


	void MplGroup::addAllOptions(const nlohmann::json& j)
	{
		for (const nlohmann::json& option : j["OptionList"])
		{
			if (option["Name"] != "None")
			{
				MplOption o(option["Name"]);
				o.addAllModdedItems(option);
				mOptions.push_back(std::move(o));
			}
		}
	}

	void MplOption::addAllModdedItems(const nlohmann::json& j)
	{
		std::vector<MplItem> allItems;
		for (const nlohmann::json& item : j["ModsJsons"])
		{
			MplItem i(
				item["Name"],
				item["FullPath"],
				item["ModOffset"],
				item["ModSize"]
			);
			i.mPath = i.mPath.substr(i.mPath.find_last_of('/') + 1);
			allItems.push_back(std::move(i));
		}

		// grab the resources related to the same ffxiv item
		std::vector<MplModdedItem> moddedItems = genModdedItemVec(allItems);

		// add those items with more than 1 textures available
		for (const auto& mi : moddedItems)
		{
			if (mi.mAvailableTextures >= 1)
				mModdedItems.push_back(mi);
		}
	}

	std::tuple<unsigned, unsigned> MplModdedItem::getMdlOffSize() const
	{
		return { mRelatedResourceItems[0].mOffset, mRelatedResourceItems[0].mSize };
	}

	std::tuple<unsigned, unsigned> MplModdedItem::getDiffuseOffSize() const
	{
		return { mRelatedResourceItems[1].mOffset, mRelatedResourceItems[1].mSize };
	}

	std::tuple<unsigned, unsigned> MplModdedItem::getNormalOffSize() const
	{
		return { mRelatedResourceItems[2].mOffset, mRelatedResourceItems[2].mSize };
	}

	std::tuple<unsigned, unsigned> MplModdedItem::getSpecularOffSize() const
	{
		return { mRelatedResourceItems[3].mOffset, mRelatedResourceItems[3].mSize };
	}
}
