#include "Ttmp.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace Tt
{
	bool Ttmp::unzip()
	{
		std::wstringstream cmd;
		cmd << ".\\7z.exe e \"";
		cmd << mTtmpPath;
		cmd << "\" -aoa -o\"";
		cmd << mUnzipPath;
		// extract filename
		auto beg = mTtmpPath.find_last_of('\\');
		auto end = mTtmpPath.find_last_of('.');
		std::wstring fileName = mTtmpPath.substr(beg + 1, end - beg - 1);
		// fileName.erase(std::remove_if(fileName.begin(), fileName.end(), std::isspace), fileName.end());
		cmd << fileName;
		cmd << "\"";

		auto *fp = _wpopen(cmd.str().c_str(), L"r");

		wchar_t *pBuffer = new wchar_t[512];
		
		while (fgetws(pBuffer, 512, fp))
			_putws(pBuffer);

		delete[] pBuffer;
		
		if(feof(fp))
		{
			auto retNum = _pclose(fp);
			if(retNum == 0)
			{
				std::wcout << "\nProcess Returned " << retNum << std::endl;
				return true;
			}
			std::wcerr << "\nFailed To Execute the following command:\n" << cmd.str() << std::endl;
			return false;
			
		}

		return false;

		
	}
}
