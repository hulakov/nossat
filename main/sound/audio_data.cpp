#include "audio_data.h"
#include <cassert>

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

AudioData::AudioData(AudioFormat format, size_t num_samples) : m_format(format)
{
    resize(num_samples);
}

AudioData::AudioData(AudioFormat format, std::vector<int8_t> data) : m_format(format), m_data(std::move(data))
{
}

size_t AudioData::get_num_samples() const
{
    const size_t bytes_per_sample = m_format.bits_per_sample / 8;
    return m_data.size() / (bytes_per_sample * m_format.num_channels);
}

void AudioData::set_format(AudioFormat format, size_t num_samples)
{
    m_format = format;
    resize(num_samples);
}

void AudioData::resize(size_t num_samples)
{
    // TODO 24 bits
    const size_t bytes_per_sample = m_format.bits_per_sample / 8;
    const size_t lenght = num_samples * bytes_per_sample * m_format.num_channels;
    m_data.resize(lenght);
}

template <typename ItemType>
static void add_channels_impl(ItemType *data, size_t num_samples, size_t old_num_channels, size_t new_num_channels)
{
    for (int i = num_samples - 1; i >= 0; i--)
    {
        const int new_offset = i * new_num_channels;
        const int old_offset = i * old_num_channels;

        for (int j = new_num_channels - 1; j >= old_num_channels; j--)
            data[new_offset + j] = 0;

        for (int j = old_num_channels - 1; j >= 0; j--)
            data[new_offset + j] = data[old_offset + j];
    }
}

void AudioData::add_channels(size_t num)
{
    const size_t num_samples = get_num_samples();
    const size_t original_num_channels = get_num_channels();
    m_format.num_channels += num;
    resize(num_samples);

    switch (m_format.bits_per_sample)
    {
    case 8:
        add_channels_impl(get_data_typed<int8_t>(), num_samples, original_num_channels, m_format.num_channels);
        break;
    case 16:
        add_channels_impl(get_data_typed<int16_t>(), num_samples, original_num_channels, m_format.num_channels);
        break;
    case 32:
        add_channels_impl(get_data_typed<int32_t>(), num_samples, original_num_channels, m_format.num_channels);
        break;
    default:
        assert(!"Audio format is not supported");
        break;
    }
}

AudioData AudioData::load_wav(const std::vector<int8_t> &buffer)
{
    const auto header = reinterpret_cast<const wav_header_t *>(&buffer[0]);
    const AudioFormat audio_format = {
        .num_channels = static_cast<uint32_t>(header->NumChannels),
        .bits_per_sample = static_cast<uint32_t>(header->BitsPerSample),
        .sample_rate = static_cast<uint32_t>(header->SampleRate),
    };
    std::vector<int8_t> data(buffer.begin() + sizeof(wav_header_t), buffer.end());
    return AudioData(audio_format, std::move(data));
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
    switch (m_format.bits_per_sample)
    {
    case 8:
        adjust_volume_impl<int8_t>(m_data, factor);
        break;
    case 16:
        adjust_volume_impl<int16_t>(m_data, factor);
        break;
    case 32:
        adjust_volume_impl<int32_t>(m_data, factor);
        break;
    default:
        assert(!"Audio format is not supported");
        break;
    }
}

void AudioData::join(const AudioData &audio)
{
    assert(m_format == audio.m_format);
    m_data.insert(m_data.end(), audio.m_data.begin(), audio.m_data.end());
}

int32_t AudioData::get_value(uint32_t sample, uint32_t channel) const
{
    const uint32_t bytes_per_sample = (m_format.bits_per_sample / 8);
    uint32_t offset = sample * bytes_per_sample * m_format.num_channels + bytes_per_sample * channel;

    const int8_t *ptr = m_data.data() + offset;
    switch (m_format.bits_per_sample)
    {
    case 8:
        return static_cast<int32_t>(*ptr);
    case 16:
        return static_cast<int32_t>(*reinterpret_cast<const int16_t *>(ptr));
    case 32:
        return *reinterpret_cast<const int32_t *>(ptr);
    default:
        assert(!"Audio format is not supported");
        break;
    };
}

void AudioData::set_value(uint32_t sample, uint32_t channel, int32_t value)
{
    const uint32_t bytes_per_sample = (m_format.bits_per_sample / 8);
    uint32_t offset = sample * bytes_per_sample * m_format.num_channels + bytes_per_sample * channel;

    int8_t *ptr = m_data.data() + offset;
    switch (m_format.bits_per_sample)
    {
    case 8:
        *ptr = static_cast<int8_t>(value);
        break;
    case 16:
        *reinterpret_cast<int16_t *>(ptr) = static_cast<int16_t>(value);
        break;
    case 32:
        *reinterpret_cast<int32_t *>(ptr) = value;
        break;
    default:
        assert(!"Audio format is not supported");
        break;
    };
}