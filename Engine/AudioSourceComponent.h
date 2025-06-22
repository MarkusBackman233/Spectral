#pragma once
#include "pch.h"
#include "Component.h"
#include "AudioSource.h"
#include <phonon.h>


class AudioSourceComponent : public Component
{
public:
	AudioSourceComponent(GameObject* owner);
	AudioSourceComponent(GameObject* owner, AudioSourceComponent* audioComponent);

	Component::Type GetComponentType() override { return Component::Type::AudioSource; };
	void Update(float deltaTime) override;
	void Render() override;
	void Start() override;
	void Reset() override;

	Json::Object SaveComponent() override;

	void LoadComponent(const rapidjson::Value& object) override;

#ifdef EDITOR
	void ComponentEditor() override;
	void DisplayComponentIcon() override;
#endif // EDITOR

	std::shared_ptr<AudioSource> GetAudioSource();
	AudioSource::Settings GetSettings();

	int IncrementCurrentSample(int frameSize);
	int GetCurrentSample();
	int GetSamplesLeft();

	bool IsPlaying() const;

	void PlayAudio();

	void SetAudioSource(std::shared_ptr<AudioSource> audioSource);

	IPLDirectEffect dirEffectMono{};
	IPLDirectEffect dirEffectStereo{};
	IPLPanningEffect surroundEffect{};
	IPLBinauralEffect biEffect{};
	IPLSource source{};


private:
	void CreateAudioResources();

	bool m_isPlaying;

	AudioSource::Settings m_settings;
	std::shared_ptr<AudioSource> m_audioSource;
	int m_currentSample;



};

