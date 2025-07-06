#pragma once
#include "Undo.h"

class GameObject;

namespace Math
{
    class Matrix;
}

class UndoSetParent :
    public Undo
{
public:
    UndoSetParent(const GameObject* object, const GameObject* previousParent);
    ~UndoSetParent() override;

    void Execute() override;

private:
    std::pair<size_t, size_t> m_undoStates;
};

