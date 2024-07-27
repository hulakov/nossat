#include "gui_one.h"
#include "nossat-one/src/ui/ui.h"
#include <mutex>
#include "bsp/esp-bsp.h"
#include "esp_log.h"

static const char *TAG = "gui";

Gui::Gui(std::shared_ptr<Display> display, std::shared_ptr<EventLoop> event_loop) : m_display(display)
{
    ESP_LOGI(TAG, "******* Initialize GUI *******");

    ESP_LOGI(TAG, "Create LVGL screens");
    create_screens();

    ESP_LOGI(TAG, "Initialize controls");
    initialize_knobs(event_loop);
}

void Gui::initialize_knobs(std::shared_ptr<EventLoop> event_loop)
{
    ESP_LOGI(TAG, "Create left knob");
    m_left_encoder = std::make_shared<Knob>(event_loop, GPIO_LEFT_KNOB_S1, GPIO_LEFT_KNOB_S2, GPIO_LEFT_KNOB_KEY);
    m_left_encoder->set_step_value(2);
    m_left_encoder->set_value_changed_handler([](int value) { ESP_LOGI(TAG, "[left] changed %d", value); });
    m_left_encoder->set_left_handler(std::bind(&Gui::switch_to_screen, this, false));
    m_left_encoder->set_right_handler(std::bind(&Gui::switch_to_screen, this, true));

    ESP_LOGI(TAG, "Create right knob");
    m_right_encoder =
        std::make_shared<LvglKnob>(m_display, GPIO_RIGHT_KNOB_S1, GPIO_RIGHT_KNOB_S2, GPIO_RIGHT_KNOB_KEY);
    m_right_encoder->set_page(objects.main);
}

void Gui::show_message(const char *message, bool animation)
{
    std::unique_lock<Display> lock(*m_display);
    lv_label_set_text_static(objects.message_label, message);
    lv_scr_load_anim(objects.message_box, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
}

void Gui::show_current_page()
{
    std::unique_lock<Display> lock(*m_display);
    lv_obj_t *page = get_page(m_current_page_index);
    lv_scr_load_anim(page, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    m_right_encoder->set_page(page);
}

lv_obj_t *Gui::get_page(int page_index)
{
    switch (page_index)
    {
    case 0:
        return objects.main;
    case 1:
        return objects.settings;
    default:
        assert(!"bad page index");
    }
}

void Gui::switch_to_screen(bool right)
{
    ESP_LOGI(TAG, "Switch to page to %s", right ? "right" : "left");

    std::unique_lock<Display> lock(*m_display);

    lv_screen_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE;
    if (right)
    {
        m_current_page_index = (m_current_page_index + 1) % PAGE_COUNT;
        anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    }
    else
    {
        m_current_page_index = (m_current_page_index - 1 + PAGE_COUNT) % PAGE_COUNT;
        anim = LV_SCR_LOAD_ANIM_MOVE_LEFT;
    }

    lv_obj_t *page = get_page(m_current_page_index);
    lv_scr_load_anim(page, anim, 200, 0, false);
    m_right_encoder->set_page(page);
}
