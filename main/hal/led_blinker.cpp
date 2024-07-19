#include "led_blinker.h"
#include "bsp/esp-bsp.h"

#include <cstdint>

constexpr const int8_t HUE_MAX = 255;

void color_wheel(int8_t hue, int8_t &red, int8_t &green, int8_t &blue)
{
    if (hue > 2 * HUE_MAX / 3)
    {
        int8_t shift = (hue - 2 * HUE_MAX / 3) * 3;
        red = shift;
        green = 0;
        blue = HUE_MAX - shift;
    }
    else if (hue > HUE_MAX / 3)
    {
        int8_t shift = (hue - HUE_MAX / 3) * 3;
        red = 0;
        green = HUE_MAX - shift;
        blue = shift;
    }
    else
    {
        int8_t shift = hue * 3;
        red = HUE_MAX - shift;
        green = shift;
        blue = 0;
    }

    // avoid extra bright red
    red /= 5;
}

void run_cylon_bounce(led_strip_handle_t led_strip)
{
    constexpr const double MIN = 1.5;
    constexpr const double MAX = 6.5;
    constexpr const double DELTA = 0.1 * 10;
    constexpr const double DELAY_MS = 6 * 10 * 100;

    double position = MIN;
    int direction = 1;
    int hue = 0;

    while (true)
    {
        position += direction * DELTA;
        if (position >= MAX)
            direction = -1;
        else if (position <= MIN)
            direction = 1;

        hue = (hue + 1) % HUE_MAX;
        int8_t red, green, blue;
        color_wheel(hue, red, green, blue);

        int position_int = static_cast<int>(position);

        for (int i = 0; i < NUM_LEDS; i++)
        {
            if (i == position_int)
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
            else if (i == position_int - 1 || i == position_int + 1)
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red / 2, green / 2, blue / 2));
            else
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 0, 0, 0));
        }
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));

        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}