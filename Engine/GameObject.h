#pragma once
#include "pch.h"
#include "Matrix.h"
#include "Vector3.h"

class Component;
class Mesh;
class Triangle;
class CharacterControllerComponent;
class AudioSourceComponent;
class NavmeshActorComponent;
class ObjectManager;

class GameObject
{
public:
	GameObject();
	~GameObject();
	const Math::Matrix& GetWorldMatrix() { return m_worldMatrix; }
	const Math::Matrix& GetLocalMatrix() { return m_localMatrix; }
	void SetWorldMatrix(const Math::Matrix& worldMatrix);
	void SetLocalMatrix(const Math::Matrix& localMatrix);
	void SetPosition(const Math::Vector3& position);
	const Math::Vector3& GetPosition();
	void SetRotation(const Math::Vector3& rotationInDegrees);
	void Rotate(const Math::Vector3& rotationInDegrees);
	Math::Vector3 GetRotation();

	void SetWorldMatrixNoUpdate(const Math::Matrix& worldMatrix);
	void SetLocalMatrixNoUpdate(const Math::Matrix& localMatrix);

	void SetParent(GameObject* newParent);
	GameObject* GetParent() { return m_parent; }
	const std::vector<GameObject*> GetChildren() { return m_children; }

	void DestroyOnReset();
	bool ShouldDestroyOnReset() const;

	void RemoveComponent(std::shared_ptr<Component> component);
	void AddComponent(std::shared_ptr<Component> component);
	std::vector<std::shared_ptr<Component>>& GetComponents() { return m_components; };
	template<typename T>
	inline std::shared_ptr<T> GetComponentOfType() const
	{
		for (const auto& component : m_components)
		{
			auto castedComponent = std::dynamic_pointer_cast<T>(component);
			if (castedComponent) {
				return castedComponent;
			}
		}
		return nullptr;
	}


	// hax for lua script getter
	CharacterControllerComponent* GetCharacterControllerComponent() const;
	AudioSourceComponent*		  GetAudioSourceComponent() const;
	NavmeshActorComponent*		  GetNavmeshActorComponent() const;

	void SetName(const std::string& name);
	const std::string& GetName() { return m_name; }

	//void Translate(const Math::Vector3& position, const Math::Vector4& quat);
	GameObject* GetRootGameObject();

	unsigned long long GetId() const;

private:
	friend class ObjectManager;
	void UpdateLocalMatrix();

	void UpdateTransform();

	void Start();
	void Reset();

	void AddChild(GameObject* gameObject);
	void RemoveChild(GameObject* gameObject);

	void UpdateChildrenGlobalMatrix();


	std::vector<GameObject*> m_children;
	GameObject* m_parent;
	std::vector<std::shared_ptr<Component>> m_components;
	std::string m_name;

	Math::Matrix m_worldMatrix;
	Math::Matrix m_localMatrix;
#ifdef EDITOR
	Math::Matrix m_initialWorldMatrix;
	Math::Matrix m_initialLocalMatrix;
#endif // EDITOR

	bool m_shouldDestroyOnReset;
	unsigned long long m_id;
};


