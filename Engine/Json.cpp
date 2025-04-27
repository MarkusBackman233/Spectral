#include "Json.h"
#include <fstream>
#include <iostream>
#include <sstream>

Json Json::ParseFile(const std::string& file)
{
    std::ifstream f(file);
    if (!f)
    {
        std::cerr << "Error opening file!" << std::endl;
    }
    std::string data = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    size_t index = 0;
    Json json = ParseValue(data, index);

    return std::move(json);
}

void Json::Serialize(const Json& json, const std::string& file)
{

    std::ofstream outFile(file);
    if (outFile.is_open()) 
    {
        outFile << json.JsonToString(0);
        outFile.close();
    }
    else 
    {
        std::cerr << "Failed to open file for writing!" << std::endl;
    }
}

std::string Json::JsonToString(int indent) const
{
    const std::string indentA(indent * 4, ' ');
    const std::string innerIndentA((indent + 1) * 4, ' ');


    if (std::holds_alternative<std::string>(m_value))
    {
        return std::string("\"" + AsString() + "\"");
    }
    if (std::holds_alternative<float>(m_value)) 
    {
        return std::to_string(AsFloat());
    }        
    if (std::holds_alternative<int>(m_value)) 
    {
        return std::to_string(AsInt());
    }    
    if (std::holds_alternative<bool>(m_value)) 
    {
        return AsBool() ? "true" : "false";
    }       
    if (std::holds_alternative<Array>(m_value)) 
    {
        std::ostringstream stream{};
        stream << "[\n";

        bool isFirst = true;

        for (auto& json : AsArray())
        {
            if (isFirst)
            {
                isFirst = false;
            }
            else
            {
                stream << ",\n";
            }
            stream << innerIndentA  << json.JsonToString(indent + 1);
        }
        stream << "\n";

        stream << indentA << "]";
        return stream.str();
    }    
    if (std::holds_alternative<Object>(m_value)) 
    {
        std::ostringstream stream{};
        stream <<"{\n";

        bool isFirst = true;

        for (auto& [key, json] : AsObject())
        {
            if (isFirst)
            {
                isFirst = false;
            }
            else
            {
                stream << ",\n";
            }
            stream << innerIndentA << "\"" << key << "\": " << json.JsonToString(indent + 1);
        }
        stream << "\n";

        stream << indentA <<"}";
        return stream.str();
    }
}

Json Json::ParseString(const std::string& string)
{
    size_t index = 0;
    return std::move(ParseValue(string, index));
}

void Json::SkipWhitespace(const std::string& file, size_t& index)
{
    while (index < file.size() && std::isspace(file[index])) 
    {
        ++index;
    }
}

Json Json::ParseValue(const std::string& file, size_t& index, bool asArray /* = false*/)
{
    Object object;
    Array array;

    auto AddItem = [&](const std::string& key, const Json& json)
    {
        if (asArray)
        {
            array.push_back(json);
        }
        else
        {
            object.try_emplace(key, json);
        }
    };


    while (true)
    {
        std::string key;
        if (asArray)
        {
            SkipWhitespace(file, index);
            if (file[index] == ']')
            {
                break;
            }

            index++;
            SkipWhitespace(file, index);
            if (file[index] == ']')
            {
                break;
            }
        }
        else
        {
            key = GetNextKey(file, index);
            if (key == "null")
            {
                break;
            }
            SkipWhitespace(file, index);
            index++; // skip the ":"
            SkipWhitespace(file, index);
        }

        switch (file[index])
        {
        case '{':
            AddItem(key, ParseValue(file, index));
            break;
        case '\"':
            AddItem(key, GetNextKey(file, index));
            break;        
        case '[':
            AddItem(key, ParseValue(file, index, true));
            break;
        default:

            if (file.substr(index, 4) == "null") 
            {
                index += 4;
            }
            else if (file.substr(index, 4) == "true")
            {
                AddItem(key, Json(true));
                index += 4;
            }            
            else if (file.substr(index, 5) == "false")
            {
                AddItem(key, Json(false));
                index += 5;
            }
            else
            { // if it is not any of the before its a number
                for (size_t i = index; i < file.size(); i++)
                {
                    char ch = file[i];

                    if (isdigit(ch) == false && ch != '.' && ch != '-')
                    {
                        std::string contents(file.data() + index, i - index );
                        if (contents.find('.') == std::string::npos) // if its an int not a float
                        {
                            index = i;
                            AddItem(key, Json(std::stoi(contents)));
                            break;
                        }
                        index = i;
                        AddItem(key, Json(std::stof(contents)));
                        break;
                    }
                }
            }
            break;
        }
    }

    if (asArray)
    {
        return Json(array);
    }

    return Json(object);
}

std::string Json::GetNextKey(const std::string& file, size_t& index)
{
    size_t startIndex = std::string::npos;
    for (size_t i = index; i < file.size(); i++)
    {
        if (file[i] == '}')
        {
            index = i + 1;
            return "null";
        }
        if (file[i] != '\"')
        {
            continue;
        }
        if (startIndex == std::string::npos)
        {
            startIndex = i;
        }
        else
        {
            std::string contents(file.data() + startIndex + 1, i - startIndex - 1);
            index = i + 1;
           
            return contents;
        }
    }
    return "null";
}

Json Json::operator[](const std::string& key)
{
    return AsObject().find(key)->second;
}

const std::string& Json::AsString() const
{
    return std::get<std::string>(m_value);
}

const Json::Object& Json::AsObject() const
{
    return std::get<Json::Object>(m_value);
}

const Json::Array& Json::AsArray() const
{
    return std::get<Json::Array>(m_value);
}

const float Json::AsFloat() const
{
    return std::get<float>(m_value);
}

const int Json::AsInt() const
{
    return std::get<int>(m_value);
}

const bool Json::AsBool() const
{
    return std::get<bool>(m_value);
}

