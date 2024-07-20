#include "audio_output.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "nossat_err.h"

static const char *TAG = "audio_output";

static const AudioFormat SPEAKER_AUDIO_FORMAT = {
    .num_channels = 2,
    .bits_per_sample = 16,
    .sample_rate = 16000,
};

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
    esp_codec_dev_sample_info_t config = make_codec_config(SPEAKER_AUDIO_FORMAT);
    ESP_ERROR_CHECK(esp_codec_dev_open(m_impl->play_dev_handle, &config));
}

AudioOutput::~AudioOutput()
{
}

bool AudioOutput::play(const AudioData &audio)
{

    esp_err_t ret = esp_codec_dev_close(m_impl->play_dev_handle);
    esp_codec_dev_sample_info_t config = make_codec_config(SPEAKER_AUDIO_FORMAT);
    ret |= esp_codec_dev_open(m_impl->play_dev_handle, &config);
    ret |= esp_codec_dev_set_out_mute(m_impl->play_dev_handle, true);
    ret |= esp_codec_dev_set_out_mute(m_impl->play_dev_handle, false);
    ret |= esp_codec_dev_set_out_vol(m_impl->play_dev_handle, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    ret |= esp_codec_dev_write(m_impl->play_dev_handle, const_cast<int8_t *>(audio.get_data()), audio.get_size());
    vTaskDelay(pdMS_TO_TICKS(20));

    return true;
}
