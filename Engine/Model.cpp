#include "Model.h"
#include "Logger.h"
#include "IOManager.h"
#include "Mesh.h"
#include "iRender.h"
#include "DefaultMaterial.h"
#include "ResourceManager.h"

bool Model::Load(const std::filesystem::path& file)
{
	m_path = file;

	Logger::Info(std::string("Loading Model: ") + file.filename().string());
	ReadObject readObject(file);
	if (!readObject.GetFile().is_open())
	{
		return false;
	}

	readObject.Read(m_filename);

	uint8_t numMaterials = 0;
	readObject.Read(numMaterials);
	for (uint8_t i = 0; i < numMaterials; i++)
	{
		std::string materialName;
		readObject.Read(materialName);
		m_materials.push_back(ResourceManager::GetInstance()->GetResource<DefaultMaterial>(materialName));
	}

	LoadSubmesh(readObject, m_root);
	CalculateBoundingBox();

	return true;
}

void Model::Save()
{
	WriteObject writeObject(m_path);
	writeObject.Write(GetFilename());
	writeObject.Write(static_cast<uint8_t>(m_materials.size()));

	for (auto& material : m_materials)
	{
		writeObject.Write(material->GetFilename());
	}

	SaveSubmesh(writeObject, m_root);
}

void Model::LoadSubmesh(ReadObject& readObject, SubMesh& subMesh)
{

	bool meshExists = false;
	readObject.Read(meshExists);
	if (meshExists)
	{
		std::string meshName;
		readObject.Read(meshName);
		subMesh.m_mesh = std::make_shared<Mesh>(meshName);
		readObject.Read(subMesh.m_mesh->vertexes);
		readObject.Read(subMesh.m_mesh->indices32);
		readObject.Read(subMesh.m_materialIndex);

	}

	readObject.Read(subMesh.m_localMatrix);



	uint32_t numChildren = 0;
	readObject.Read(numChildren);
	for (uint32_t i = 0; i < numChildren; i++)
	{
		subMesh.m_submeshes.push_back(SubMesh());
		LoadSubmesh(readObject, subMesh.m_submeshes.back());
	}
	if (meshExists)
	{
		subMesh.m_mesh->CalculateBoundingBox();
		subMesh.m_mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
	}
}

void Model::AddSubMeshBound(SubMesh& subMesh)
{
	if (subMesh.m_mesh)
	{
		auto subMeshMinBound = subMesh.m_mesh->GetBoundingBoxMin().Transform(subMesh.m_localMatrix);
		auto subMeshMaxBound = subMesh.m_mesh->GetBoundingBoxMax().Transform(subMesh.m_localMatrix);

		m_minBounds.x = std::min(m_minBounds.x, subMeshMinBound.x);
		m_minBounds.y = std::min(m_minBounds.y, subMeshMinBound.y);
		m_minBounds.z = std::min(m_minBounds.z, subMeshMinBound.z);
		m_maxBounds.x = std::max(m_maxBounds.x, subMeshMaxBound.x);
		m_maxBounds.y = std::max(m_maxBounds.y, subMeshMaxBound.y);
		m_maxBounds.z = std::max(m_maxBounds.z, subMeshMaxBound.z);
	}

	for (auto& child : subMesh.m_submeshes)
	{
		AddSubMeshBound(child);
	}
}

void Model::CalculateBoundingBox()
{
	m_minBounds = Math::Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	m_maxBounds = Math::Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

	AddSubMeshBound(m_root);
}

std::vector<std::shared_ptr<DefaultMaterial>>& Model::GetMaterials()
{
	return m_materials;
}



void Model::SaveSubmesh(WriteObject& writeObject, const SubMesh& subMesh)
{

	bool meshExists = subMesh.m_mesh != nullptr;


	writeObject.Write(meshExists);
	if (meshExists)
	{
		writeObject.Write(subMesh.m_mesh->GetFilename());
		writeObject.Write(subMesh.m_mesh->vertexes);
		writeObject.Write(subMesh.m_mesh->indices32);
		writeObject.Write(subMesh.m_materialIndex);
	}



	writeObject.Write(subMesh.m_localMatrix);
	writeObject.Write(static_cast<uint32_t>(subMesh.m_submeshes.size()));

	for (auto& child : subMesh.m_submeshes)
	{
		SaveSubmesh(writeObject, child);
	}
}
