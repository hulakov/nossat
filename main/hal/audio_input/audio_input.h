#pragma once

#include <vector>
#include <memory>

class AudioInput
{
  public:
    static const int MICROPHONE_CHANNEL_COUNT;
    static const int REFERENCE_CHANNEL_COUNT;

  public:
    AudioInput();
    ~AudioInput();

    bool capture_audio(std::vector<int16_t> &buffer, size_t chunk_size);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
