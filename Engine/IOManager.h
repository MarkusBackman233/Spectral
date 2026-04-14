#pragma once
#include "pch.h"
#include <fstream>
#include <rapidjson/document.h>
#include <filesystem>
#include "Json.h"
#include "Model.h"

struct aiScene;
struct aiMesh;
struct aiNode;

class Mesh;
class GameObject;
class DefaultMaterial;
class Model;
struct SubMesh;

namespace Math
{
	class Matrix;
};

class WriteObject
{
public:
	WriteObject(const std::filesystem::path& filePath)
		: m_path(filePath)
	{
		try
		{
			m_file = std::ofstream(filePath, std::ios::out | std::ios::binary);
		}
		catch (std::exception& e)
		{
			Logger::Error(e.what());
		}
	}
	~WriteObject() 
	{ 
		try
		{
			std::filesystem::create_directories(m_path.parent_path());
		}
		catch (const std::exception& e)
		{
			Logger::Error(e.what());
			return;
		}
		m_file.close(); 
	}


	template<typename T>
	inline void Write(const T& object, const size_t numberOfElements = 1)
	{
		m_file.write(reinterpret_cast<const char*>(&object), sizeof(T) * numberOfElements);
	}

	template<typename T>
	inline void Write(const std::vector<T> vector)
	{
		size_t elementsInVector = vector.size();
		m_file.write(reinterpret_cast<const char*>(&elementsInVector), sizeof(size_t));
		m_file.write(reinterpret_cast<const char*>(vector.data()), sizeof(T) * elementsInVector);
	}

	void Write(const std::string& string)
	{
		size_t charsInName = string.size();
		m_file.write(reinterpret_cast<const char*>(&charsInName), sizeof(size_t));
		m_file.write(string.c_str(), charsInName);
	}


	std::ofstream& GetFile() { return m_file; }

private:
	std::ofstream m_file;
	std::filesystem::path m_path;
};


class ReadObject
{
public:
	ReadObject(const std::filesystem::path& filePath)
	{
		m_file = std::ifstream(filePath, std::ios::in | std::ios::binary);
	}
	~ReadObject() { m_file.close(); }

	template<typename T>
	std::enable_if_t<!std::is_same_v<T, std::string>, T> Read(const size_t numberOfElements = 1)
	{
		T data{};
		m_file.read(reinterpret_cast<char*>(&data), sizeof(T) * numberOfElements);
		return data;
	}

	template<typename T>
	inline void Read(T& object, const size_t numberOfElements = 1)
	{
		m_file.read(reinterpret_cast<char*>(&object), sizeof(T) * numberOfElements);
	}

	template<typename T>
	inline void Read(std::vector<T>& vector)
	{
		size_t numberOfChars = Read<size_t>();
		vector.resize(numberOfChars);
		m_file.read(reinterpret_cast<char*>(vector.data()), sizeof(T) * numberOfChars);
	}

	inline void Read(std::string& string)
	{
		size_t numberOfChars = Read<size_t>();
		string.resize(numberOfChars);
		m_file.read(reinterpret_cast<char*>(string.data()), sizeof(char) * numberOfChars);
	}	

	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string>, std::string> Read(const size_t numberOfElements = 1) {
		size_t numberOfChars = Read<size_t>();
		std::string str;
		str.resize(numberOfChars);
		m_file.read(reinterpret_cast<char*>(str.data()), numberOfChars);
		return str;
	}

	std::ifstream& GetFile() { return m_file; }
private:
	std::ifstream m_file;
};


class IOManager
{
public:
	static std::string IniFailedToFindItem;

	static std::string ProjectName;
	static std::filesystem::path ProjectDirectory;
	static std::filesystem::path ExecutableDirectory;



	struct IOResourceData
	{
		std::string Folder;
		std::string SpectralExtension;
		std::vector<std::string> SupportedExtensions;
	};
	static std::array<IOResourceData, static_cast<uint8_t>(ResourceType::Num)> IOResources;

	template <ResourceType T>
	static IOResourceData& GetResourceData()
	{
		return IOResources[static_cast<size_t>(T)];
	}

	static void SetExecutableDirectiory();
	static bool LoadProject(std::optional<std::string> forceProject = std::nullopt);

	static bool LoadFBX(const std::filesystem::path& filename);

	static bool LoadDroppedResource(const std::filesystem::path& file);


	static void SaveSpectralScene(const std::string& sceneName);
	static bool LoadSpectralScene(const std::string& filename);

	static bool CreateDefaultScene();

	static void CollectProjectFiles();

	static Json::Object SaveGameObject(GameObject* gameObject);
	static void LoadGameObject(const rapidjson::Value& object, GameObject* parent);
private: 

	static void ProcessMesh(Model* model, const std::filesystem::path& path, aiMesh* mesh, const aiScene* scene, SubMesh& subMesh);
	static void ProcessNode(Model* model, const std::filesystem::path& path, aiNode* node, const aiScene* scene, SubMesh& subMesh, const Math::Matrix& accTransform);

};
