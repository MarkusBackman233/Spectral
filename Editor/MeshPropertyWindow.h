#pragma once
#include "PropertyWindow.h"

class Model;

class MeshPropertyWindow :
    public PropertyWindow
{
public:
    MeshPropertyWindow(std::function<void(std::shared_ptr<Model>)> onSelectedMesh);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<Model>&)> m_onSelectedMesh;
};

