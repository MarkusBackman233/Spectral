#include "Undo.h"
#include "Editor.h"
#include "GameObject.h"
Undo::Undo(UndoOperator undoOperator, ChangedItem changedItem, ChangedValue changedValue)
	: m_operator(undoOperator)
	, m_item(changedItem)
	, m_value(changedValue)
{
}

void Undo::Execute()
{
	switch (m_operator)
	{
	case UNDO_CHANGE_SELECTION:
		Editor::GetInstance()->GetSelectedGameObjects().clear();
		if (m_item.gameObject)
		{
			Editor::GetInstance()->GetSelectedGameObjects().push_back(m_item.gameObject);
		}
		break;
	case UNDO_EDIT_TRANSFORM:
		m_item.gameObject->GetMatrix() = m_value.matrix;
		m_item.gameObject->UpdateLocalMatrix();
		m_item.gameObject->UpdateTransform();
		break;
	case UNDEFINED:
		Editor::GetInstance()->LogMessage("Undefined Undo Operator!");
		break;
	}
}
