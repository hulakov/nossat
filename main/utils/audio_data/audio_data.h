#pragma once

#include <vector>
#include <cstdint>

struct AudioData
{
    std::vector<int8_t> data;
    uint32_t num_channels;
    uint32_t bits_per_sample;
    uint32_t sample_rate;

    static AudioData load_wav(const std::vector<uint8_t> &buffer);

    void adjust_volume(float factor);
};
