#pragma once
#include "pch.h"
#include <fstream>
#include <algorithm>

struct aiScene;
struct aiMesh;
struct aiNode;

class Mesh;
class GameObject;
class Material;

namespace Math
{
	class Matrix;
};

class WriteObject
{
public:
	WriteObject(std::string filename)
	{
		try
		{
			m_file = std::ofstream(filename, std::ios::out | std::ios::binary);
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
	~WriteObject() { m_file.close(); }


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
};


class ReadObject
{
public:
	ReadObject(std::string filename)
	{
		m_file = std::ifstream(filename, std::ios::in | std::ios::binary);
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
	static std::string ProjectName;
	static std::string ProjectDirectory;
	static std::string ExecutableDirectory;
	static std::wstring ProjectDirectoryWide;
	static std::wstring ExecutableDirectoryWide;

	static std::string SpectralModelExtention;
	static std::string SpectralSceneExtention;
	static std::string SpectralMaterialExtention;

	static bool LoadFBX(const std::string& filename);
	static bool LoadTexture(const std::string& filename);

	static void SaveSpectralModel(std::shared_ptr<Mesh> mesh);
	static bool LoadSpectralModel(const std::string& filename, std::shared_ptr<Mesh>& mesh);

	static void SaveSpectralScene(const std::string& sceneName);
	static bool LoadSpectralScene(const std::string& filename);

	static void SaveSpectralMaterial(std::shared_ptr<Material> material);
	static bool LoadSpectralMaterial(const std::string& filename, std::shared_ptr<Material>& material);

	static void CollectProjectFiles();

private: 
	static void SaveGameObject(WriteObject& writeObject, GameObject* gameObject);
	static void LoadGameObject(ReadObject& readObject, GameObject* parent);

	static void ProcessMesh(const std::string& filename, aiMesh* mesh, const aiScene* scene, GameObject* gameObject);
	static void ProcessNode(const std::string& filename, aiNode* node, const aiScene* scene, GameObject* parent, const Math::Matrix& accTransform);
};

