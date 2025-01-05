#pragma once
#include "PropertyWindow.h"

class Mesh;

class MeshPropertyWindow :
    public PropertyWindow
{
public:
    MeshPropertyWindow(std::function<void(std::shared_ptr<Mesh>)> onSelectedMesh);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<Mesh>&)> m_onSelectedMesh;
};

