#include "UndoTransform.h"
#include "Matrix.h"
#include "GameObject.h"
#include <ObjectManager.h>

UndoTransform::UndoTransform(const std::vector<std::pair<GameObject*, Math::Matrix>>& undoState)
{
	for (auto& [object, matrix] : undoState)
	{

		m_undoStates.emplace_back(object->GetId(), matrix);
	}
}

UndoTransform::~UndoTransform()
{
}

void UndoTransform::Execute()
{
	
	auto& objects = ObjectManager::GetInstance()->GetGameObjects();


	for (auto& [id, matrix] : m_undoStates)
	{
		for (auto& obj : objects)
		{
			if (obj->GetId() == id)
			{
				obj->SetWorldMatrix(matrix);
				break;
			}
		}
	}
}
