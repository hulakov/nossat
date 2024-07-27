#pragma once

#include "lvgl.h"
#include <functional>

class LvglTimer
{
public:
    using Handler = std::function<void()>;
    LvglTimer(Handler handler, uint32_t period);
    ~LvglTimer();

private:
    const Handler m_handler;
    lv_timer_t *m_timer = 0;
};
