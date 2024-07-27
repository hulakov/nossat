#pragma once

#include "hal/display.h"
#include "nossat-one/src/ui/ui.h"
#include "system/event_loop.h"
#include "hal/knob.h"
#include "hal/lvgl_knob.h"
#include "lvgl_timer.h"

class Gui
{
public:
    constexpr static const int PAGE_COUNT = 3;

    Gui(std::shared_ptr<Display> display, std::shared_ptr<EventLoop> event_loop);

    void show_message(const char *message, bool animation = false);
    void show_current_page();

private:
    lv_obj_t *get_page(int page_index);
    void switch_to_screen(bool right);
    void initialize_knobs(std::shared_ptr<EventLoop> event_loop);
    void update_clock();

private:
    std::shared_ptr<Display> m_display;
    int m_current_page_index = 0;

    std::shared_ptr<Knob> m_left_encoder;
    std::shared_ptr<LvglKnob> m_right_encoder;
    std::shared_ptr<LvglTimer> m_clock_timer;
};
