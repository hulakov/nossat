#include "task.h"

#include "sanity_checks.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct TaskContext
{
    Proc proc;
};

void create_task(Proc proc, const char *name, uint32_t stack_depth, int priority, int affinity)
{
    TaskHandle_t handle;
    TaskFunction_t adapter = [](void *param)
    {
        auto context = reinterpret_cast<TaskContext *>(param);
        Proc proc = context->proc;
        delete context;
        proc();
        vTaskDelete(NULL);
    };

    auto context = new TaskContext{.proc = proc};
    ENSURE_TRUE(xTaskCreatePinnedToCore(adapter, name, stack_depth, context, priority, &handle, affinity));
}