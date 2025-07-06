#include "UndoCreateGameObject.h"
#include "Matrix.h"
#include "GameObject.h"
#include <ObjectManager.h>
#include "Editor.h"
UndoCreateGameObject::UndoCreateGameObject(const GameObject* undoState)
{
	m_undoStates = undoState->GetId();
}

UndoCreateGameObject::~UndoCreateGameObject()
{
}

void UndoCreateGameObject::Execute()
{
	
	auto& objects = ObjectManager::GetInstance()->GetGameObjects();

	for (auto& obj : objects)
	{
		if (obj->GetId() == m_undoStates)
		{
			ObjectManager::GetInstance()->Destroy(obj.get());
			Editor::GetInstance()->GetObjectSelector()->SetSelectedGameObject(nullptr);
			break;
		}
	}
}
