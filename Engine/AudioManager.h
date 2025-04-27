#pragma once
#include <pch.h>
#include <phonon.h>
#include "AudioSource.h"
#include <shared_mutex>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <thread>
class AudioManager
{
public:
	static AudioManager* GetInstance() {
		static AudioManager instance;
		return &instance;
	}
	AudioManager();

	void Deinitialize();

	std::shared_ptr<AudioSource> GetAudioSource(const std::filesystem::path& file);

	std::unordered_map<std::string, std::shared_ptr<AudioSource>> GetCachedAudioSources();

	void AddAudioSource(std::shared_ptr<AudioSource> AudioSource);



	int GetFrameSize() { return m_frameSize; }

	IPLContext GetContext() { return m_context; }
	IPLAudioSettings GetAudioSettings() { return m_audioSettings; }
	IPLHRTF GetHRTF() { return m_hrtf; }
	IPLSimulator GetSimulator() { return m_simulator; }


private:
	void ProcessAudioThread(std::stop_token stopToken);

	std::jthread m_audioThread;
	IPLContext m_context;
	IPLHRTF m_hrtf;
	int m_frameSize;


	IPLAudioBuffer m_attenuationBufferMono;
	IPLAudioBuffer m_attenuationBufferStereo;
	IPLAudioBuffer m_binauralBuffer;

	std::unordered_map<std::string, std::shared_ptr<AudioSource>> m_audioSources;
	mutable std::shared_mutex m_audioSourceLoadingMutex;
	IPLAudioSettings m_audioSettings;

	IPLSimulator m_simulator;
	IPLScene m_scene;

	IAudioClient* m_audioClient;
	IAudioRenderClient* m_renderClient;
	BYTE* m_pData;
	HANDLE m_hEvent;
};

