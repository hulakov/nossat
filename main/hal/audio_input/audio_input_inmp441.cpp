#include "audio_input.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "driver/i2s_std.h"

static const AudioFormat MICROPHONE_AUDIO_FORMAT = {
    .num_channels = 2,
    .bits_per_sample = 16,
    .sample_rate = 16000,
};

constexpr const auto SLOT_MODE = I2S_SLOT_MODE_STEREO;
constexpr const auto DATA_BIT_WIDTH = I2S_DATA_BIT_WIDTH_32BIT;

struct AudioInput::Impl
{
    i2s_chan_handle_t rx_handle = nullptr;
    std::vector<int32_t> temp_buffer;
};

static i2s_chan_handle_t bsp_i2s_microphone_init()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_AUDIO, I2S_ROLE_MASTER);
    i2s_chan_handle_t rx_handle;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(MICROPHONE_AUDIO_FORMAT.sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(DATA_BIT_WIDTH, I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = GPIO_NUM_NC,
                .bclk = GPIO_MICROPHONE_I2S_SCLK,
                .ws = GPIO_MICROPHONE_I2S_LRCK,
                .dout = GPIO_NUM_NC,
                .din = GPIO_MICROPHONE_I2S_SDIN,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    return rx_handle;
}

AudioInput::AudioInput() : m_impl(std::make_unique<Impl>())
{
    m_impl->rx_handle = bsp_i2s_microphone_init();
}

AudioInput::~AudioInput()
{
}

const AudioFormat &AudioInput::get_audio_format() const
{
    return MICROPHONE_AUDIO_FORMAT;
}

void AudioInput::capture_audio(AudioData &audio)
{
    assert(audio.get_format() == MICROPHONE_AUDIO_FORMAT);

    const size_t num_samples = audio.get_num_samples();
    m_impl->temp_buffer.resize(num_samples * audio.get_num_channels());

    size_t bytes_read;
    ESP_ERROR_CHECK(i2s_channel_read(m_impl->rx_handle, m_impl->temp_buffer.data(),
                                     m_impl->temp_buffer.size() * sizeof(int32_t), &bytes_read, portMAX_DELAY));

    int16_t *output = audio.get_data_typed<int16_t>();
    for (int i = 0; i < num_samples; i++)
    {
        // 32:8 are valid bits, 8:0 are the lower 8 bits, all are 0. The input
        // of AFE is 16-bit voice data, and 29:13 bits are used to amplify the
        // voice signal.
        // https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf
        const int offset = i * audio.get_num_channels();
        output[offset + 1] = m_impl->temp_buffer[offset + 1] >> 14;
        output[offset] = m_impl->temp_buffer[offset] >> 14;
    }
}
