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
    initialize_sound_chart();

    m_clock_timer = std::make_shared<LvglTimer>(std::bind(&Gui::update_clock, this), 1000);
    m_sound_chart_timer = std::make_shared<LvglTimer>(std::bind(&Gui::update_sound_chart, this), 200);
}

void Gui::initialize_sound_chart()
{
    m_audio_serie = lv_chart_add_series(objects.sound_chart, lv_color_white(), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(objects.sound_chart, 100);
    lv_chart_set_range(objects.sound_chart, LV_CHART_AXIS_PRIMARY_Y, -1000, 1000);
    lv_chart_set_type(objects.sound_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_div_line_count(objects.sound_chart, 0, 0);
    lv_obj_set_style_size(objects.sound_chart, 0, 0, LV_PART_INDICATOR);
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

void Gui::update_clock()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time);

    char buffer[11];

    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    lv_label_set_text(objects.time_label, buffer);

    std::strftime(buffer, sizeof(buffer), "%d.%m.%Y", &tm);
    lv_label_set_text(objects.date_label, buffer);
}

void Gui::update_sound_chart()
{
    std::vector<int32_t> data;
    {
        std::unique_lock<std::mutex> lock(m_pending_sound_data_mutex);
        std::swap(data, m_pending_sound_data);
    }

    if (data.size() == 0)
        return;

    std::unique_lock<Display> lock(*m_display);
    for (uint32_t value : data)
        lv_chart_set_next_value(objects.sound_chart, m_audio_serie, value);
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

void Gui::show_recording_screen()
{
    std::unique_lock<Display> lock(*m_display);
    lv_chart_set_all_value(objects.sound_chart, m_audio_serie, LV_CHART_POINT_NONE);
    lv_scr_load_anim(objects.sound_recorder, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    m_right_encoder->set_page(objects.sound_recorder);
}

void Gui::add_recording_data(const std::vector<int32_t> &values)
{
    ESP_LOGI(TAG, "Add audio data: %d points", static_cast<int>(values.size()));
    std::unique_lock<std::mutex> lock(m_pending_sound_data_mutex);
    m_pending_sound_data.insert(m_pending_sound_data.end(), values.begin(), values.end());
}

lv_obj_t *Gui::get_page(int page_index)
{
    switch (page_index)
    {
    case 0:
        return objects.clock;
    case 1:
        return objects.main;
    case 2:
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
        anim = LV_SCR_LOAD_ANIM_MOVE_LEFT;
    }
    else
    {
        m_current_page_index = (m_current_page_index - 1 + PAGE_COUNT) % PAGE_COUNT;
        anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    }

    lv_obj_t *page = get_page(m_current_page_index);
    lv_scr_load_anim(page, anim, 200, 0, false);
    m_right_encoder->set_page(page);
}
