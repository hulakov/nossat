#include "gui_box.h"
#include "esp_log.h"
#include "lvgl/speech/lvgl_ui.h"

#include <mutex>

static const char *TAG = "gui";

Gui::Gui(std::shared_ptr<Display> display) : m_display(display)
{
    ESP_LOGI(TAG, "Configure LVGL");
    ui_initialize(display->get_lv_display());
}

void Gui::show_message(const char *message, bool animation)
{
    std::unique_lock<Display> lock(*m_display);
    ui_show_message(message, animation);
}

void Gui::hide_message()
{
    std::unique_lock<Display> lock(*m_display);
    ui_hide_message();
}
