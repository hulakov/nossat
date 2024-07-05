#include "led_blinker.h"
#include "bsp/esp-bsp.h"

#include <cstdint>

void color_wheel(int8_t hue, int8_t &red, int8_t &green, int8_t &blue)
{
    if (hue > 80)
    {
        int8_t shift = (hue - 80) * 3;
        red = 120 + shift * 2;
        green = 0;
        blue = 255 - shift;
    }
    else if (hue > 40)
    {
        int8_t shift = (hue - 40) * 3;
        red = 0;
        green = 255 - shift;
        blue = 120 + shift * 2;
    }
    else
    {
        int8_t shift = hue * 3;
        red = (255 - shift);
        green = 120 + shift * 2;
        blue = 0;
    }
}

void run_cylon_bounce(led_strip_handle_t led_strip)
{
    constexpr const double MIN = 1.5;
    constexpr const double MAX = 6.5;
    constexpr const double DELTA = 0.1;
    constexpr const double DELAY_MS = 10;

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

        hue = (hue + 1) % (255);
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

        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    }
}