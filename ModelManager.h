#pragma once
#include <string>
#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "Triangle.h"
#include <fstream>
#include <strstream>
#include "Mesh.h"

class ModelManager
{
public:
	ModelManager();

	static ModelManager& GetInstance() {
		static ModelManager instance;
		return instance;
	}

	Mesh GetMesh(std::string filename);

private:
	Mesh LoadObjFromFile(std::string fileName, bool hasTexture);

	std::vector<Mesh> m_meshes;
};

