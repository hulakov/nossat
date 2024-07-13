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

bool AudioOutput::play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                             const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                             uint32_t bits_per_sample, size_t channels)
{
    return true;
}
