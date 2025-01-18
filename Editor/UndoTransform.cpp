#include "UndoTransform.h"
#include "Matrix.h"
#include "GameObject.h"

UndoTransform::UndoTransform(const std::vector<std::pair<GameObject*, Math::Matrix>>& undoState)
	: m_undoStates(undoState)
{
}

UndoTransform::~UndoTransform()
{
}

void UndoTransform::Execute()
{
	for (auto& [gameObject, matrix] : m_undoStates)
	{
		gameObject->SetWorldMatrix(matrix);
	}
}
