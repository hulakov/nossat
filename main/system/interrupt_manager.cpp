#include "interrupt_manager.h"

#include "esp_log.h"

static const char *TAG = "interrupt_manager";

InterruptManager::InterruptManager(std::shared_ptr<EventLoop> event_loop) : m_event_loop(event_loop)
{
}

void InterruptManager::initialize()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_install_isr_service(0));
}

esp_err_t InterruptManager::gpio_configure(gpio_num_t gpio_num, gpio_int_type_t type)
{
    ESP_LOGI(TAG, "add interrupt handler for %d (%d)", static_cast<int>(gpio_num), static_cast<int>(type));
    gpio_config_t io_conf;
    io_conf.intr_type = type;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    return gpio_config(&io_conf);
}

void InterruptManager::on_state_changed(gpio_num_t gpio_num, State state)
{
    auto &gpio_info = m_gpio_infos[gpio_num];
    if (gpio_info.state == state)
        return;
    gpio_info.state = state;

    if (gpio_info.handler != nullptr)
        gpio_info.handler(state);
}

esp_err_t InterruptManager::add_interrupt_handler(gpio_num_t gpio_num, Handler handler, gpio_int_type_t type)
{
    if (m_gpio_infos[gpio_num].manager != nullptr)
    {
        ESP_LOGE(TAG, "gpio %d is already configured", static_cast<int>(gpio_num));
        return ESP_FAIL;
    }

    auto err = gpio_configure(gpio_num, type);
    if (err != ESP_OK)
        return err;

    auto &gpio_info = m_gpio_infos[gpio_num];
    gpio_info = {
        .gpio_num = gpio_num,
        .manager = shared_from_this(),
        .state = State::OFF,
        .handler = handler,
    };

    const auto gpio_isr_handler = [](void *user_ctx)
    {
        auto gpio_info = reinterpret_cast<GpioInfo *>(user_ctx);
        const auto level = gpio_get_level(gpio_info->gpio_num);
        const auto state = level == 0 ? State::ON : State::OFF;
        auto proc = std::bind(&InterruptManager::on_state_changed, gpio_info->manager, gpio_info->gpio_num, state);
        gpio_info->manager->m_event_loop->post_from_isr(proc);
    };

    err = gpio_isr_handler_add(gpio_num, gpio_isr_handler, &gpio_info);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}