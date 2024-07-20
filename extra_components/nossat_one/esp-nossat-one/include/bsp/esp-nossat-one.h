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
