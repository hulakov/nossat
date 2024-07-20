#pragma once

// #include "lvgl.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include <memory>

class Display
{
public:
    Display();
    ~Display();

    lv_display_t *get_lv_display() const { return m_display; }
    void enable_backlight(bool enable = true);

    void lock();
    void unlock();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    lv_display_t *m_display;
};