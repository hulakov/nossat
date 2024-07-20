#pragma once

#include "esp_err.h"
#include "esp_log.h"

#define BSP_ERROR_CHECK_RETURN_NULL(x)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        if (unlikely((x) != ESP_OK))                                                                                   \
        {                                                                                                              \
            return NULL;                                                                                               \
        }                                                                                                              \
    } while (0)

#define BSP_NULL_CHECK(x, ret)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((x) == NULL)                                                                                               \
        {                                                                                                              \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)
#define BSP_ERROR_CHECK_RETURN_ERR(x)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        esp_err_t err_rc_ = (x);                                                                                       \
        if (unlikely(err_rc_ != ESP_OK))                                                                               \
        {                                                                                                              \
            return err_rc_;                                                                                            \
        }                                                                                                              \
    } while (0)

#define ESP_TRUE_CHECK(x)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (unlikely(!(x)))                                                                                            \
        {                                                                                                              \
            _esp_error_check_failed(ESP_FAIL, __FILE__, __LINE__, __ASSERT_FUNC, #x);                                  \
        }                                                                                                              \
    } while (false)
