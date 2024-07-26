#pragma once

#include "hal/display.h"

class Gui
{
public:
    Gui(std::shared_ptr<Display> display);

    void show_message(const char *message, bool animation = false);
    void hide_message();

    void set_value(int32_t value);

private:
    std::shared_ptr<Display> m_display;
};
