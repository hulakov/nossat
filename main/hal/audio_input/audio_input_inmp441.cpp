#include "audio_input.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "driver/i2s_std.h"

const int AudioInput::MICROPHONE_CHANNEL_COUNT = 2;
const int AudioInput::REFERENCE_CHANNEL_COUNT = 1;

struct AudioInput::Impl
{
    i2s_chan_handle_t rx_handle = nullptr;
    std::vector<int32_t> temp_buffer;
};

i2s_chan_handle_t bsp_i2s_microphone_init()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    i2s_chan_handle_t rx_handle;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = GPIO_I2S_MCLK,
                .bclk = GPIO_I2S_SCLK,
                .ws = GPIO_I2S_LRCK,
                .dout = GPIO_I2S_DOUT,
                .din = GPIO_I2S_SDIN,
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

bool AudioInput::capture_audio(std::vector<int16_t> &buffer, size_t chunk_size)
{
    const constexpr int TOTAL_CHANNEL_COUNT = MICROPHONE_CHANNEL_COUNT + REFERENCE_CHANNEL_COUNT;
    assert(TOTAL_CHANNEL_COUNT == 3);

    const size_t buffer_size = chunk_size * MICROPHONE_CHANNEL_COUNT;
    if (m_impl->temp_buffer.size() != buffer_size)
    {
        m_impl->temp_buffer.resize(buffer_size);
    }

    size_t bytes_read;
    esp_err_t ret = i2s_channel_read(m_impl->rx_handle, &m_impl->temp_buffer[0], buffer_size * sizeof(int32_t),
                                     &bytes_read, portMAX_DELAY);
    if (ret != ESP_OK)
    {
        return false;
    }

    for (int i = 0; i < chunk_size; i++)
    {
        // 32:8 are valid bits, 8:0 are the lower 8 bits, all are 0. The input
        // of AFE is 16-bit voice data, and 29:13 bits are used to amplify the
        // voice signal.
        // https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf
        buffer[i * 3 + 2] = 0;
        buffer[i * 3 + 1] = m_impl->temp_buffer[i * 2 + 1] >> 14;
        buffer[i * 3] = m_impl->temp_buffer[i * 2] >> 14;
    }
    return true;
}
