#pragma once
#include "Undo.h"

class GameObject;

namespace Math
{
    class Matrix;
}

class UndoCreateGameObject :
    public Undo
{
public:
    UndoCreateGameObject(const GameObject* undoState);
    ~UndoCreateGameObject() override;

    void Execute() override;

private:
    size_t m_undoStates;
};

