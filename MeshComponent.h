#pragma once
#include "pch.h"
#include "Component.h"
#include <string>

class Mesh;
class Material;
struct aiMesh;

class MeshComponent : public Component, public std::enable_shared_from_this<MeshComponent>
{
public:
	MeshComponent(GameObject* owner);
	MeshComponent(GameObject* owner, std::string filename);
	MeshComponent(GameObject* owner, MeshComponent* meshComponent);
	MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh);

	void Render() override;
	void Update(float deltaTime) override;
	void SaveComponent(WriteObject& readObject) override;
	void LoadComponent(ReadObject& readObject) override;

#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	std::shared_ptr<Mesh> GetMesh() { return m_mesh; }
	void SetMesh(const std::string& filename);
	void SetMesh(std::shared_ptr<Mesh> mesh);

private:
	std::shared_ptr<Mesh> m_mesh;

};

