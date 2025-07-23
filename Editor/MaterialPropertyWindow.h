#pragma once
#ifdef EDITOR
#include "PropertyWindow.h"
#include <string>
class DefaultMaterial;

class MaterialPropertyWindow :
    public PropertyWindow
{
public:
    MaterialPropertyWindow(std::function<void(std::shared_ptr<DefaultMaterial>)> onSelectedMaterial);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<DefaultMaterial>)> m_onSelectedMaterial;
};

#endif