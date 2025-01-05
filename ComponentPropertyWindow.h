#pragma once
#ifdef EDITOR
#include "PropertyWindow.h"
#include "Component.h"

class ComponentPropertyWindow :
    public PropertyWindow
{
public:
    ComponentPropertyWindow(std::function<void(Component::Type)> onSelectedComponent);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const Component::Type)> m_onSelectedComponent;
};

#endif