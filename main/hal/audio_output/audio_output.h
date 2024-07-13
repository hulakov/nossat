#pragma once

#include <vector>
#include <memory>

class AudioOutput
{
  public:
    AudioOutput();
    ~AudioOutput();

    bool play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                    const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                    uint32_t bits_per_sample, size_t channels);
    bool play_wav(const std::vector<uint8_t> &buffer);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};