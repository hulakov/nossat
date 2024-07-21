#pragma once

#include "lvgl.h"

void ui_initialize(lv_disp_t *disp);
void ui_show_message(const char *text, bool animation = false);
void ui_hide_message();
