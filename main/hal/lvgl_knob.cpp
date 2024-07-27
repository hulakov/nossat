#include "lvgl_knob.h"

#include "esp_log.h"
#include "esp_lvgl_port_knob.h"

static const char *TAG = "lvgl_knob";

static lv_indev_t *lvgl_port_encoder_init(lv_display_t *display, gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num,
                                          gpio_num_t button_gpio_num)
{
    const knob_config_t knob_cfg = {
        .default_direction = 0,
        .gpio_encoder_a = static_cast<uint8_t>(s2_gpio_num),
        .gpio_encoder_b = static_cast<uint8_t>(s1_gpio_num),
        .enable_power_save = true,
    };
    const button_config_t button_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 1000,
        .short_press_time = 200,
        .gpio_button_config =
            {
                .gpio_num = button_gpio_num,
                .active_level = 0,
            },
    };
    const lvgl_port_encoder_cfg_t cfg = {
        .disp = display,
        .encoder_a_b = &knob_cfg,
        .encoder_enter = &button_cfg,
    };

    return lvgl_port_add_encoder(&cfg);
}

LvglKnob::LvglKnob(std::shared_ptr<Display> display, gpio_num_t s1_gpio_num, gpio_num_t s2_gpio_num,
                   gpio_num_t button_gpio_num)
{
    ESP_LOGI(TAG, "Initialize LVGL knob");
    m_knob_indev = lvgl_port_encoder_init(display->get_lv_display(), s1_gpio_num, s2_gpio_num, button_gpio_num);

    m_group = lv_group_create();
    lv_group_set_default(m_group);
    lv_group_set_wrap(m_group, false);
    lv_indev_set_group(m_knob_indev, m_group);
}

void LvglKnob::set_page(lv_obj_t *page)
{
    lv_group_remove_all_objs(m_group);

    auto walk_cb = [](lv_obj_t *obj, void *arg)
    {
        auto group = reinterpret_cast<lv_group_t *>(arg);

        if (obj->class_p == &lv_button_class || obj->class_p == &lv_switch_class)
        {
            lv_group_add_obj(group, obj);
            if (lv_group_get_focused(group) == 0)
                lv_group_focus_obj(obj);
            return LV_OBJ_TREE_WALK_SKIP_CHILDREN;
        }

        return LV_OBJ_TREE_WALK_NEXT;
    };

    lv_obj_tree_walk(page, walk_cb, m_group);
}