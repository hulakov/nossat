#include "audio_output.h"

#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "bsp/esp-bsp.h"

struct AudioOutput::Impl
{
};

AudioOutput::AudioOutput() : m_impl(std::make_unique<Impl>())
{
}

AudioOutput::~AudioOutput()
{
}

bool AudioOutput::play(const AudioData &audio)
{
    const i2s_slot_mode_t slot_mode = static_cast<i2s_slot_mode_t>(audio.get_num_channels());
    const i2s_data_bit_width_t data_bit_width = static_cast<i2s_data_bit_width_t>(audio.get_bits_per_sample());

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_AUDIO, I2S_ROLE_MASTER);
    i2s_chan_handle_t tx_handle = 0;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    const i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(audio.get_sample_rate()),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(data_bit_width, slot_mode),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = GPIO_SPEAKER_I2S_SCLK,
                .ws = GPIO_SPEAKER_I2S_LRCK,
                .dout = GPIO_SPEAKER_I2S_DOUT,
                .din = I2S_GPIO_UNUSED,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));

    size_t bytes_written = 0;

    // avoid clicks and pops before playing
    int8_t empty[1];
    ESP_ERROR_CHECK(i2s_channel_preload_data(tx_handle, empty, 0, &bytes_written));

    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_write(tx_handle, audio.get_data(), audio.get_size(), &bytes_written, portMAX_DELAY));
    ESP_ERROR_CHECK(i2s_channel_disable(tx_handle));
    ESP_ERROR_CHECK(i2s_del_channel(tx_handle));

    return true;
}
