#pragma once

#include "driver/gpio.h"
#include "event_loop.h"

#include <functional>
#include <array>

class InterruptManager : public std::enable_shared_from_this<InterruptManager>
{
public:
    enum class State {
        OFF,
        ON,
    };

    InterruptManager(std::shared_ptr<EventLoop> event_loop);
    void initialize();

public:
    using Handler = std::function<void(State state)>;
    esp_err_t add_interrupt_handler(gpio_num_t gpio_num, Handler handler, gpio_int_type_t type);

private:
    esp_err_t gpio_configure(gpio_num_t gpio_num, gpio_int_type_t type);
    void on_state_changed(gpio_num_t gpio_num, State state);

private:
    std::shared_ptr<EventLoop> m_event_loop;

    struct GpioInfo
    {
        gpio_num_t gpio_num;
        std::shared_ptr<InterruptManager> manager;
        State state = State::OFF;
        Handler handler;
    };
    std::array<GpioInfo, GPIO_NUM_MAX> m_gpio_infos;
};