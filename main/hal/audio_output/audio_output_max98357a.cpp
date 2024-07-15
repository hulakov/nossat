#include "audio_output.h"

struct AudioOutput::Impl
{
};

AudioOutput::AudioOutput() : m_impl(std::make_unique<Impl>())
{
}

AudioOutput::~AudioOutput()
{
}

bool AudioOutput::play(const AudioData &audio)
{
    return true;
}
