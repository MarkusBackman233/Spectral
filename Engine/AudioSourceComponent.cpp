#include "AudioSourceComponent.h"
#include "iRender.h"
#include "TextureManager.h"
#include "Editor.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "TextureManager.h"
#include "Texture.h"
#include "RenderManager.h"
#include "PropertyWindowFactory.h"
#include "AudioManager.h"

AudioSourceComponent::AudioSourceComponent(GameObject* owner)
	: Component(owner)
	, m_audioSource(nullptr)
	, m_currentSample(0)
	, m_isPlaying(false)
{
	CreateAudioResources();
}


AudioSourceComponent::AudioSourceComponent(GameObject* owner, AudioSourceComponent* audioSourceComponent)
	: Component(owner)
	, m_currentSample(0)
	, m_isPlaying(false)
{
	SetAudioSource(audioSourceComponent->GetAudioSource());
	m_settings = audioSourceComponent->GetSettings();
	CreateAudioResources();
}

void AudioSourceComponent::Update(float deltaTime)
{

}

void AudioSourceComponent::SetAudioSource(std::shared_ptr<AudioSource> audioSource)
{
	m_audioSource = audioSource;
}

void AudioSourceComponent::CreateAudioResources()
{

	auto audioSettings = AudioManager::GetInstance()->GetAudioSettings();

	{
		IPLPanningEffectSettings effectSettings{};
		effectSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_STEREO;
		iplPanningEffectCreate(AudioManager::GetInstance()->GetContext(), &audioSettings, &effectSettings, &surroundEffect);
	}
	{
		IPLDirectEffectSettings effectSettings{ 1 };
		iplDirectEffectCreate(AudioManager::GetInstance()->GetContext(), &audioSettings, &effectSettings, &dirEffect);
	}
	{
		IPLBinauralEffectSettings effectSettings{};
		effectSettings.hrtf = AudioManager::GetInstance()->GetHRTF();
		iplBinauralEffectCreate(AudioManager::GetInstance()->GetContext(), &audioSettings, &effectSettings, &biEffect);
	}
	//{
	//	IPLSourceSettings sourceSettings{};
	//	sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT; // this enables occlusion/transmission simulator for this source
	//	iplSourceCreate(AudioManager::GetInstance()->GetSimulator(), &sourceSettings, &source);
	//	iplSourceAdd(source, AudioManager::GetInstance()->GetSimulator());
	//	iplSimulatorCommit(AudioManager::GetInstance()->GetSimulator());
	//}
}

int AudioSourceComponent::IncrementCurrentSample(int frameSize)
{
	m_currentSample += frameSize;

	if (GetSamplesLeft() <= 0)
	{
		m_currentSample = 0;
		if (m_settings.Looping == false)
		{
			m_isPlaying = false;
		}
	}

	return m_currentSample;
}

int AudioSourceComponent::GetCurrentSample()
{
	return m_currentSample;
}

void AudioSourceComponent::Render()
{

#ifdef EDITOR
	//static auto texture = TextureManager::GetInstance()->GetTexture("Light.bmp");
	//Render::DrawGuizmo(m_owner->GetWorldMatrix().GetPosition(), texture, m_light->Color.GetNormalizedColor());
	//if (m_light->Type == Light::LightType::Directional && Editor::GetInstance()->IsStarted() == false)
	//{
	//	Render::DrawLine(m_owner->GetWorldMatrix().GetPosition(), m_owner->GetWorldMatrix().GetPosition() + m_owner->GetWorldMatrix().GetFront()*2);
	//}
#endif // EDITOR
}

void AudioSourceComponent::Start()
{
	m_currentSample = 0;
	m_isPlaying = m_settings.PlayOnStart;
}

void AudioSourceComponent::Reset()
{
	m_isPlaying = false;
}



Json::Object AudioSourceComponent::SaveComponent()
{
	Json::Object object;
	if (m_audioSource != nullptr)
	{
		object.emplace("AudioSource", m_audioSource->m_fileName);
	}
	else
	{
		object.emplace("AudioSource", "Null");
	}

	object.emplace("MinDistance", m_settings.MinDistance);
	object.emplace("MaxDistance", m_settings.MaxDistance);
	object.emplace("Rolloff", m_settings.Rolloff);
	object.emplace("Volume", m_settings.Volume);
	object.emplace("Looping", m_settings.Looping);
	object.emplace("PlayOnStart", m_settings.PlayOnStart);
	object.emplace("Mono", m_settings.Mono);
	object.emplace("Localize", m_settings.Localize);
	return std::move(object);
}

void AudioSourceComponent::LoadComponent(const rapidjson::Value& object)
{
	std::string audioSourceName = object["AudioSource"].GetString();
	if (audioSourceName != "Null")
	{
		SetAudioSource(AudioManager::GetInstance()->GetAudioSource(audioSourceName));
	}


	m_settings.MinDistance = object["MinDistance"].GetFloat();
	m_settings.MaxDistance = object["MaxDistance"].GetFloat();
	m_settings.Rolloff = object["Rolloff"].GetFloat();
	m_settings.Volume = object["Volume"].GetFloat();
	m_settings.Looping = object["Looping"].GetBool();
	m_settings.PlayOnStart = object["PlayOnStart"].GetBool();
	if (object.HasMember("Mono"))
	{
		m_settings.Mono = object["Mono"].GetBool();
	}	
	if (object.HasMember("Localize"))
	{
		m_settings.Localize = object["Localize"].GetBool();
	}
}

std::shared_ptr<AudioSource> AudioSourceComponent::GetAudioSource()
{
	return m_audioSource;
}

AudioSource::Settings AudioSourceComponent::GetSettings()
{
	return m_settings;
}



int AudioSourceComponent::GetSamplesLeft()
{
	return std::max(static_cast<int>(m_audioSource->m_audioFile.samples[0].size()) - m_currentSample, 0);
}

bool AudioSourceComponent::IsPlaying() const
{
	return m_isPlaying;
}

void AudioSourceComponent::PlayAudio()
{
	m_currentSample = 0;
	m_isPlaying = true;
}

#ifdef EDITOR
void AudioSourceComponent::ComponentEditor()
{
	ImGui::DragFloat("Volume",		&m_settings.Volume,0.01f,0.0f,1.0f);
	ImGui::Checkbox("Mono",			&m_settings.Mono);
	ImGui::Checkbox("Loop",			&m_settings.Looping);
	ImGui::Checkbox("Play On Start",&m_settings.PlayOnStart);
	ImGui::Separator();
	ImGui::Checkbox("Localize",		&m_settings.Localize);
	ImGui::Indent(10.0f);
	ImGui::BeginDisabled(m_settings.Localize == false);
	ImGui::DragFloat("MaxDistance", &m_settings.MaxDistance,1.0f,0.0f,100000.0f);
	ImGui::DragFloat("MinDistance", &m_settings.MinDistance,1.0f,0.0f,100000.0f);
	ImGui::DragFloat("Rolloff",		&m_settings.Rolloff,0.01f,0.1f,2.0f);
	ImGui::EndDisabled();
	ImGui::Unindent(10.0f);
	ImGui::Separator();

	if (ImGui::Button("Select AudioSource##AudioSource"))
	{
		PropertyWindowFactory::SelectAudioSource(this);
	}
	ImGui::Text("Selected Audio Source:");
	if (m_audioSource != nullptr)
	{
		ImGui::Text(m_audioSource->m_fileName.c_str());
	}
	else
	{
		ImGui::Text("Null");
	}
}
void AudioSourceComponent::DisplayComponentIcon()
{
	//auto color = GetLight()->Color.GetNormalizedColor();
	//ImGui::SameLine();
	//ImGui::Image(TextureManager::GetInstance()->GetTexture("Light.bmp")->GetResourceView().Get(), ImVec2(15, 15), ImVec2(0, 0), ImVec2(1, 1), ImVec4(color.x, color.y, color.z, 1.0f));
}
#endif