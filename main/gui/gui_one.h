#pragma once

#include "hal/display.h"
#include "nossat-one/src/ui/ui.h"
#include "system/event_loop.h"
#include "hal/knob.h"
#include "hal/lvgl_knob.h"
#include "lvgl_timer.h"
#include "sound/audio_data.h"

class Gui
{
public:
    constexpr static const int PAGE_COUNT = 3;

    Gui(std::shared_ptr<Display> display, std::shared_ptr<EventLoop> event_loop);

    void show_message(const char *message, bool animation = false);
    void show_current_page();

    void show_recording_screen();
    void add_recording_data(const std::vector<int32_t> &values);

private:
    lv_obj_t *get_page(int page_index);
    void switch_to_screen(bool right);
    void initialize_knobs(std::shared_ptr<EventLoop> event_loop);
    void initialize_sound_chart();
    void update_clock();
    void update_sound_chart();

private:
    std::shared_ptr<Display> m_display;
    int m_current_page_index = 0;

    std::shared_ptr<Knob> m_left_encoder;
    std::shared_ptr<LvglKnob> m_right_encoder;
    std::shared_ptr<LvglTimer> m_clock_timer;

    lv_chart_series_t *m_audio_serie = nullptr;

    std::shared_ptr<LvglTimer> m_sound_chart_timer;
    std::vector<int32_t> m_pending_sound_data;
    std::mutex m_pending_sound_data_mutex;
};
