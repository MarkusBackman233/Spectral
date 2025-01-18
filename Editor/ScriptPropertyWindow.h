#pragma once
#ifdef EDITOR
#include "PropertyWindow.h"
#include <string>
class Script;

class ScriptPropertyWindow :
    public PropertyWindow
{
public:
    ScriptPropertyWindow(std::function<void(std::shared_ptr<Script>)> onSelectedMaterial);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<Script>)> m_onSelectedScript;
};

#endif