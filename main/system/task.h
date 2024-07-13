#pragma once

#include <functional>

using Proc = std::function<void()>;

void create_task(Proc proc, const char *name, uint32_t stack_depth, int priority, int affinity);