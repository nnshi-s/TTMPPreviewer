#pragma once

#include <string>

namespace Tt
{
	class Ttmp
	{
	public:
		Ttmp(const std::wstring& ttmpPath) : mTtmpPath(ttmpPath) {}
		bool unzip();
	private:
		std::wstring mTtmpPath;
		std::wstring mUnzipPath = L".\\tmp\\";
	};
}
