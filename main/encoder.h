#pragma once

#include "system/interrupt_manager.h"


#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#include <memory>
#include <functional>

using ValueChangedHandler = std::function<void(int previous_value, int new_value)>;
using ClickHandler = std::function<void()>;

struct new_encoder_pcnt_channel_config_t;

class Encoder : public std::enable_shared_from_this<Encoder>
{
public:
    Encoder(gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num, gpio_num_t button_gpio_num);
    void initialize(std::shared_ptr<InterruptManager> interrupt_manager);

    void set_step_value(int value) { m_step_value = value; }
    void set_value_changed_handler(ValueChangedHandler handler) { m_on_value_changed = handler; }
    void set_click_handler(ClickHandler handler) { m_on_click = handler; }

private:
    static void gpio_isr_handler(void *user_ctx);
    void new_isr_handler(gpio_num_t gpio_num);
    void on_value_changed();
    void on_click(InterruptManager::State state);

    void new_encoder_channel(gpio_num_t edge_gpio_num, gpio_num_t level_gpio_num, bool increase);

private:
    std::shared_ptr<EventLoop> m_event_loop;
    const gpio_num_t m_s1_gpio_num;
    const gpio_num_t m_s2_gpio_num;
    const gpio_num_t m_button_gpio_num;

    ValueChangedHandler m_on_value_changed;
    ClickHandler m_on_click;

    pcnt_unit_handle_t m_pcnt_unit = nullptr;
    int m_last_value = 0;
    int m_step_value = 1;
};