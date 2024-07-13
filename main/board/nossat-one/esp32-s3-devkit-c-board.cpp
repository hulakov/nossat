#include "board/board.h"

#include "bsp/esp-bsp.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "led_strip.h"
#include "sanity_checks.h"
#include "led_blinker.h"
#include "encoder.h"

#include <functional>

const constexpr char *TAG = "board";

extern "C"
{
void lvgl_ui_main(lv_disp_t *disp);
void set_lvgl_value(int32_t v);
}

auto encoder1 = std::make_shared<Encoder>(GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_9);
auto encoder2 = std::make_shared<Encoder>(GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_8);

struct Board::Data
{

    led_strip_handle_t led_strip;

    void enable_led(uint32_t red, uint32_t green, uint32_t blue);
    void disable_led();
};

Board::Board(std::shared_ptr<InterruptManager> interrupt_manager) : m_data(new Data)
{
    ESP_LOGI(TAG, "Initialize LED");
    m_data->led_strip = bsp_led_strip_init();
    // TaskHandle_t led_task;
    // TaskFunction_t run_cylon_bounce_adapter = [](void *led_strip_ptr)
    // {
    //     run_cylon_bounce(static_cast<led_strip_handle_t>(led_strip_ptr));
    //     vTaskDelete(NULL);
    // };
    // xTaskCreatePinnedToCore(run_cylon_bounce_adapter, "LED Blinker", 2 * 1024, m_data->led_strip, 1, &led_task, 1);

    ESP_LOGI(TAG, "Initialize UI");
    bsp_display_cfg_t cfg = {.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG()};
    lv_disp_t *disp = bsp_display_start_with_config(&cfg);
    lvgl_ui_main(disp);

    encoder1->set_step_value(4);
    encoder1->set_value_changed_handler(
        [](int previous_value, int new_value)
        {
            ESP_LOGI(TAG, "[left] changed %d -> %d", previous_value, new_value);
            set_lvgl_value(new_value);
        });
    encoder1->set_click_handler([]() { ESP_LOGI(TAG, "[left] click"); });
    encoder1->initialize(interrupt_manager);

    encoder2->set_step_value(4);
    encoder2->set_value_changed_handler([](int previous_value, int new_value)
                                        { ESP_LOGI(TAG, "[right] changed %d -> %d", previous_value, new_value); });
    encoder2->set_click_handler([]() { ESP_LOGI(TAG, "[right] click"); });
    encoder2->initialize(interrupt_manager);
}

Board::~Board()
{
}

void Board::Data::enable_led(uint32_t red, uint32_t green, uint32_t blue)
{
    for (int i = 0; i < NUM_LEDS; i++)
        ENSURE_NO_ESP_ERRORS(led_strip_set_pixel(led_strip, i, red, green, blue));
    ENSURE_NO_ESP_ERRORS(led_strip_refresh(led_strip));
}

void Board::Data::disable_led()
{
    ENSURE_NO_ESP_ERRORS(led_strip_clear(led_strip));
}

bool Board::show_message(MessageType type, const char *message)
{
    switch (type)
    {
    case MessageType::HELLO:
        m_data->enable_led(255, 0, 0);
        break;
    case MessageType::SAY_COMMAND:
        m_data->enable_led(0, 0, 255);
        break;
    case MessageType::COMMAND_ACCEPTED:
        m_data->enable_led(0, 255, 0);
        break;
    case MessageType::TIMEOUT:
        m_data->enable_led(255, 0, 0);
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