#pragma once
#include "Undo.h"

class GameObject;

namespace Math
{
    class Matrix;
}

class UndoTransform :
    public Undo
{
public:
    UndoTransform(const std::vector<std::pair<GameObject*, Math::Matrix>>& undoState);
    ~UndoTransform() override;

    void Execute() override;

private:
    std::vector<std::pair<GameObject*, Math::Matrix>> m_undoStates;
};

