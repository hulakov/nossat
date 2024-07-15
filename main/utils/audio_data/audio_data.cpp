#include "audio_data.h"

struct wav_header_t
{
    // The "RIFF" chunk descriptor
    uint8_t ChunkID[4];
    int32_t ChunkSize;
    uint8_t Format[4];
    // The "fmt" sub-chunk
    uint8_t Subchunk1ID[4];
    int32_t Subchunk1Size;
    int16_t AudioFormat;
    int16_t NumChannels;
    int32_t SampleRate;
    int32_t ByteRate;
    int16_t BlockAlign;
    int16_t BitsPerSample;
    // The "data" sub-chunk
    uint8_t Subchunk2ID[4];
    int32_t Subchunk2Size;
};

AudioData AudioData::load_wav(const std::vector<uint8_t> &buffer)
{
    const auto header = reinterpret_cast<const wav_header_t *>(&buffer[0]);
    return AudioData{
        .data = std::vector<int8_t>(buffer.begin() + sizeof(wav_header_t), buffer.end()),
        .num_channels = static_cast<uint32_t>(header->NumChannels),
        .bits_per_sample = static_cast<uint32_t>(header->BitsPerSample),
        .sample_rate = static_cast<uint32_t>(header->SampleRate),
    };
}

template <typename ItemType> static void adjust_volume_impl(std::vector<int8_t> &buffer, float factor)
{
    auto typed_buffer = reinterpret_cast<ItemType *>(buffer.data());
    auto typed_size = buffer.size() / sizeof(ItemType);

    for (size_t i = 0; i < typed_size; i++)
        typed_buffer[i] = static_cast<ItemType>(typed_buffer[i] * factor);
}

void AudioData::adjust_volume(float factor)
{
    switch (bits_per_sample)
    {
    case 8:
        adjust_volume_impl<int8_t>(data, factor);
        break;
    case 16:
        adjust_volume_impl<int16_t>(data, factor);
        break;
    case 32:
        adjust_volume_impl<int32_t>(data, factor);
        break;
    }
}