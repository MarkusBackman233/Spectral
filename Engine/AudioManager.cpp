#include "AudioManager.h"
#include <vector>
#include <ostream>
#include <combaseapi.h>
#include <thread>
#include "iRender.h"
#include "AudioSourceComponent.h"
#include "ObjectManager.h"



AudioManager::AudioManager() : m_context(nullptr)
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "CoInitialize failed: " << std::hex << hr << "\n";
    }

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        std::cerr << "CoCreateInstance failed: " << std::hex << hr << "\n";

    }

    IMMDevice* pDevice = nullptr;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) {
        std::cerr << "GetDefaultAudioEndpoint failed: " << std::hex << hr << "\n";

    }

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&m_audioClient);
    pDevice->Release();
    if (FAILED(hr)) {
        std::cerr << "Activate failed: " << std::hex << hr << "\n";

    }

    WAVEFORMATEX* pwfx = nullptr;
    hr = m_audioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        std::cerr << "GetMixFormat failed: " << std::hex << hr << "\n";

    }

    // Create an event for buffer-ready notifications
    m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_hEvent == nullptr) {
        std::cerr << "CreateEvent failed: " << GetLastError() << "\n";

    }

    // Initialize with event-driven mode
    //REFERENCE_TIME hnsBufferDuration = 10000000; // 1 second
    REFERENCE_TIME hnsBufferDuration = 500000;
    hr = m_audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        hnsBufferDuration,
        0,
        pwfx,
        nullptr
    );
    if (FAILED(hr)) {
        std::cerr << "Initialize failed: " << std::hex << hr << "\n";

    }

    hr = m_audioClient->SetEventHandle(m_hEvent);
    if (FAILED(hr)) {
        std::cerr << "SetEventHandle failed: " << std::hex << hr << "\n";

    }

    hr = m_audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_renderClient);
    if (FAILED(hr)) {
        std::cerr << "GetService failed: " << std::hex << hr << "\n";

    }

    UINT32 bufferFrameCount;
    hr = m_audioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) {
        std::cerr << "GetBufferSize failed: " << std::hex << hr << "\n";
    }
    m_frameSize = bufferFrameCount;
    // Pre-fill the buffer

    hr = m_renderClient->GetBuffer(bufferFrameCount, &m_pData);
    if (FAILED(hr)) {
        std::cerr << "GetBuffer failed: " << std::hex << hr << "\n";
    }
    memset(m_pData, 0, bufferFrameCount * pwfx->nBlockAlign);
    hr = m_renderClient->ReleaseBuffer(bufferFrameCount, 0);
    if (FAILED(hr)) {
        std::cerr << "ReleaseBuffer failed: " << std::hex << hr << "\n";
    }

    hr = m_audioClient->Start();
    if (FAILED(hr)) {
        std::cerr << "Start failed: " << std::hex << hr << "\n";

    }




    static IPLContextSettings contextSettings{};
    contextSettings.version = STEAMAUDIO_VERSION;
    iplContextCreate(&contextSettings, &m_context);

    const int samplingrate = 44100;
    m_audioSettings = IPLAudioSettings{ samplingrate, GetFrameSize() };


    {
        IPLHRTFSettings hrtfSettings{};
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        hrtfSettings.volume = 1.0f;
        iplHRTFCreate(m_context, &m_audioSettings, &hrtfSettings, &m_hrtf);
    }

    iplAudioBufferAllocate(m_context, 1, GetFrameSize(), &m_attenuationBufferMono);
    iplAudioBufferAllocate(m_context, 2, GetFrameSize(), &m_attenuationBufferStereo);
    iplAudioBufferAllocate(m_context, 2, GetFrameSize(), &m_binauralBuffer);
    

    //{
    //    IPLSimulationSettings simulationSettings{};
    //    simulationSettings.flags = IPL_SIMULATIONFLAGS_DIRECT; // this enables occlusion/transmission simulation
    //    simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
    //    iplSimulatorCreate(m_context, &simulationSettings, &m_simulator);
    //
    //
    //    iplSimulatorSetScene(m_simulator, m_scene);
    //    iplSimulatorCommit(m_simulator);
    //}



    m_audioThread = std::jthread([this](std::stop_token stopToken) 
    {
        this->ProcessAudioThread(stopToken);
    });
}

void AudioManager::Deinitialize()
{
    m_audioThread.request_stop();
    if (m_audioThread.joinable()) 
    {
        m_audioThread.join();
    }

    m_audioClient->Stop();
    m_renderClient->Release();
    CloseHandle(m_hEvent);
//CoTaskMemFree(pwfx);
    m_audioClient->Release();
//CoUninitialize();
    iplAudioBufferFree(m_context, &m_attenuationBufferMono);
    iplAudioBufferFree(m_context, &m_attenuationBufferStereo);
    iplAudioBufferFree(m_context, &m_binauralBuffer);
//iplBinauralEffectRelease(&biEffect);
//iplDirectEffectRelease(&dirEffect);
    iplHRTFRelease(&m_hrtf);
    iplContextRelease(&m_context);
}

void AudioManager::ProcessAudioThread(std::stop_token stopToken)
{


    HRESULT hr;

    //while (GetTickCount() - startTime < duration_seconds * 1000) {
    while (!stopToken.stop_requested()) {
        DWORD waitResult = WaitForSingleObject(m_hEvent, 5000); // wait up to 5s
        if (waitResult != WAIT_OBJECT_0) {
            std::cerr << "Wait error or timeout.\n";
            break;
        }

        // Buffer is ready
        UINT32 padding = 0;
        hr = m_audioClient->GetCurrentPadding(&padding);
        if (FAILED(hr)) {
            std::cerr << "GetCurrentPadding failed: " << std::hex << hr << "\n";
            break;
        }

        int framesAvailable = std::min( static_cast<int>(GetFrameSize() - padding), GetFrameSize());

        if (framesAvailable == 0) 
            continue;

        hr = m_renderClient->GetBuffer(framesAvailable, &m_pData);
        if (FAILED(hr)) {
            std::cerr << "GetBuffer failed: " << std::hex << hr << "\n";
            break;
        }
        std::vector<float> mixedAudio(2 * GetFrameSize());

        auto audioSourceComponents = ObjectManager::GetInstance()->GetComponentsOfType(Component::Type::AudioSource);
        for (auto& audioCompWeak : audioSourceComponents) // might cause some race conition
        {
            if (audioCompWeak.expired())
            {
                continue;
            }

            auto audioComp = std::dynamic_pointer_cast<AudioSourceComponent>(audioCompWeak.lock());
            auto audioSource = audioComp->GetAudioSource();
            if (audioSource == nullptr || audioComp->IsPlaying() == false)
            {
                continue;
            }
            
            int currentFramesAvailable = std::min(framesAvailable, audioComp->GetSamplesLeft() );


            m_attenuationBufferMono.numSamples = currentFramesAvailable;
            m_attenuationBufferStereo.numSamples = currentFramesAvailable;
            m_binauralBuffer.numSamples = currentFramesAvailable;
            auto settings = audioComp->GetSettings();
            auto& samples = audioSource->GetAudioFile().samples;

            int channels = static_cast<int>(audioSource->GetAudioFile().samples.size());

            std::vector<float> outputaudioframe(2 * GetFrameSize(), 0.0f);


            std::vector<float*> data;
            for (size_t i = 0; i < channels; i++)
            {
                data.push_back(&samples[i][audioComp->GetCurrentSample()]);
            }
            

            IPLAudioBuffer inBuffer{ channels, currentFramesAvailable, data.data() };

            if (settings.Localize)
            {
                IPLAudioBuffer* attenuationBuffer = nullptr;
                IPLDirectEffect* dirEffect = nullptr;
                if (channels == 2)
                {
                    attenuationBuffer = &m_attenuationBufferStereo;
                    dirEffect = &audioComp->dirEffectStereo;
                }
                else
                {
                    attenuationBuffer = &m_attenuationBufferMono;
                    dirEffect = &audioComp->dirEffectMono;
                }
                Math::Matrix cameraPose;
                if (ObjectManager::GetInstance()->GetMainCameraGameObject())
                {
                    cameraPose = ObjectManager::GetInstance()->GetMainCameraGameObject()->GetWorldMatrix();
                }
                else
                {
                    cameraPose = Render::GetCamera()->GetWorldMatrix();
                }

                Math::Vector3 sourcePos = audioComp->GetOwner()->GetWorldMatrix().GetPosition();
                Math::Vector3 camPosNorm = (cameraPose.GetPosition() - sourcePos).GetNormal().TransformNormal(cameraPose.GetInverse());


                float distance = (cameraPose.GetPosition() - sourcePos).Length();

                float minDistance = settings.MinDistance;
                {
                    IPLDirectEffectParams params{};
                    params.flags = IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION;

    
                    float maxDistance = settings.MaxDistance;
                    float rolloff = settings.Rolloff;

                    if (distance <= minDistance)
                    {
                        params.distanceAttenuation = 1.0f;
                    }

                    else if (distance >= maxDistance)
                    {
                        params.distanceAttenuation = 0.0f;
                    }
                    else
                    {
                        float normalized = (distance - minDistance) / (maxDistance - minDistance);
                        params.distanceAttenuation = powf(1.0f - normalized, rolloff);
                    }

                    
                    iplDirectEffectApply(*dirEffect, &params, &inBuffer, attenuationBuffer);
                }
                {
                    IPLBinauralEffectParams params;
                    params.direction = IPLVector3{ -camPosNorm.x, camPosNorm.y, -camPosNorm.z };
                    params.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
                    params.spatialBlend = 1.0f;
                    //if (distance <= minDistance)
                    //{
                    //    params.spatialBlend = 0.0f;
                    //}
                    params.hrtf = m_hrtf;
                    params.peakDelays = nullptr;
                
                    iplBinauralEffectApply(audioComp->biEffect, &params, attenuationBuffer, &m_binauralBuffer);
                }
                //{
                //    IPLPanningEffectParams params{ IPLVector3{ -camPosNorm.x, camPosNorm.y, -camPosNorm.z } };
                //    iplPanningEffectApply(audioComp->surroundEffect, &params, attenuationBuffer, &m_binauralBuffer);
                //}
                iplAudioBufferInterleave(m_context, &m_binauralBuffer, outputaudioframe.data());

            }
            else
            {
                iplAudioBufferInterleave(m_context, &inBuffer, outputaudioframe.data());
            }


            float volume = settings.Volume;

            for (size_t i = 0; i < currentFramesAvailable * 2; i++)
            {
                mixedAudio[i] += outputaudioframe[i] * volume;
            }

            audioComp->IncrementCurrentSample(currentFramesAvailable);
        }


        //for (size_t i = 0; i < framesAvailable * 2; i++) {
        //    mixedAudio[i] = std::clamp(mixedAudio[i], -1.0f, 1.0f);
        //}
        memcpy(m_pData, mixedAudio.data(), framesAvailable * sizeof(float) * 2);
        hr = m_renderClient->ReleaseBuffer(framesAvailable, 0);

        if (FAILED(hr)) {
            std::cerr << "ReleaseBuffer failed: " << std::hex << hr << "\n";
            break;
        }
    }

    std::cout << "Audio stopped.\n";
    

}