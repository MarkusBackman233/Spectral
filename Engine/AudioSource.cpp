#include "AudioSource.h"
#include "IOManager.h"
#include "AudioManager.h"

bool AudioSource::Load(const std::filesystem::path& file)
{
    m_fileName = file.filename().string();

    m_audioFile.load(file.string());


    return true;

}
