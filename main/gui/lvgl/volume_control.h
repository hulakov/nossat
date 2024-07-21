#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

void ui_lvgl_main(lv_disp_t *disp);
void ui_set_value(int32_t v);

void ui_show_message(const char *message);
void ui_hide_message();

#ifdef __cplusplus
}
#endif
