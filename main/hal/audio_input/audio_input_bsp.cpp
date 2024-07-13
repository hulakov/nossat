#include "audio_input.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "sanity_checks.h"

static const char *TAG = "audio_input";

const int AudioInput::MICROPHONE_CHANNEL_COUNT = 2;
const int AudioInput::REFERENCE_CHANNEL_COUNT = 1;
const constexpr float CODEC_DEFAULT_ADC_VOLUME = 24.0;

static esp_codec_dev_sample_info_t DEFAULT_CODEC_DEV_SAMPLE_INFO = {
    .bits_per_sample = 16,
    .channel = 2,
    .channel_mask = 0,
    .sample_rate = 16000,
    .mclk_multiple = 0,
};

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
    ESP_ERROR_CHECK(esp_codec_dev_open(m_impl->rx_handle, &DEFAULT_CODEC_DEV_SAMPLE_INFO));
}

AudioInput::~AudioInput()
{
}

bool AudioInput::capture_audio(std::vector<int16_t> &buffer, size_t chunk_size)
{
    const constexpr int TOTAL_CHANNEL_COUNT = MICROPHONE_CHANNEL_COUNT + REFERENCE_CHANNEL_COUNT;
    assert(TOTAL_CHANNEL_COUNT == 3);
    assert(MICROPHONE_CHANNEL_COUNT == 2);
    assert(buffer.size() == chunk_size * TOTAL_CHANNEL_COUNT);

    esp_codec_dev_read(m_impl->rx_handle, &buffer[0], chunk_size * MICROPHONE_CHANNEL_COUNT * sizeof(int16_t));

    // 2 channels -> 3 channels
    for (int i = chunk_size - 1; i >= 0; i--)
    {
        buffer[i * 3 + 2] = 0;
        buffer[i * 3 + 1] = buffer[i * 2 + 1];
        buffer[i * 3 + 0] = buffer[i * 2 + 0];
    }

    return true;
}
