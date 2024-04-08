
#include "GameObject.h"
#include "Component.h"
#include <iostream>
#include "Mesh.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "MeshComponent.h"

GameObject::GameObject(const Math::Vector3& position, const Math::Vector3& rotation)
	: m_components{}
{

	Translate(position, rotation.EulerToQuat());
}

GameObject::~GameObject()
{
}

void GameObject::SetParent(GameObject* newParent)
{
	if (m_parent != nullptr)
	{
		m_parent->RemoveChild(this);
	}
	if (newParent != nullptr)
	{
		m_parent = newParent;
		m_parent->AddChild(this);
	}

	UpdateLocalMatrix();
}

void GameObject::AddComponent(std::shared_ptr<Component> component)
{
	m_components.push_back(component);
}

void GameObject::AddChild(GameObject* gameObject)
{
	m_children.push_back(gameObject);
	gameObject->m_parent = this;
}

void GameObject::RemoveChild(GameObject* gameObject)
{
	auto it = std::find_if(m_children.begin(), m_children.end(),
		[&](GameObject* p) {return p == gameObject; });
	if (it != m_children.end())
	{
		m_children.erase(it);
	}
	else 
	{
		std::cerr << "Error: Could not remove child: " << gameObject->GetName() << std::endl;
	}
}


void GameObject::SetName(const std::string& name)
{
	m_name = name;
}

void GameObject::Translate(const Math::Vector3& position, const Math::Vector3& quat)
{
	m_matrix = m_matrix.MakeIdentity();

	if (quat.x != 0 || quat.y != 0 || quat.z != 0)
	{
		float angle = 2 * acos(quat.w);
		Math::Vector3 axis = quat / sqrt(1 - quat.w * quat.w);

		Math::Matrix matrixRotation;
		matrixRotation = matrixRotation.MakeRotationAA(axis, angle);
		m_matrix = m_matrix * matrixRotation;
	}

	m_matrix = m_matrix * m_matrix.MakeTranslation(position);
	UpdateTransform();
	UpdateLocalMatrix();
}

void GameObject::UpdateLocalMatrix()
{
	if (GetParent())
	{
		auto dxMatrix = *(DirectX::XMMATRIX*)((void*)&GetParent()->GetMatrix());
		auto determinant = DirectX::XMMatrixDeterminant(dxMatrix);
		dxMatrix = DirectX::XMMatrixInverse(&determinant, dxMatrix);
		GetLocalMatrix() = GetMatrix() * (*(Math::Matrix*)((void*)&dxMatrix));
	}
	else
	{
		GetLocalMatrix() = Math::Matrix();
	}
}

GameObject* GameObject::GetRootGameObject()
{
	if (m_parent)
	{
		return m_parent->GetRootGameObject();
	}
	return this;
}


void GameObject::UpdateTransform()
{
	UpdateLocalMatrix();
	for (auto& child : GetChildren())
	{
		UpdateLocalMatrix();
		child->GetMatrix() = child->GetLocalMatrix() * GetMatrix();
		child->UpdateTransform();
	}
}