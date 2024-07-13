#include "bsp/esp-nossat-one.h"
#include "bsp_err.h"

led_strip_handle_t bsp_led_strip_init()
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_LED_STRIP,         // The GPIO that connected to the LED strip's data line
        .max_leds = NUM_LEDS,                     // The number of LEDs in the strip,
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

    led_strip_handle_t led_strip;
    BSP_ERROR_CHECK_RETURN_NULL(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    return led_strip;
}
