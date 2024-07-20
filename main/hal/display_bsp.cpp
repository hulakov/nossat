#include "display.h"
#include "bsp/esp-bsp.h"

struct Display::Impl
{
};

Display::Display()
{
    const bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = 0,
        .flags =
            {
                .buff_dma = true,
                .buff_spiram = false,
            },
    };
    m_display = bsp_display_start_with_config(&cfg);
}

Display::~Display()
{
}

void Display::enable_backlight(bool enable)
{
    ESP_ERROR_CHECK(bsp_display_brightness_set(enable ? 100 : 0));
}

void Display::lock()
{
    bsp_display_lock(0);
}

void Display::unlock()
{
    bsp_display_unlock();
}