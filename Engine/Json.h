#pragma once
#include <string>
#include <variant>
#include <map>
#include <vector>
#include <filesystem>


class Json
{
public:
	using Object = std::map<std::string, Json>;
	using Array  = std::vector<Json>;

	static Json ParseString(const std::string& string);
	static Json ParseFile(const std::filesystem::path& path);

private:
	static void SkipWhitespace(const std::string& file, size_t& index);
	static Json ParseValue(const std::string& file, size_t& index, bool asArray = false);
	static std::string GetNextKey(const std::string& file, size_t& index);

public:
	static void Serialize(const Json& json, const std::filesystem::path& file);
private:
	std::string JsonToString(int indent) const;
public:
	Json(const Object& value){ m_value = value; }
	Json(const std::string& value){ m_value = value; }
	Json(const char* value){ m_value = value; }
	Json(bool value){ m_value = value; }
	Json(float value){ m_value = value; };
	Json(int value){ m_value = value; };
	Json(const Array& value){ m_value = value; }

	Json operator[](const std::string& key);

	const std::string& AsString() const;
	const Object& AsObject() const;
	const Array& AsArray() const;
	const float AsFloat() const;
	const int AsInt() const;
	const bool AsBool() const;


	std::string& AsStringRef();
	Object& AsObjectRef();
	Array& AsArrayRef();
	float& AsFloatRef();
	int& AsIntRef();
	bool& AsBoolRef();

	const bool Has(const std::string& key) const;

	bool HasError() const;

private:
	bool m_hasError = false;

	std::variant<Object, Array, std::string, float, int, bool> m_value;
};

