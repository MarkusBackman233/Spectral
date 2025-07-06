#include "UndoSetParent.h"
#include "Matrix.h"
#include "GameObject.h"
#include <ObjectManager.h>
#include "Editor.h"
UndoSetParent::UndoSetParent(const GameObject* object, const GameObject* previousParent)
{
	m_undoStates = { object->GetId(),  previousParent ? previousParent->GetId() : 0};
}

UndoSetParent::~UndoSetParent()
{
}

void UndoSetParent::Execute()
{
	auto& objects = ObjectManager::GetInstance()->GetGameObjects();

	for (auto& obj : objects)
	{
		if (obj->GetId() == m_undoStates.first)
		{
			if (m_undoStates.second == 0)
			{
				obj->SetParent(nullptr);
			}
			else
			{
				for (auto& parent : objects)
				{
					if (parent->GetId() == m_undoStates.second)
					{
						obj->SetParent(parent.get());
					}
				}
			}
			break;
		}
	}
}
