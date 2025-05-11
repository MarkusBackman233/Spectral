#pragma once
#include "AudioFile.h"
#include "LoadableResource.h"


class AudioSource : public Resource
{
public:

	struct Settings
	{
		float MinDistance = 2.0f;
		float MaxDistance = 200.0f;
		float Rolloff = 1.0f;
		float Volume = 1.0f;
		bool Looping = false;
		bool PlayOnStart = false;
		bool Mono = true;
		bool Localize = true;

	};


	AudioFile<float>& GetAudioFile();

	bool Load(const std::filesystem::path& file) override;

private:

	AudioFile<float> m_audioFile;
};

