#pragma once

#include "driver/gpio.h"

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

/* Left Knob */
#define GPIO_LEFT_KNOB_S1 GPIO_NUM_14
#define GPIO_LEFT_KNOB_S2 GPIO_NUM_13
#define GPIO_LEFT_KNOB_KEY GPIO_NUM_9

/* Right Knob */
#define GPIO_RIGHT_KNOB_S1 GPIO_NUM_17
#define GPIO_RIGHT_KNOB_S2 GPIO_NUM_18
#define GPIO_RIGHT_KNOB_KEY GPIO_NUM_8
