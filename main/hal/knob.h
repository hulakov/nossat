#pragma once

#include "system/interrupt_manager.h"

#include "iot_button.h"
#include "iot_knob.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#include <memory>
#include <functional>
#include <chrono>

using ValueChangedHandler = std::function<void(int value)>;
using Handler = std::function<void()>;

struct new_encoder_pcnt_channel_config_t;

class Knob
{
public:
    Knob(std::shared_ptr<EventLoop> event_loop, gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num,
         gpio_num_t button_gpio_num);

    int get_value() const;
    void set_value(int value);

    void set_step_value(int value) { m_step_value = value; }
    void set_value_changed_handler(ValueChangedHandler handler) { m_on_value_changed = handler; }
    void set_click_handler(Handler handler) { m_on_click = handler; }
    void set_left_handler(Handler handler) { m_on_left = handler; }
    void set_right_handler(Handler handler) { m_on_right = handler; }

private:
    void on_click();
    void on_value_changed(bool left);

private:
    std::shared_ptr<EventLoop> m_event_loop;

    ValueChangedHandler m_on_value_changed;
    Handler m_on_click;
    Handler m_on_left;
    Handler m_on_right;

    int m_step_value = 1;
    int m_value_offset = 0;
    int m_last_reported_value = 0;
    button_handle_t m_button = 0;
    knob_handle_t m_knob = 0;
};
