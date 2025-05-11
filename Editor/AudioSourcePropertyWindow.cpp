#ifdef EDITOR
#include "AudioSourcePropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "AudioSource.h"
#include "ResourceManager.h"
AudioSourcePropertyWindow::AudioSourcePropertyWindow(std::function<void(std::shared_ptr<AudioSource>)> onSelectedAudioSource)
    : m_onSelectedAudioSource(onSelectedAudioSource)
{

}

void AudioSourcePropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);

    auto audioSources = ResourceManager::GetInstance()->GetResources<AudioSource>();

    for (const auto& audioSource : audioSources)
    {
        if (ImGui::Button(audioSource->GetFilename().c_str(), buttonSize))
        {
            m_onSelectedAudioSource(audioSource);
            CloseThisWindow();
            break;
        }
    }
}
#endif