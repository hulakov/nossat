#include "display.h"
#include "esp_lvgl_port.h"

extern "C"
{
lv_disp_t *bsp_display_start_with_config();
esp_err_t bsp_display_brightness_set(int brightness_percent);
}

struct Display::Impl
{
};

Display::Display()
{
    m_display = bsp_display_start_with_config();
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
    lvgl_port_lock(0);
}

void Display::unlock()
{
    lvgl_port_unlock();
}