#pragma once
#include "pch.h"
#include "Matrix.h"
#include "Vector3.h"
#include <vector>
#include <string>
#include <memory>

class Component;
class Mesh;
class Triangle;

class GameObject
{
public:
	GameObject(const Math::Vector3& position = Math::Vector3(0,0,0), const Math::Vector3& rotation = Math::Vector3(0, 0, 0));
	~GameObject();

	std::string& GetName() { return m_name; }
	Math::Matrix& GetMatrix() { return m_matrix; }
	Math::Matrix& GetLocalMatrix() { return m_localMatrix; }

	void UpdateTransform();
	

	void SetParent(GameObject* newParent);
	GameObject* GetParent() { return m_parent; }
	const std::vector<GameObject*>& GetChildren() { return m_children; }

	void AddComponent(std::shared_ptr<Component> component);
	std::vector<std::shared_ptr<Component>> GetComponents() { return m_components; };
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

	void AddChild(GameObject* gameObject);
	void RemoveChild(GameObject* gameObject);

	void SetName(const std::string& name);

	void Translate(const Math::Vector3& position, const Math::Vector3& quat);

	void UpdateLocalMatrix();


	GameObject* GetRootGameObject();

private:
	std::vector<GameObject*> m_children;
	GameObject* m_parent;
	std::vector<std::shared_ptr<Component>> m_components;
	std::string m_name;

	Math::Matrix m_matrix;
	Math::Matrix m_localMatrix;
};


