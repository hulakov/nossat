#include "audio_output.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "sanity_checks.h"

static const char *TAG = "audio_output";

esp_codec_dev_sample_info_t DEFAULT_CODEC_DEV_SAMPLE_INFO = {
    .bits_per_sample = 16,
    .channel = 2,
    .channel_mask = 0,
    .sample_rate = 16000,
    .mclk_multiple = 0,
};

struct AudioOutput::Impl
{
    esp_codec_dev_handle_t play_dev_handle = 0;
};

AudioOutput::AudioOutput() : m_impl(std::make_unique<Impl>())
{
    ESP_LOGI(TAG, "Initialize speaker via BSP");
    ESP_ERROR_CHECK(bsp_i2c_init());
    m_impl->play_dev_handle = bsp_audio_codec_speaker_init();
    ESP_TRUE_CHECK(m_impl->play_dev_handle);
    ESP_ERROR_CHECK(esp_codec_dev_close(m_impl->play_dev_handle));
    ESP_ERROR_CHECK(esp_codec_dev_open(m_impl->play_dev_handle, &DEFAULT_CODEC_DEV_SAMPLE_INFO));
}

AudioOutput::~AudioOutput()
{
}

bool AudioOutput::play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                             const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                             uint32_t bits_per_sample, size_t channels)
{
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = (uint8_t)bits_per_sample,
        .channel = (uint8_t)channels,
        .channel_mask = 0,
        .sample_rate = sample_rate,
        .mclk_multiple = 0,
    };
    esp_err_t ret = esp_codec_dev_close(m_impl->play_dev_handle);
    ret |= esp_codec_dev_open(m_impl->play_dev_handle, &fs);
    ret |= esp_codec_dev_set_out_mute(m_impl->play_dev_handle, true);
    ret |= esp_codec_dev_set_out_mute(m_impl->play_dev_handle, false);
    ret |= esp_codec_dev_set_out_vol(m_impl->play_dev_handle, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    const size_t len = (buffer_end - buffer_begin) & 0xfffffffc;
    ret |= esp_codec_dev_write(m_impl->play_dev_handle, const_cast<uint8_t *>(&*buffer_begin), len);
    vTaskDelay(pdMS_TO_TICKS(20));

    return true;
}
