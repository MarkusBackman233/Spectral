#pragma once
#include "Matrix.h"
#include "Vector3.h"
#include "Mesh.h"
#include <SDL_image.h>
#include <iostream>
#include "ModelManager.h"
#include "TextureManager.h"

class GameObject
{
public:
	GameObject(Math::Vector3 position, Math::Vector3 rotation);
	GameObject(Math::Vector3 position);
	GameObject();
	~GameObject();

	enum physicsType
	{
		staticActor,
		dynamicActor
	};		

	enum physicsShape
	{
		triangleMesh,
		box,
		sphere,
	};	


	Math::Vector3 GetPosition() { return m_position; }
	Math::Vector3 GetRotation() { return m_rotation; }
	std::string GetName() { return m_name; }

	Math::Vector2i GetTextureSize() { return Math::Vector2i(m_texture.width, m_texture.height); }
	Math::Vector2i GetNormalMapSize() { return Math::Vector2i(m_normalMap.width, m_normalMap.height); }
	Math::Matrix GetMatrix() { return m_matrix; }

	TextureManager::Texture m_texture;
	TextureManager::Texture m_normalMap;


	void CalculateBoundingBox();

	Mesh* GetMesh() { return &m_mesh; }
	physx::PxRigidDynamic* GetPhysXMesh() { return m_physicsDynamicObject; }

	void SetTexture(std::string fileName);
	void SetNormal(std::string fileName);

	void SetMesh(std::string fileName);
	void SetName(std::string name);

	void Translate(Math::Vector3 position);
	void Rotate(Math::Vector3 rotation);

	void UpdatePhysicsPosition();

	void CreateActor(physicsType type, bool isKinematic);

	void CreateShape(physicsShape shape);

	physx::PxRigidDynamic* m_physicsDynamicObject;

	bool m_isKinematic;

	std::vector<Triangle> m_triangles;

	Math::Vector3 m_maxBounds;
	Math::Vector3 m_minBounds;
private:

	void CreatePhysXMesh();

	std::string m_name;

	Mesh m_mesh;

	physx::PxTriangleMesh* m_meshCollider;

	physx::PxRigidStatic* m_physicsRigidObject;

	Math::Vector3 m_position;
	Math::Vector3 m_rotation;

	Math::Matrix m_matrix;


	
};