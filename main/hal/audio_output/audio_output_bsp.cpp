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

bool AudioOutput::play(const AudioData &audio)
{
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = audio.bits_per_sample,
        .channel = audio.num_channels,
        .channel_mask = 0,
        .sample_rate = audio.sample_rate,
        .mclk_multiple = 0,
    };
    esp_err_t ret = esp_codec_dev_close(m_impl->play_dev_handle);
    ret |= esp_codec_dev_open(m_impl->play_dev_handle, &fs);
    ret |= esp_codec_dev_set_out_mute(m_impl->play_dev_handle, true);
    ret |= esp_codec_dev_set_out_mute(m_impl->play_dev_handle, false);
    ret |= esp_codec_dev_set_out_vol(m_impl->play_dev_handle, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    ret |= esp_codec_dev_write(m_impl->play_dev_handle, audio.data.data(), audio.data.size());
    vTaskDelay(pdMS_TO_TICKS(20));

    return true;
}
