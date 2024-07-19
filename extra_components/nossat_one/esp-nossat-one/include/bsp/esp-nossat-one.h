#pragma once

#include "esp_lcd_types.h"
#include "esp_err.h"
#include "esp_lvgl_port.h"
#include "led_strip.h"
#include "driver/i2s_common.h"

/* Display */
#define BSP_LCD_DATA0 (GPIO_NUM_7)
#define BSP_LCD_PCLK (GPIO_NUM_15)
#define BSP_LCD_CS (GPIO_NUM_4)
#define BSP_LCD_DC (GPIO_NUM_5)
#define BSP_LCD_RST (GPIO_NUM_6)
#define BSP_LCD_BACKLIGHT (GPIO_NUM_16)

/* LED */
#define GPIO_BLINK GPIO_NUM_48
#define GPIO_LED_STRIP GPIO_NUM_1
#define NUM_LEDS 8

/* I2S */
#define I2S_AUDIO I2S_NUM_1

/* I2S microphone */
#define GPIO_MICROPHONE_I2S_LRCK GPIO_NUM_11 // WS
#define GPIO_MICROPHONE_I2S_SCLK GPIO_NUM_12 // SCK
#define GPIO_MICROPHONE_I2S_SDIN GPIO_NUM_10 // SD

/* I2S speaker */
#define GPIO_SPEAKER_I2S_LRCK GPIO_NUM_2  // WS
#define GPIO_SPEAKER_I2S_SCLK GPIO_NUM_38 // SCK
#define GPIO_SPEAKER_I2S_DOUT GPIO_NUM_21

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct
{
    lvgl_port_cfg_t lvgl_port_cfg;
} bsp_display_cfg_t;

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct
{
    int max_transfer_sz; /*!< Maximum transfer size, in bytes. */
} bsp_display_config_t;

/**************************************************************************************************
 *
 * LCD interface
 *
 * Nossat One is shipped with 1.77inch ST7735 display controller.
 * https://www.aliexpress.com/item/1005005621773758.html?spm=a2g0o.order_list.order_list_main.15.4eeb5e5b6OoJjW
 * It features 16-bit colors, 128x160 resolution.
 *
 * LVGL is used as graphics library. LVGL is NOT thread safe, therefore the user must take LVGL mutex
 * by calling bsp_display_lock() before calling and LVGL API (lv_...) and then give the mutex with
 * bsp_display_unlock().
 *
 * Display's backlight must be enabled explicitly by calling bsp_display_backlight_on()
 **************************************************************************************************/

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling bsp_display_brightness_set()
 *
 * @param cfg display configuration
 *
 * @return Pointer to LVGL display or NULL when error occured
 */
lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Take LVGL mutex
 *
 * @param timeout_ms Timeout in [ms]. 0 will block indefinitely.
 * @return true  Mutex was taken
 * @return false Mutex was NOT taken
 */
bool bsp_display_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 *
 */
void bsp_display_unlock(void);

led_strip_handle_t bsp_led_strip_init();

#ifdef __cplusplus
}
#endif
