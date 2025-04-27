#pragma once
#ifdef EDITOR
#include "PropertyWindow.h"
#include <string>
class AudioSource;

class AudioSourcePropertyWindow :
    public PropertyWindow
{
public:
    AudioSourcePropertyWindow(std::function<void(std::shared_ptr<AudioSource>)> onSelectedAudioSource);

protected:
    void PopulateWindow() override;

private:
    std::function<void(const std::shared_ptr<AudioSource>)> m_onSelectedAudioSource;
};

#endif