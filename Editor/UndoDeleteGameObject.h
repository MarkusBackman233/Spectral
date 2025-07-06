#pragma once
#include "Undo.h"

class GameObject;

namespace Math
{
    class Matrix;
}

class UndoDeleteGameObject :
    public Undo
{
public:
    UndoDeleteGameObject(const std::vector<GameObject*>& undoState);
    ~UndoDeleteGameObject() override;

    void Execute() override;

private:
    void DestroyGameObject(GameObject* gameObject);

    std::vector<GameObject*> m_undoStates;
};

