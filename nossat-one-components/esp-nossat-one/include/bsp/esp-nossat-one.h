#pragma once

#include "esp_lcd_types.h"
#include "esp_err.h"
#include "esp_lvgl_port.h"
#include "led_strip.h"

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

/**
 * @brief Mount SPIFFS to virtual file system
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if esp_vfs_spiffs_register was already called
 *      - ESP_ERR_NO_MEM if memory can not be allocated
 *      - ESP_FAIL if partition can not be mounted
 *      - other error codes
 */
esp_err_t bsp_spiffs_mount(void);

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

led_strip_handle_t bsp_led_strip_init();
#ifdef __cplusplus
}
#endif
