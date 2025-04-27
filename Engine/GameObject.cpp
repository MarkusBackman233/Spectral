
#include "GameObject.h"
#include "Component.h"
#include <iostream>
#include "Mesh.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "MeshComponent.h"
#include "CharacterControllerComponent.h"
#include "AudioSourceComponent.h"
#include "random"
#include "limits"
#include "RigidbodyComponent.h"
#include "MathFunctions.h"
#include "Editor.h"
#include "ObjectManager.h"

GameObject::GameObject()
	: m_components{}
	, m_shouldDestroyOnReset(false)
{
	std::random_device rd;
	std::mt19937_64 eng(rd());
	std::uniform_int_distribution<unsigned long long> distr;
	m_id = distr(eng);


}

GameObject::~GameObject()
{
	for (auto& component : m_components)
	{
		ObjectManager::GetInstance()->UnregisterComponent(component);
	}
}

void GameObject::SetWorldMatrix(const Math::Matrix& worldMatrix)
{
	m_worldMatrix = worldMatrix;
	UpdateTransform();
}

void GameObject::SetLocalMatrix(const Math::Matrix& localMatrix)
{
	m_localMatrix = localMatrix;
	UpdateTransform();
}

void GameObject::SetPosition(const Math::Vector3& position)
{
	m_worldMatrix.SetPosition(position);
	UpdateTransform();
	if (auto rigidbody = GetComponentOfType<RigidbodyComponent>())
	{
		rigidbody->GetActor()->setGlobalPose(PhysXManager::MatrixToPxTransform(m_worldMatrix));
	}
}

void GameObject::SetRotation(const Math::Vector3& rotationInDegrees)
{
	auto scale = m_worldMatrix.GetScale();
	auto position = m_worldMatrix.GetPosition();
	m_worldMatrix = Math::Matrix::MakeRotationX(rotationInDegrees.x) * Math::Matrix::MakeRotationY(rotationInDegrees.y) * Math::Matrix::MakeRotationZ(rotationInDegrees.z);
	SetPosition(position);

	m_worldMatrix = Math::Matrix::MakeScale(scale) * m_worldMatrix;
	UpdateTransform();
}

void GameObject::Rotate(const Math::Vector3& rotationInDegrees)
{
	Math::Vector3 rotationInRads = rotationInDegrees; // Assuming input is in radians

	Math::Vector3 currentRotation = m_worldMatrix.GetRotationXYZInRads();
	Math::Vector3 newRotation = currentRotation + rotationInRads;

	newRotation.x = fmodf(newRotation.x, Math::TwoPI);
	newRotation.y = fmodf(newRotation.y, Math::TwoPI);
	newRotation.z = fmodf(newRotation.z, Math::TwoPI);

	if (newRotation.x < 0) newRotation.x += Math::TwoPI;
	if (newRotation.y < 0) newRotation.y += Math::TwoPI;
	if (newRotation.z < 0) newRotation.z += Math::TwoPI;

	auto scale = m_worldMatrix.GetScale();
	auto position = m_worldMatrix.GetPosition();

	m_worldMatrix = Math::Matrix::MakeRotationX(newRotation.x) *
		Math::Matrix::MakeRotationY(newRotation.y) *
		Math::Matrix::MakeRotationZ(newRotation.z);

	SetPosition(position);
	m_worldMatrix = Math::Matrix::MakeScale(scale) * m_worldMatrix;

	UpdateTransform();
}

Math::Vector3 GameObject::GetRotation()
{
	return m_worldMatrix.GetRotationXYZInRads();
}

void GameObject::SetWorldMatrixNoUpdate(const Math::Matrix& worldMatrix)
{
	m_worldMatrix = worldMatrix;
}

void GameObject::SetLocalMatrixNoUpdate(const Math::Matrix& localMatrix)
{
	m_localMatrix = localMatrix;
}	

const Math::Vector3& GameObject::GetPosition()
{
	//m_worldMatrix.data
	return *reinterpret_cast<Math::Vector3*>(&m_worldMatrix.data[3][0]);
}

void GameObject::SetParent(GameObject* newParent)
{
	if (m_parent)
	{
		m_parent->RemoveChild(this);
	}

	if (newParent)
	{
		newParent->AddChild(this);
	}
	m_parent = newParent;

	UpdateLocalMatrix();
}

void GameObject::DestroyOnReset()
{
	m_shouldDestroyOnReset = true;
}

bool GameObject::ShouldDestroyOnReset() const
{
	return m_shouldDestroyOnReset;
}

void GameObject::RemoveComponent(std::shared_ptr<Component> component)
{
	auto it = std::find_if(m_components.begin(), m_components.end(),
		[&](std::shared_ptr<Component> p) { return p.get() == component.get(); });

	AssertAndReturn(it != m_components.end(), "Could not remove component!", return);
	m_components.erase(it);

}

void GameObject::AddComponent(std::shared_ptr<Component> component)
{
	Assert(component != nullptr, "Tried to add a null component");
	m_components.push_back(component);
}

void GameObject::Start()
{
#ifdef EDITOR
	m_initialLocalMatrix = m_localMatrix;
	m_initialWorldMatrix = m_worldMatrix;
#endif
}

void GameObject::Reset()
{
#ifdef EDITOR
	 m_localMatrix = m_initialLocalMatrix;
	 m_worldMatrix = m_initialWorldMatrix;
	 UpdateTransform();
#endif
}

void GameObject::AddChild(GameObject* gameObject)
{
	m_children.push_back(gameObject);
	gameObject->m_parent = this;
	gameObject->UpdateLocalMatrix();
}

void GameObject::RemoveChild(GameObject* gameObject)
{
	auto it = std::find_if(m_children.begin(), m_children.end(),
		[&](GameObject* p) { return p->GetId() == gameObject->GetId(); });

	AssertAndReturn(it != m_children.end(), "Could not remove child!", return);

	m_children.erase(it);
}


CharacterControllerComponent* GameObject::GetCharacterControllerComponent() const
{
	return GetComponentOfType<CharacterControllerComponent>().get();
}

AudioSourceComponent* GameObject::GetAudioSourceComponent() const
{
	return GetComponentOfType<AudioSourceComponent>().get();
}

void GameObject::SetName(const std::string& name)
{
	m_name = name;
}
/*
void GameObject::Translate(const Math::Vector3& position, const Math::Vector4& quat)
{
	m_worldMatrix = m_worldMatrix.MakeIdentity();

	if (quat.x != 0 || quat.y != 0 || quat.z != 0)
	{
		float angle = 2 * acos(quat.w);
		Math::Vector4 axis = quat / sqrt(1.0f - quat.w * quat.w);

		Math::Matrix matrixRotation = Math::Matrix::MakeRotationAA(Math::Vector3(axis.x, axis.y, axis.z), angle);
		m_worldMatrix = m_worldMatrix * matrixRotation;
	}

	m_worldMatrix = m_worldMatrix * m_worldMatrix.MakeTranslation(position);
	UpdateTransform();
}
*/
void GameObject::UpdateLocalMatrix()
{
	if (m_parent)
	{
		m_localMatrix = m_worldMatrix * m_parent->GetWorldMatrix().GetInverse();
	}
	else
	{
		m_localMatrix = Math::Matrix();
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

unsigned long long GameObject::GetId() const
{
	return m_id;
}

void GameObject::UpdateTransform()
{
	UpdateLocalMatrix();
	UpdateChildrenGlobalMatrix();
}


void GameObject::UpdateChildrenGlobalMatrix()
{
	for (auto& child : GetChildren())
	{
		child->SetWorldMatrix(child->GetLocalMatrix() * GetWorldMatrix());
		child->UpdateChildrenGlobalMatrix();
	}
}
