#pragma once
#include <pch.h>
#include <phonon.h>
#include "AudioFile.h"



class AudioSource
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

	bool Load(const std::filesystem::path& file);


	std::string m_fileName;


	AudioFile<float> m_audioFile;
};

