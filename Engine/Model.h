#pragma once
#include <vector>
#include <memory>
#include "LoadableResource.h"
#include "Matrix.h"
class Mesh;
class WriteObject;
class ReadObject;
class DefaultMaterial;

struct SubMesh
{
	std::shared_ptr<Mesh> m_mesh;
	uint8_t m_materialIndex;
	std::vector<SubMesh> m_submeshes;
	Math::Matrix m_localMatrix;
};



class Model : public Resource
{
public:
	bool Load(const std::filesystem::path& file) override;
	void Save() override;
	ResourceType GetResourceType() override { return ResourceType::Model; }
	static ResourceType StaticType() { return ResourceType::Model; }
	SubMesh m_root;


	Math::Vector3 GetBoundingBoxMin() const { return m_minBounds; }
	Math::Vector3 GetBoundingBoxMax() const { return m_maxBounds; }
	void CalculateBoundingBox();

	std::vector<std::shared_ptr<DefaultMaterial>>& GetMaterials();

private:

	void AddSubMeshBound(SubMesh& subMesh);


	void LoadSubmesh(ReadObject& readObject,  SubMesh& subMesh);
	void SaveSubmesh(WriteObject& writeObject, const SubMesh& subMesh);

	std::vector<std::shared_ptr<DefaultMaterial>> m_materials;


	Math::Vector3 m_minBounds;
	Math::Vector3 m_maxBounds;

};

