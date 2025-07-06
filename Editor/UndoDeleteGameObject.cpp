#include "UndoDeleteGameObject.h"
#include "GameObject.h"
#include "EditorUtils.h"
#include <ObjectManager.h>
UndoDeleteGameObject::UndoDeleteGameObject(const std::vector<GameObject*>& undoState)
{
	for (auto& object : undoState)
	{
		GameObject* obj = new GameObject(object->GetId());
		obj->SetName(object->GetName());

		m_undoStates.push_back(obj);

		EditorUtils::DuplicateGameObject(obj, object, false);
	}
}

UndoDeleteGameObject::~UndoDeleteGameObject()
{
	for (GameObject* gameObject : m_undoStates)
	{
		DestroyGameObject(gameObject);
	}
}
void UndoDeleteGameObject::DestroyGameObject(GameObject* gameObject)
{
	for (size_t i = 0; i < gameObject->GetChildren().size(); i++)
	{
		DestroyGameObject(gameObject->GetChildren()[i]);
	}

	delete gameObject;
}

void UndoDeleteGameObject::Execute()
{
	for (GameObject* gameObject : m_undoStates)
	{
		auto obj = ObjectManager::GetInstance()->CreateObject(gameObject->GetName(), false, gameObject->GetId());
		EditorUtils::DuplicateGameObject(obj, gameObject, true);
	}
}
