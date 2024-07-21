#include "speech_recognition.h"

#include "esp_log.h"

#include "gui/lvgl/font/font_en_24.h"
#include "gui/lvgl/image/mic_logo.h"

#include <cmath>

const constexpr char *TAG = "lvgl_ui";

const constexpr int32_t ANIMATION_STEP = 40;
const constexpr int ANIMATION_SPEED_MS = 250;
const constexpr int32_t MIN_BAR_VALUE = -100;
const constexpr int32_t MAX_BAR_VALUE = 100;
const constexpr size_t BAR_COUNT = 8;

static bool g_sr_anim_active = false;
static int32_t g_sr_anim_count = 0;
static lv_obj_t *g_sr_label = NULL;
static lv_obj_t *g_sr_mask = NULL;
static lv_obj_t *g_sr_bar[BAR_COUNT] = {NULL};

lv_timer_t *sr_timer;

static void ui_speech_anim_cb(lv_timer_t *timer)
{
    if (!g_sr_anim_active)
        return;

    for (size_t i = 0; i < BAR_COUNT / 2; i++)
    {
        const double rad = static_cast<double>((g_sr_anim_count + i) * ANIMATION_STEP) * 3.14 / 180.0;
        const double val = std::abs(std::sin(rad)) * 50.0;

        lv_bar_set_value(g_sr_bar[i], val, LV_ANIM_ON);
        lv_bar_set_value(g_sr_bar[BAR_COUNT - i - 1], val, LV_ANIM_ON);
    }
    g_sr_anim_count++;
}

void ui_initialize(lv_disp_t *disp)
{
    ESP_LOGI(TAG, "sr animation initialize");
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    g_sr_mask = lv_obj_create(scr);
    lv_obj_set_size(g_sr_mask, lv_obj_get_width(lv_obj_get_parent(g_sr_mask)),
                    lv_obj_get_height(lv_obj_get_parent(g_sr_mask)));
    lv_obj_clear_flag(g_sr_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_sr_mask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_radius(g_sr_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_sr_mask, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_sr_mask, lv_obj_get_style_bg_color(lv_obj_get_parent(g_sr_mask), LV_PART_MAIN),
                              LV_STATE_DEFAULT);
    lv_obj_align(g_sr_mask, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *obj_img = lv_obj_create(g_sr_mask);
    lv_obj_set_size(obj_img, 80, 80);
    lv_obj_clear_flag(obj_img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(obj_img, 40, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj_img, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(obj_img, 40, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(obj_img, LV_OPA_30, LV_STATE_DEFAULT);
    lv_obj_align(obj_img, LV_ALIGN_CENTER, 0, -30);
    lv_obj_t *img_mic_logo = lv_img_create(obj_img);
    lv_img_set_src(img_mic_logo, &mic_logo);
    lv_obj_center(img_mic_logo);

    g_sr_label = lv_label_create(g_sr_mask);
    lv_obj_set_style_text_font(g_sr_label, &font_en_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_sr_label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align(g_sr_label, LV_ALIGN_CENTER, 0, 80);

    for (size_t i = 0; i < sizeof(g_sr_bar) / sizeof(g_sr_bar[0]); i++)
    {
        g_sr_bar[i] = lv_bar_create(g_sr_mask);
        lv_obj_set_size(g_sr_bar[i], 5, 60);
        lv_obj_set_style_anim_time(g_sr_bar[i], 400, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_sr_bar[i], lv_color_make(237, 238, 239), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_sr_bar[i], lv_color_make(246, 175, 171), LV_PART_INDICATOR);
        lv_bar_set_range(g_sr_bar[i], MIN_BAR_VALUE, MAX_BAR_VALUE);
        lv_bar_set_value(g_sr_bar[i], MIN_BAR_VALUE, LV_ANIM_OFF);
        lv_bar_set_start_value(g_sr_bar[i], MIN_BAR_VALUE, LV_ANIM_OFF);
        lv_obj_set_style_anim_time(g_sr_bar[i], ANIMATION_SPEED_MS, LV_STATE_DEFAULT);
    }

    for (size_t i = 0; i < sizeof(g_sr_bar) / sizeof(g_sr_bar[0]) / 2; i++)
    {
        lv_obj_align_to(g_sr_bar[i], obj_img, LV_ALIGN_OUT_LEFT_MID, 15 * i - 65, 0);
        lv_obj_align_to(g_sr_bar[i + 4], obj_img, LV_ALIGN_OUT_RIGHT_MID, 15 * i + 20, 0);
    }

    g_sr_anim_count = 0;
    g_sr_anim_active = false;
    sr_timer = lv_timer_create(ui_speech_anim_cb, ANIMATION_SPEED_MS, NULL);
    lv_timer_pause(sr_timer);
}

void ui_show_message(const char *text, bool animation)
{
    g_sr_anim_active = animation;
    lv_label_set_text_static(g_sr_label, text);

    if (animation)
    {
        g_sr_anim_count = 0;
        lv_timer_resume(sr_timer);
        lv_obj_move_foreground(g_sr_mask);
    }
    else
    {
        lv_timer_pause(sr_timer);
        for (size_t i = 0; i < BAR_COUNT; i++)
        {
            lv_bar_set_value(g_sr_bar[i], MIN_BAR_VALUE, LV_ANIM_ON);
            lv_bar_set_start_value(g_sr_bar[i], MIN_BAR_VALUE, LV_ANIM_ON);
        }
    }

    lv_obj_clear_flag(g_sr_mask, LV_OBJ_FLAG_HIDDEN);
}

void ui_hide_message()
{
    lv_obj_add_flag(g_sr_mask, LV_OBJ_FLAG_HIDDEN);
}