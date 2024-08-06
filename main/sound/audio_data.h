#pragma once

#include <vector>
#include <cstdint>

struct AudioFormat
{
    bool operator==(const AudioFormat &other) const
    {
        return other.num_channels == num_channels && other.bits_per_sample == bits_per_sample &&
               other.sample_rate == sample_rate;
    }

    uint32_t num_channels = 0;
    uint32_t bits_per_sample = 0;
    uint32_t sample_rate = 0;
};

class AudioData
{
public:
    AudioData() = default;
    AudioData(AudioFormat format, size_t num_samples);
    AudioData(AudioFormat format, std::vector<int8_t> data);

    static AudioData load_wav(const std::vector<int8_t> &buffer);

    void adjust_volume(float factor);

    int8_t *get_data() { return m_data.data(); }
    template <typename T> T *get_data_typed() { return reinterpret_cast<T *>(get_data()); }
    const int8_t *get_data() const { return m_data.data(); }
    template <typename T> const T *get_data_typed() const { return reinterpret_cast<const T *>(get_data()); }
    size_t get_size() const { return m_data.size(); }
    bool is_empty() const { return m_format == AudioFormat(); }
    void join(const AudioData &data);

    int32_t get_value(uint32_t sample, uint32_t channel) const;
    void set_value(uint32_t sample, uint32_t channel, int32_t value);

    const AudioFormat &get_format() const { return m_format; }
    uint32_t get_num_channels() const { return m_format.num_channels; }
    uint32_t get_bits_per_sample() const { return m_format.bits_per_sample; }
    uint32_t get_sample_rate() const { return m_format.sample_rate; }
    size_t get_num_samples() const;
    void set_format(AudioFormat format, size_t num_samples);

    void resize(size_t num_samples);
    void add_channels(size_t num);

private:
    AudioFormat m_format;
    std::vector<int8_t> m_data;
};
