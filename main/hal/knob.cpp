#include "knob.h"

#include "esp_log.h"

static const char *TAG = "encoder";

struct HandlerContext
{
    HandlerContext(std::shared_ptr<EventLoop> e, std::function<void()> h) : event_loop(e), handler(h) {}
    std::shared_ptr<EventLoop> event_loop;
    std::function<void()> handler;
};

static button_handle_t button_init(gpio_num_t gpio_num)
{
    const button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 1000,
        .short_press_time = 200,
        .gpio_button_config =
            {
                .gpio_num = gpio_num,
                .active_level = 0,
            },
    };
    return iot_button_create(&cfg);
}

static knob_handle_t knob_init(gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num)
{
    const knob_config_t cfg = {
        .default_direction = 0,
        .gpio_encoder_a = static_cast<uint8_t>(s2_gpio_num),
        .gpio_encoder_b = static_cast<uint8_t>(s1_gpio_num),
        .enable_power_save = false,
    };
    return iot_knob_create(&cfg);
}

Knob::Knob(gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num, gpio_num_t button_gpio_num)
    : m_s1_gpio_num(s1_gpio_num), m_s2_gpio_num(s2_gpio_num), m_button_gpio_num(button_gpio_num)
{
}

void Knob::initialize(std::shared_ptr<EventLoop> event_loop)
{
    m_event_loop = event_loop;

    ESP_LOGI(TAG, "Initialize knob button");
    m_button = button_init(m_button_gpio_num);
    m_knob = knob_init(m_s1_gpio_num, m_s2_gpio_num);

    const auto handler_adapter = [](void *arg, void *data)
    {
        auto context = static_cast<HandlerContext *>(data);
        context->event_loop->post([context] { context->handler(); });
    };

    const auto click_context = new HandlerContext(m_event_loop, std::bind(&Knob::on_click, shared_from_this()));
    iot_button_register_cb(m_button, BUTTON_SINGLE_CLICK, handler_adapter, click_context);
    const auto left_context = new HandlerContext(m_event_loop, std::bind(&Knob::on_left, shared_from_this()));
    iot_knob_register_cb(m_knob, KNOB_LEFT, handler_adapter, left_context);
    const auto right_context = new HandlerContext(m_event_loop, std::bind(&Knob::on_right, shared_from_this()));
    iot_knob_register_cb(m_knob, KNOB_RIGHT, handler_adapter, right_context);
}

void Knob::set_value(int value)
{
    m_value_offset = value;
    iot_knob_clear_count_value(m_knob);
    if (m_on_value_changed != nullptr)
        m_on_value_changed(get_value());
}

int Knob::get_value() const
{
    return m_value_offset + iot_knob_get_count_value(m_knob) / m_step_value;
}

void Knob::on_left()
{
    if (m_on_value_changed != nullptr)
        m_on_value_changed(get_value());
    if (m_on_left != nullptr)
        m_on_left();
}

void Knob::on_right()
{
    if (m_on_value_changed != nullptr)
        m_on_value_changed(get_value());
    if (m_on_right != nullptr)
        m_on_right();
}

void Knob::on_click()
{
    m_on_click();
}