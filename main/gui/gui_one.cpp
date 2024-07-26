#include "gui_one.h"
#include "nossat-one/src/ui/ui.h"
#include <mutex>

Gui::Gui(std::shared_ptr<Display> display) : m_display(display)
{
    create_screens();
}

void Gui::show_message(const char *message, bool animation)
{
    std::unique_lock<Display> lock(*m_display);
    lv_label_set_text_static(objects.message_label, message);
    loadScreen(SCREEN_ID_MESSAGE_BOX);
}

void Gui::hide_message()
{
    std::unique_lock<Display> lock(*m_display);
    loadScreen(SCREEN_ID_MAIN);
}

void Gui::set_value(int32_t value)
{
    // std::unique_lock<Display> lock(*m_display);
    // ui_set_value(value);
}
