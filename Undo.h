#pragma once
#include "pch.h"
#include "Matrix.h"

class GameObject;

enum UndoOperator
{
	UNDO_CHANGE_SELECTION,
	UNDO_EDIT_TRANSFORM,

	UNDEFINED,
};

union ChangedItem
{
	GameObject* gameObject;
};

union ChangedValue
{
	Math::Matrix matrix;
};

class Undo
{
	public:
		Undo(UndoOperator undoOperator, ChangedItem changedItem, ChangedValue changedValue);

		void Execute();

	private:
		UndoOperator m_operator;
		ChangedItem m_item;
		ChangedValue m_value;

};