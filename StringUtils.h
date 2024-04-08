#pragma once
#include "pch.h"

class StringUtils
{
public:

	static std::string StripPathFromFilename(const std::string& filename);
	static std::string GetOnlyPathFromFilename(const std::string& filename);

	static bool StringContainsCaseInsensitive(std::string string, std::string subString);

};

