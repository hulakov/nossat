#include "lvgl_timer.h"

LvglTimer::LvglTimer(Handler handler, uint32_t period) : m_handler(handler)
{
    auto adapter = [](lv_timer_t *timer)
    {
        auto self = reinterpret_cast<LvglTimer *>(timer->user_data);
        self->m_handler();
    };
    m_timer = lv_timer_create(adapter, period, this);
}

LvglTimer::~LvglTimer()
{
    lv_timer_delete(m_timer);
}
