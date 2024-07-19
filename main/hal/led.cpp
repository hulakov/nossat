#include "led.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"

const constexpr char *TAG = "led";

Led::Led()
{
    ESP_LOGI(TAG, "Initialize LED");
    m_led_strip = bsp_led_strip_init();
}

Led::~Led()
{
}

void Led::solid(uint32_t red, uint32_t green, uint32_t blue)
{
    for (int i = 0; i < NUM_LEDS; i++)
        ESP_ERROR_CHECK(led_strip_set_pixel(m_led_strip, i, red, green, blue));
    ESP_ERROR_CHECK(led_strip_refresh(m_led_strip));
}

void Led::clear()
{
    ESP_ERROR_CHECK(led_strip_clear(m_led_strip));
}

// TaskHandle_t led_task;
// TaskFunction_t run_cylon_bounce_adapter = [](void *led_strip_ptr)
// {
//     run_cylon_bounce(static_cast<led_strip_handle_t>(led_strip_ptr));
//     vTaskDelete(NULL);
// };
// xTaskCreatePinnedToCore(run_cylon_bounce_adapter, "LED Blinker", 2 * 1024, m_data->led_strip, 1, &led_task, 1);