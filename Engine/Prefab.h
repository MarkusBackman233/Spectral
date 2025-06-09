#pragma once
#include "LoadableResource.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "Matrix.h"
class GameObject;



class Prefab : public Resource
{
public:
	Prefab() {};

	virtual bool Load(const std::filesystem::path& file) override;

	void Reload(const std::filesystem::path& file);

	size_t GetPrefabId() const;
	void SetPrefabId(size_t id);

	GameObject* GetPrefabRoot();

private:
	void LoadGameObject(const rapidjson::Value& object, GameObject* parent);
	Math::Matrix JsonToMatrix(const rapidjson::Value& array);

	void DestroyGameObject(GameObject* gameObject);


	size_t m_prefabId;
	GameObject* m_prefabRoot;
};

