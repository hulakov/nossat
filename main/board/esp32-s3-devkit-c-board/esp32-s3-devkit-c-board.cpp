#include "board/board.h"

#include "driver/gpio.h"
#include "driver/i2s_common.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "led_strip.h"
#include "sanity_checks.h"

const constexpr char *TAG = "board";

const int Board::MICROPHONE_CHANNEL_COUNT = 1;
const int Board::REFERENCE_CHANNEL_COUNT = 1;

constexpr const gpio_num_t GPIO_I2S_LRCK = GPIO_NUM_11;
constexpr const gpio_num_t GPIO_I2S_MCLK = GPIO_NUM_NC;
constexpr const gpio_num_t GPIO_I2S_SCLK = GPIO_NUM_12;
constexpr const gpio_num_t GPIO_I2S_SDIN = GPIO_NUM_10;
constexpr const gpio_num_t GPIO_I2S_DOUT = GPIO_NUM_NC;

constexpr const gpio_num_t GPIO_BLINK = GPIO_NUM_48;
struct Board::Data
{
    i2s_chan_handle_t rx_handle = nullptr;

    led_strip_handle_t led_strip;

    void configure_led();
    void enable_led(uint32_t red, uint32_t green, uint32_t blue);
    void disable_led();
};

Board::Board() : m_data(new Data)
{
}

Board::~Board()
{
}

static esp_err_t bsp_spiffs_mount()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_BSP_SPIFFS_MOUNT_POINT,
        .partition_label = CONFIG_BSP_SPIFFS_PARTITION_LABEL,
        .max_files = CONFIG_BSP_SPIFFS_MAX_FILES,
        .format_if_mount_failed = false,
    };

    esp_err_t ret_val = esp_vfs_spiffs_register(&conf);

    ENSURE_NO_ESP_ERRORS(ret_val);

    size_t total = 0, used = 0;
    ret_val = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret_val != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret_val));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ret_val;
}

void Board::Data::configure_led()
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_BLINK,             // The GPIO that connected to the LED strip's data line
        .max_leds = 1,                            // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags = {.invert_out = false},           // whether to invert the output signal (useful
                                                  // when your hardware has a level inverter)
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to
                                           // different power consumption
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 0,
        .flags = {.with_dma = false}, // whether to enable the DMA feature
    };

    ENSURE_NO_ESP_ERRORS(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

void Board::Data::enable_led(uint32_t red, uint32_t green, uint32_t blue)
{
    ENSURE_NO_ESP_ERRORS(led_strip_set_pixel(led_strip, 0, red, green, blue));
    ENSURE_NO_ESP_ERRORS(led_strip_refresh(led_strip));
}

void Board::Data::disable_led()
{
    ENSURE_NO_ESP_ERRORS(led_strip_clear(led_strip));
}

Board &Board::instance()
{
    static Board board;
    return board;
}

void Board::initialize()
{
    ESP_LOGI(TAG, "Initialize SPIFFS");
    ENSURE_NO_ESP_ERRORS(bsp_spiffs_mount());

    ESP_LOGI(TAG, "Initialize I2S");

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    ENSURE_NO_ESP_ERRORS(i2s_new_channel(&chan_cfg, nullptr, &m_data->rx_handle));
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
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

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    ENSURE_NO_ESP_ERRORS(i2s_channel_init_std_mode(m_data->rx_handle, &std_cfg));
    ENSURE_NO_ESP_ERRORS(i2s_channel_enable(m_data->rx_handle));

    ESP_LOGI(TAG, "Initialize LED");
    m_data->configure_led();
}

bool Board::capture_audio(std::vector<int16_t> &buffer, size_t chunk_size)
{
    const constexpr int TOTAL_CHANNEL_COUNT = MICROPHONE_CHANNEL_COUNT + REFERENCE_CHANNEL_COUNT;
    assert(TOTAL_CHANNEL_COUNT == 2);
    assert(buffer.size() == chunk_size * TOTAL_CHANNEL_COUNT);

    size_t bytes_read;
    esp_err_t ret =
        i2s_channel_read(m_data->rx_handle, &buffer[0], buffer.size() * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    if (ret != ESP_OK)
    {
        return false;
    }

    int32_t *tmp_buff = (int32_t *)&buffer[0];
    for (int i = 0; i < chunk_size; i++)
    {
        // 32:8 are valid bits, 8:0 are the lower 8 bits, all are 0. The input of AFE is 16-bit voice data, and 29:13
        // bits are used to amplify the voice signal.
        tmp_buff[i] = tmp_buff[i] >> 14;
    }

    return true;
}

bool Board::play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                       const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                       uint32_t bits_per_sample, size_t channels)
{
    return true;
}

bool Board::show_message(MessageType type, const char *message)
{
    switch (type)
    {
    case MessageType::HELLO:
        m_data->enable_led(50, 50, 50);
        break;
    case MessageType::SAY_COMMAND:
        m_data->enable_led(0, 0, 100);
        break;
    case MessageType::COMMAND_ACCEPTED:
        m_data->enable_led(0, 100, 0);
        break;
    case MessageType::TIMEOUT:
        m_data->enable_led(100, 0, 0);
        break;
    default:
        m_data->enable_led(255, 255, 255);
        return false;
    }

    return true;
}

bool Board::hide_message()
{
    m_data->disable_led();
    return true;
}