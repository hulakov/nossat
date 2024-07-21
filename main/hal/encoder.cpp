#include "encoder.h"

#include "esp_log.h"

static const char *TAG = "encoder";

Encoder::Encoder(gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num, gpio_num_t button_gpio_num)
    : m_s1_gpio_num(s1_gpio_num), m_s2_gpio_num(s2_gpio_num), m_button_gpio_num(button_gpio_num)
{
}

void Encoder::initialize(std::shared_ptr<InterruptManager> isr_manager)
{
    ESP_LOGI(TAG, "install pcnt unit");
    pcnt_unit_config_t unit_config = {
        .low_limit = SHRT_MIN,
        .high_limit = SHRT_MAX,
        .intr_priority = 0,
        .flags =
            {
                .accum_count = true,
            },
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &m_pcnt_unit));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(m_pcnt_unit, &filter_config));

    new_encoder_channel(m_s1_gpio_num, m_s2_gpio_num, false);
    new_encoder_channel(m_s2_gpio_num, m_s1_gpio_num, true);
    const auto on_value_changed_handler = std::bind(&Encoder::on_value_changed, shared_from_this());
    const auto on_click_handler = std::bind(&Encoder::on_click, shared_from_this(), std::placeholders::_1);
    ESP_ERROR_CHECK(isr_manager->add_interrupt_handler(m_s1_gpio_num, on_value_changed_handler, GPIO_INTR_NEGEDGE));
    ESP_ERROR_CHECK(isr_manager->add_interrupt_handler(m_s2_gpio_num, on_value_changed_handler, GPIO_INTR_NEGEDGE));
    ESP_ERROR_CHECK(isr_manager->add_interrupt_handler(m_button_gpio_num, on_click_handler, GPIO_INTR_ANYEDGE));

    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(m_pcnt_unit));
    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(m_pcnt_unit));
    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(m_pcnt_unit));
}

void Encoder::set_value(int value)
{
    int last_value = m_value;
    m_value = value;
    if (m_on_value_changed)
        m_on_value_changed(last_value, value);
}

void Encoder::new_encoder_channel(gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num, bool increase)
{
    ESP_LOGI(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_config = {
        .edge_gpio_num = s1_gpio_num,
        .level_gpio_num = s2_gpio_num,
        .flags = {},
    };
    pcnt_channel_handle_t pcnt_chan = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(m_pcnt_unit, &chan_config, &pcnt_chan));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    if (increase)
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                                     PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    else
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                     PCNT_CHANNEL_EDGE_ACTION_DECREASE));

    ESP_ERROR_CHECK(
        pcnt_channel_set_level_action(pcnt_chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
}

void Encoder::on_value_changed()
{
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(m_pcnt_unit, &count));

    int value = count / m_step_value;
    if (m_value == value)
        return;

    int last_value = m_value;
    m_value = value;

    if (m_on_value_changed)
        m_on_value_changed(last_value, value);
}

void Encoder::on_click(InterruptManager::State state)
{
    // const auto level = gpio_get_level(gpio_info->gpio_num);
    // InterruptManager::State state = level;
    if (state == InterruptManager::State::ON)
    {
        // ESP_LOGI(TAG, "ON");
        m_button_state_begin = std::chrono::steady_clock::now();
    }
    else
    {
        // ESP_LOGI(TAG, "OFF");
    }

    if (m_button_state == InterruptManager::State::ON && state == InterruptManager::State::OFF)
    {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_button_state_begin).count();

        if (duration > 50)
        {
            ESP_LOGI(TAG, "click - %d ms", duration);
            m_on_click();
        }
    }
    m_button_state = state;
}
