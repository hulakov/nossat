#include "gui_one.h"

#include "lvgl/lvgl_ui.h"

#include <mutex>

Gui::Gui(std::shared_ptr<Display> display) : m_display(display)
{
    ui_lvgl_main(display->get_lv_display());
}

void Gui::show_message(const char *message)
{
}

void Gui::hide_message()
{
}

void Gui::set_value(int32_t value)
{
    std::unique_lock<Display> lock(*m_display);
    ui_set_value(value);
}
