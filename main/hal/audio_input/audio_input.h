#pragma once

#include "utils/audio_data/audio_data.h"

#include <vector>
#include <memory>

class AudioInput
{
public:
    AudioInput();
    ~AudioInput();

    void capture_audio(AudioData &audio);
    const AudioFormat &get_audio_format() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
