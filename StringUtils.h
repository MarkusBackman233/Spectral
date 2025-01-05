#pragma once
#include "pch.h"

class StringUtils
{
public:

	static std::string StripPathFromFilename(const std::string& filename);
	static std::string GetOnlyPathFromFilename(const std::string& filename);
	static std::string StringToLower(const std::string& string);
	static std::wstring StringToWideString(const std::string& string);
	static std::string WideStringToString(const std::wstring& string);

	static bool StringContainsCaseInsensitive(std::string string, std::string subString);

};

