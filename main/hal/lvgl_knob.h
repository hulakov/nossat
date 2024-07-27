#pragma once

#include "hal/display.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "lvgl.h"

#include <memory>
#include <functional>

using ValueChangedHandler = std::function<void(int value)>;
using Handler = std::function<void()>;

struct new_encoder_pcnt_channel_config_t;

class LvglKnob : public std::enable_shared_from_this<LvglKnob>
{
public:
    LvglKnob(std::shared_ptr<Display> display, gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num,
             gpio_num_t button_gpio_num);

    void set_page(lv_obj_t *page);

private:
    lv_indev_t *m_knob_indev;
    lv_group_t *m_group;
};
