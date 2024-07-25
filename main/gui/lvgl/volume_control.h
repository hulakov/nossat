#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

void ui_lvgl_main(lv_disp_t *disp);
void ui_set_value(int32_t v);

#ifdef __cplusplus
}
#endif
