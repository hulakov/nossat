#pragma once

#include "sound/audio_data.h"
#include <memory>

class AudioOutput
{
public:
    AudioOutput();
    ~AudioOutput();

    bool play(const AudioData &audio);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};