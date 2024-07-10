#include "board/board.h"
#include "gui/lvgl_ui.h"
#include "sanity_checks.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
extern "C"
{
#include "bsp/esp-bsp.h"
}
#pragma GCC diagnostic pop

const constexpr char *TAG = "board";

const int Board::MICROPHONE_CHANNEL_COUNT = 2;
const int Board::REFERENCE_CHANNEL_COUNT = 1;

esp_codec_dev_sample_info_t DEFAULT_CODEC_DEV_SAMPLE_INFO = {
    .bits_per_sample = 16,
    .channel = 2,
    .channel_mask = 0,
    .sample_rate = 16000,
};
const constexpr float CODEC_DEFAULT_ADC_VOLUME = 24.0;

struct Board::Data
{
    esp_codec_dev_handle_t play_dev_handle;
    esp_codec_dev_handle_t record_dev_handle;
    button_handle_t button = nullptr;
    int btn_num = 0;
};

Board::Board() : m_data(new Data)
{
}

Board::~Board()
{
}

Board &Board::instance()
{
    static Board board;
    return board;
}

void Board::initialize()
{
    ESP_LOGI(TAG, "Initialize I2C");
    ENSURE_NO_ESP_ERRORS(bsp_i2c_init());

    ESP_LOGI(TAG, "Initialize display");
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
    };
    cfg.lvgl_port_cfg.task_affinity = 1;
    bsp_display_start_with_config(&cfg);

    ENSURE_NO_ESP_ERRORS(bsp_iot_button_create(&m_data->button, &m_data->btn_num, BSP_BUTTON_NUM));

    ESP_LOGI(TAG, "Initialize speaker");
    m_data->play_dev_handle = bsp_audio_codec_speaker_init();
    ENSURE_TRUE(m_data->play_dev_handle);
    ENSURE_NO_ESP_ERRORS(esp_codec_dev_close(m_data->play_dev_handle));
    ENSURE_NO_ESP_ERRORS(esp_codec_dev_open(m_data->play_dev_handle, &DEFAULT_CODEC_DEV_SAMPLE_INFO));

    ESP_LOGI(TAG, "Initialize microphone");
    m_data->record_dev_handle = bsp_audio_codec_microphone_init();
    ENSURE_TRUE(m_data->record_dev_handle);
    ENSURE_NO_ESP_ERRORS(esp_codec_dev_close(m_data->record_dev_handle));
    ENSURE_NO_ESP_ERRORS(esp_codec_dev_set_in_gain(m_data->record_dev_handle, CODEC_DEFAULT_ADC_VOLUME));
    ENSURE_NO_ESP_ERRORS(esp_codec_dev_open(m_data->record_dev_handle, &DEFAULT_CODEC_DEV_SAMPLE_INFO));

    ESP_LOGI(TAG, "Initialize LVGL");
    ui_initialize();

    ENSURE_NO_ESP_ERRORS(bsp_display_backlight_off());

    ESP_LOGI(TAG, "******* Board Initialized *******");
}

bool Board::capture_audio(std::vector<int16_t> &buffer, size_t chunk_size)
{
    const constexpr int TOTAL_CHANNEL_COUNT = MICROPHONE_CHANNEL_COUNT + REFERENCE_CHANNEL_COUNT;
    assert(TOTAL_CHANNEL_COUNT == 3);
    assert(MICROPHONE_CHANNEL_COUNT == 2);
    assert(buffer.size() == chunk_size * TOTAL_CHANNEL_COUNT);

    esp_codec_dev_read(m_data->record_dev_handle, &buffer[0], chunk_size * MICROPHONE_CHANNEL_COUNT * sizeof(int16_t));

    // 2 channels -> 3 channels
    for (int i = chunk_size - 1; i >= 0; i--)
    {
        buffer[i * 3 + 2] = 0;
        buffer[i * 3 + 1] = buffer[i * 2 + 1];
        buffer[i * 3 + 0] = buffer[i * 2 + 0];
    }

    return true;
}

bool Board::play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                       const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                       uint32_t bits_per_sample, size_t channels)
{
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = (uint8_t)bits_per_sample,
        .channel = (uint8_t)channels,
        .channel_mask = 0,
        .sample_rate = sample_rate,
    };
    esp_err_t ret = esp_codec_dev_close(m_data->play_dev_handle);
    ret |= esp_codec_dev_open(m_data->play_dev_handle, &fs);
    ret |= esp_codec_dev_set_out_mute(m_data->play_dev_handle, true);
    ret |= esp_codec_dev_set_out_mute(m_data->play_dev_handle, false);
    ret |= esp_codec_dev_set_out_vol(m_data->play_dev_handle, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    const size_t len = (buffer_end - buffer_begin) & 0xfffffffc;
    ret |= esp_codec_dev_write(m_data->play_dev_handle, const_cast<uint8_t *>(&*buffer_begin), len);
    vTaskDelay(pdMS_TO_TICKS(20));

    return true;
}

bool Board::show_message(MessageType type, const char *message)
{
    ui_show_message(message, type == MessageType::SAY_COMMAND);
    bsp_display_backlight_on();
    return true;
}

bool Board::hide_message()
{
    bsp_display_backlight_off();
    ui_hide_message();
    return true;
}