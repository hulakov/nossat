#pragma once

#include "hal/display.h"

class Gui
{
public:
    Gui(std::shared_ptr<Display> display);

    void show_message(const char *message, bool animation = false);
    void hide_message();

private:
    std::shared_ptr<Display> m_display;
};
