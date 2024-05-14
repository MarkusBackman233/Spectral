#include "StringUtils.h"
#include <filesystem>
#include <Windows.h>

std::string StringUtils::StripPathFromFilename(const std::string& filename)
{
    return std::filesystem::path(filename).filename().generic_string();
}

std::string StringUtils::GetOnlyPathFromFilename(const std::string& filename)
{
    return std::filesystem::path(filename).parent_path().generic_string();
}

bool StringUtils::StringContainsCaseInsensitive(std::string string, std::string subString)
{
    std::transform(string.begin(), string.end(), string.begin(), ::tolower);
    std::transform(subString.begin(), subString.end(), subString.begin(), ::tolower);

    return string.find(subString) != std::string::npos;
}