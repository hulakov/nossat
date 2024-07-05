#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lvgl_port.h"

/**************************************************************************************************
 *  ESP-BOX-Lite pinout
 **************************************************************************************************/
/* Display */
#define BSP_LCD_DATA0 (GPIO_NUM_7)
#define BSP_LCD_PCLK (GPIO_NUM_15)
#define BSP_LCD_CS (GPIO_NUM_4)
#define BSP_LCD_DC (GPIO_NUM_5)
#define BSP_LCD_RST (GPIO_NUM_6)
#define BSP_LCD_BACKLIGHT (GPIO_NUM_16)

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief BSP display configuration
 * structure
 *
 */
typedef struct
{
    lvgl_port_cfg_t lvgl_port_cfg;
} bsp_display_cfg_t;

/**
 * @brief BSP display configuration
 * structure
 *
 */
typedef struct
{
    int max_transfer_sz; /*!< Maximum transfer size, in bytes.
                          */
} bsp_display_config_t;

/**
 * @brief Mount SPIFFS to virtual file
 * system
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if
 * esp_vfs_spiffs_register was already
 * called
 *      - ESP_ERR_NO_MEM if memory can
 * not be allocated
 *      - ESP_FAIL if partition can not
 * be mounted
 *      - other error codes
 */
esp_err_t bsp_spiffs_mount(void);

/**************************************************************************************************
 *
 * LCD interface
 *
 * ESP-BOX-Lite is shipped with 2.4inch
 *ST7789 display controller. It features
 *16-bit colors, 320x240 resolution.
 *
 * LVGL is used as graphics library.
 *LVGL is NOT thread safe, therefore the
 *user must take LVGL mutex by calling
 *bsp_display_lock() before calling and
 *LVGL API (lv_...) and then give the
 *mutex with bsp_display_unlock().
 *
 * Display's backlight must be enabled
 *explicitly by calling
 *bsp_display_backlight_on()
 **************************************************************************************************/
#define BSP_LCD_H_RES (160)
#define BSP_LCD_V_RES (128)
#define BSP_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define BSP_LCD_SPI_NUM (SPI2_HOST)

/**
 * @brief Initialize display
 *
 * This function initializes SPI,
 * display controller and starts LVGL
 * handling task. LCD backlight must be
 * enabled separately by calling
 * bsp_display_brightness_set()
 *
 * @param cfg display configuration
 *
 * @return Pointer to LVGL display or
 * NULL when error occured
 */
lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Set display's brightness
 *
 * Brightness is controlled with PWM
 * signal to a pin controling backlight.
 *
 * @param[in] brightness_percent
 * Brightness in [%]
 * @return
 *      - ESP_OK                On
 * success
 *      - ESP_ERR_INVALID_ARG Parameter
 * error
 */
esp_err_t bsp_display_brightness_set(int brightness_percent);
#ifdef __cplusplus
}
#endif
