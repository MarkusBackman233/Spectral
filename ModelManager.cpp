#include "ModelManager.h"

ModelManager::ModelManager()
{
	std::cout << "ModelManager initialized" << std::endl;
}

Mesh ModelManager::GetMesh(std::string filename)
{
	for(auto mesh : m_meshes)
	{
		if (mesh.filename == filename){
			return mesh;
		}
	}

	Mesh loadedMesh = LoadObjFromFile(filename, true);

	m_meshes.push_back(loadedMesh);
	return loadedMesh;
}

Mesh ModelManager::LoadObjFromFile(std::string fileName, bool hasTexture = false)
{
	Uint32 startLoadTime = SDL_GetTicks();

	Mesh loadMesh;

	loadMesh.filename = fileName;

	std::ifstream file(fileName);
	if (!file.is_open())
		std::cout << "Failed to load mesh: " << fileName << std::endl;


	std::vector<Math::Vector2> UVS;

	while (!file.eof())
	{
		char line[128];
		file.getline(line, 128);

		std::strstream stream;
		stream << line;



		char junk;
		if (line[0] == 'v' && line[1] == ' ')
		{
			Math::Vector3 vertex;

			stream >> junk >> vertex.x >> vertex.y >> vertex.z;
			loadMesh.vertexes.push_back(vertex);
		}
		if (line[0] == 'v' && line[1] == 't')
		{
			Math::Vector2 UV;

			stream >> junk >> junk >> UV.x >> UV.y;
			UVS.push_back(UV);
		}
		if (!hasTexture)
		{
			if (line[0] == 'f')
			{
				int indexes[3];

				stream >> junk >> indexes[0] >> indexes[1] >> indexes[2];

				loadMesh.indices32.push_back(indexes[0] - 1);
				loadMesh.indices32.push_back(indexes[1] - 1);
				loadMesh.indices32.push_back(indexes[2] - 1);

				loadMesh.triangles.push_back(Triangle(loadMesh.vertexes[indexes[0] - 1], loadMesh.vertexes[indexes[1] - 1], loadMesh.vertexes[indexes[2] - 1]));
				Math::Vector2 UV[3];
				UV[0] = Math::Vector2(0.0f, 1.0f, 1.0f);
				UV[1] = Math::Vector2(0.0f, 0.0f, 1.0f);
				UV[2] = Math::Vector2(1.0f, 0.0f, 1.0f);
				loadMesh.triangles.back().m_UV[0] = UV[0];
				loadMesh.triangles.back().m_UV[1] = UV[1];
				loadMesh.triangles.back().m_UV[2] = UV[2];
			}
		}
		else {
			if (line[0] == 'f')
			{
				stream >> junk;

				std::string tokens[6];
				int nTokenCount = -1;


				while (!stream.eof())
				{
					char c = stream.get();
					if (c == ' ' || c == '/')
						nTokenCount++;
					else
						tokens[nTokenCount].append(1, c);
				}

				tokens[nTokenCount].pop_back();

				Triangle newTriangle(loadMesh.vertexes[stoi(tokens[0]) - 1], loadMesh.vertexes[stoi(tokens[2]) - 1], loadMesh.vertexes[stoi(tokens[4]) - 1]);


				loadMesh.indices32.push_back((PxU32)stoi(tokens[4]) - 1);
				loadMesh.indices32.push_back((PxU32)stoi(tokens[2]) - 1);
				loadMesh.indices32.push_back((PxU32)stoi(tokens[0]) - 1);



				newTriangle.m_UV[0] = UVS[stoi(tokens[1]) - 1];
				newTriangle.m_UV[1] = UVS[stoi(tokens[3]) - 1];
				newTriangle.m_UV[2] = UVS[stoi(tokens[5]) - 1];


				float tempX;
				tempX = newTriangle.m_UV[0].x;
				newTriangle.m_UV[0].x = 1 - newTriangle.m_UV[0].y;
				newTriangle.m_UV[0].y = tempX;

				tempX = newTriangle.m_UV[1].x;
				newTriangle.m_UV[1].x = 1 - newTriangle.m_UV[1].y;
				newTriangle.m_UV[1].y = tempX;

				tempX = newTriangle.m_UV[2].x;
				newTriangle.m_UV[2].x = 1 - newTriangle.m_UV[2].y;
				newTriangle.m_UV[2].y = tempX;

				newTriangle.m_vertex[0].x = -newTriangle.m_vertex[0].x;
				newTriangle.m_vertex[1].x = -newTriangle.m_vertex[1].x;
				newTriangle.m_vertex[2].x = -newTriangle.m_vertex[2].x;
				newTriangle.m_normalDirection = newTriangle.CalculateNormal();
				loadMesh.triangles.push_back(newTriangle);

			}
		}
	}
	std::cout << "Loaded Mesh: " << fileName << " " << (SDL_GetTicks() - startLoadTime) <<"ms" << std::endl;
	return loadMesh;
}