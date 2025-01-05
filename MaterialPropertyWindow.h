#pragma once
#ifdef EDITOR
#include "PropertyWindow.h"
#include <string>
class Material;

class MaterialPropertyWindow :
    public PropertyWindow
{
public:
    MaterialPropertyWindow(std::function<void(std::shared_ptr<Material>)> onSelectedMaterial);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<Material>)> m_onSelectedMaterial;
};

#endif