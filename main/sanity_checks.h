#pragma once

#include "esp_err.h"
#include "esp_log.h"

#define ESP_TRUE_CHECK(x)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        if (unlikely(!(x)))                                                                                            \
        {                                                                                                              \
            _esp_error_check_failed(ESP_FAIL, __FILE__, __LINE__, __ASSERT_FUNC, #x);                                  \
        }                                                                                                              \
    } while (false)

#define ENSURE_NO_ESP_ERRORS ESP_ERROR_CHECK
#define ENSURE_TRUE ESP_TRUE_CHECK