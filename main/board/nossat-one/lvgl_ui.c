#include "lvgl.h"
#include <stdio.h>

#include "bsp/esp-bsp.h"

lv_obj_t *arc = 0;
lv_obj_t *label = 0;
lv_disp_t *main_disp = 0;
// static void set_angle(void *obj, int32_t v)
// {
//     lv_arc_set_value(obj, v);
// }
void set_label_style(lv_obj_t *label)
{
    // Create a new style
    static lv_style_t style;
    lv_style_init(&style);

    // Set the text color to white
    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

    // Apply the style to the label
    lv_obj_add_style(label, &style, 0);
}

void lvgl_ui_main(lv_disp_t *disp)
{
    main_disp = disp;

    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    /*Create an Arc*/
    arc = lv_arc_create(scr);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
    lv_obj_set_size(arc, 100, 100);
    lv_obj_center(arc);
    lv_arc_set_value(arc, 0);

    label = lv_label_create(scr);
    lv_obj_set_width(label, 70);  /// 1
    lv_obj_set_height(label, 25); /// 1
    // lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    set_label_style(label);

    lv_label_set_text(label, "0%");
    // lv_anim_t a;
    // lv_anim_init(&a);
    // lv_anim_set_var(&a, arc);
    // lv_anim_set_exec_cb(&a, set_angle);
    // lv_anim_set_time(&a, 1000);
    // lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    // lv_anim_set_repeat_delay(&a, 500);
    // lv_anim_set_values(&a, 0, 100);
    // lv_anim_start(&a);
}

void set_lvgl_value(int32_t v)
{
    bsp_display_lock(0);
    lv_arc_set_value(arc, v);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d %%", (int)(v));
    lv_label_set_text(label, buf);

    bsp_display_unlock();
}