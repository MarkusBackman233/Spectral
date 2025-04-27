#ifdef EDITOR
#include "AudioSourcePropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "StringUtils.h"
#include "AudioSource.h"
#include "AudioManager.h"

AudioSourcePropertyWindow::AudioSourcePropertyWindow(std::function<void(std::shared_ptr<AudioSource>)> onSelectedAudioSource)
    : m_onSelectedAudioSource(onSelectedAudioSource)
{

}

void AudioSourcePropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);

    for (const auto& [name, audioSource] : AudioManager::GetInstance()->GetCachedAudioSources())
    {
        if (ImGui::Button(StringUtils::StripPathFromFilename(name).c_str(), buttonSize))
        {
            m_onSelectedAudioSource(audioSource);
            CloseThisWindow();
            break;
        }
    }
}
#endif