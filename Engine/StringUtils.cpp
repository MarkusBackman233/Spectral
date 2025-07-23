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

std::string StringUtils::StringToLower(const std::string& string)
{
    std::string lowerString;
    lowerString.resize(string.size());

    for (size_t i = 0; i < string.size(); i++)
    {
        lowerString[i] = std::tolower(string[i]);
    }

    return lowerString;
}

std::wstring StringUtils::StringToWideString(const std::string& string)
{
    int length = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, nullptr, 0);
    if (length == 0) {
        throw std::runtime_error("Conversion from UTF-8 to UTF-16 failed");
    }

    // Create a wstring and perform the conversion
    std::wstring wideStr(length - 1, L'\0');  // -1 to exclude the null terminator
    MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, &wideStr[0], length);

    return wideStr;
}

std::string StringUtils::WideStringToString(const std::wstring& string)
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, string.data(), -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0)
    {
        Logger::Error("Failed to convert wide string");
        return "Error";
    }
    std::string result(bufferSize, 0);
    if (WideCharToMultiByte(CP_UTF8, 0, string.data(), -1, &result[0], bufferSize, nullptr, nullptr) == 0)
    {
        Logger::Error("Failed to convert wide string");
        return "Error";
    }
    return result;
}

bool StringUtils::StringContainsCaseInsensitive(std::string string, std::string subString)
{
    std::transform(string.begin(), string.end(), string.begin(), ::tolower);
    std::transform(subString.begin(), subString.end(), subString.begin(), ::tolower);
    return string.find(subString) != std::string::npos;
}

bool StringUtils::StringEqualsCaseInsensitive(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(a[i]) != std::tolower(b[i]))
            return false;
    }
    return true;
}
