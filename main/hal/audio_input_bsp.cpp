#include "audio_input.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "sanity_checks.h"

static const char *TAG = "audio_input";

static const AudioFormat MICROPHONE_AUDIO_FORMAT = {
    .num_channels = 2,
    .bits_per_sample = 16,
    .sample_rate = 16000,
};

const constexpr float CODEC_DEFAULT_ADC_VOLUME = 24.0;

static esp_codec_dev_sample_info_t make_codec_config(const AudioFormat &format)
{
    return esp_codec_dev_sample_info_t{
        .bits_per_sample = static_cast<uint8_t>(format.bits_per_sample),
        .channel = static_cast<uint8_t>(format.num_channels),
        .channel_mask = 0,
        .sample_rate = format.sample_rate,
        .mclk_multiple = 0,
    };
}

struct AudioInput::Impl
{
    esp_codec_dev_handle_t rx_handle = 0;
};

AudioInput::AudioInput() : m_impl(std::make_unique<Impl>())
{
    ESP_LOGI(TAG, "Initialize microphone via BSP");
    ESP_ERROR_CHECK(bsp_i2c_init());

    m_impl->rx_handle = bsp_audio_codec_microphone_init();
    ESP_TRUE_CHECK(m_impl->rx_handle);

    ESP_ERROR_CHECK(esp_codec_dev_close(m_impl->rx_handle));
    ESP_ERROR_CHECK(esp_codec_dev_set_in_gain(m_impl->rx_handle, CODEC_DEFAULT_ADC_VOLUME));

    esp_codec_dev_sample_info_t config = make_codec_config(MICROPHONE_AUDIO_FORMAT);
    ESP_ERROR_CHECK(esp_codec_dev_open(m_impl->rx_handle, &config));
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
    esp_codec_dev_read(m_impl->rx_handle, audio.get_data(), audio.get_size());
}
