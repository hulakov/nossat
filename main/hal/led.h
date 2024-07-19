#pragma once

#include "led_strip.h"

class Led
{
public:
    Led();
    ~Led();

    void solid(uint32_t red, uint32_t green, uint32_t blue);
    void clear();

private:
    led_strip_handle_t m_led_strip;
};
