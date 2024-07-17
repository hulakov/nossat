#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <functional>
#include <memory>

using Handler = std::function<void()>;

class EventLoop : public std::enable_shared_from_this<EventLoop>
{
public:
    EventLoop();

    void run();
    void post(Handler handler);
    void post_from_isr(Handler handler);

private:
    QueueHandle_t m_queue = nullptr;
};