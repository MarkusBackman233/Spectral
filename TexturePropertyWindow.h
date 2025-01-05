#pragma once
#ifdef EDITOR
#include "PropertyWindow.h"
#include <string>
class Texture;

class TexturePropertyWindow :
    public PropertyWindow
{
public:
    TexturePropertyWindow(std::function<void(std::shared_ptr<Texture>)> onSelectedTexture, const std::string& currentSelectedTextureName = "");

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<Texture>)> m_onSelectedTexture;
    std::string m_currentSelectedTextureName;
};

#endif