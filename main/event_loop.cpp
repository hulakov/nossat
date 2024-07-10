#include "event_loop.h"

EventLoop::EventLoop() : m_queue(xQueueCreate(10, sizeof(void *)))
{
}

void EventLoop::run()
{
    // Report counter value
    while (true)
    {
        Handler *handler = nullptr;
        if (xQueueReceive(m_queue, &handler, pdMS_TO_TICKS(1000)))
        {
            (*handler)();
            delete handler;
        }
    }
}

void EventLoop::post(Handler handler)
{
    auto handler_ptr = new Handler(handler);
    xQueueSend(m_queue, &handler_ptr, 0);
}

void EventLoop::post_from_isr(Handler handler)
{
    BaseType_t high_task_wakeup = 0;
    auto handler_ptr = new Handler(handler);
    xQueueSendFromISR(m_queue, &handler_ptr, &high_task_wakeup);
}