#include "AudioSource.h"

AudioFile<float>& AudioSource::GetAudioFile()
{
    return m_audioFile;
}

bool AudioSource::Load(const std::filesystem::path& file)
{
    m_audioFile.load(file.string());

    return true;
}
