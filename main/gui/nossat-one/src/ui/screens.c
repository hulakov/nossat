#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

objects_t objects;
lv_obj_t *tick_value_change_obj;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 160, 128);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(obj, LV_DIR_ALL);
    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 14, 11);
            lv_obj_set_size(obj, LV_PCT(100), 30);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            apply_style_button(obj);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 8, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "First");
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 15, 70);
            lv_obj_set_size(obj, LV_PCT(100), 30);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            apply_style_button(obj);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -4, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Second");
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 15, 474);
            lv_obj_set_size(obj, LV_PCT(100), 30);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            apply_style_button(obj);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 5, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Third");
                }
            }
        }
        {
            lv_obj_t *obj = lv_checkbox_create(parent_obj);
            lv_obj_set_pos(obj, 13, 731);
            lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
            lv_checkbox_set_text(obj, "Checkbox 1");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_checkbox_create(parent_obj);
            lv_obj_set_pos(obj, 8, 574);
            lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
            lv_checkbox_set_text(obj, "Checkbox 2");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_checkbox_create(parent_obj);
            lv_obj_set_pos(obj, 8, 574);
            lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
            lv_checkbox_set_text(obj, "Checkbox 3");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            lv_obj_set_pos(obj, 8, 373);
            lv_obj_set_size(obj, LV_PCT(100), 30);
            lv_dropdown_set_options(obj, "Option 1\nOption 2\nOption 3");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), 30);
            lv_style_value_t value;
            value.num = 0;
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_LEFT, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_TOP, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_RIGHT, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_BOTTOM, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_BG_OPA, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_BORDER_WIDTH, value, LV_PART_MAIN);
            create_user_widget_toggle(obj, 4);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 5, -167);
            lv_obj_set_size(obj, LV_PCT(100), 30);
            lv_style_value_t value;
            value.num = 0;
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_LEFT, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_TOP, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_RIGHT, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_PAD_BOTTOM, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_BG_OPA, value, LV_PART_MAIN);
            lv_obj_set_local_style_prop(obj, LV_STYLE_BORDER_WIDTH, value, LV_PART_MAIN);
            create_user_widget_toggle(obj, 5);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        }
    }
}

void tick_screen_main() {
    tick_user_widget_toggle(4);
    tick_user_widget_toggle(5);
}

void create_screen_message_box() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.message_box = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 160, 128);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // messageLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.message_label = obj;
            lv_obj_set_pos(obj, 0, 56);
            lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Turn off the light");
        }
    }
}

void tick_screen_message_box() {
}

void create_user_widget_toggle(lv_obj_t *parent_obj, int startWidgetIndex) {
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_rounded(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_rounded(obj, false, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 40, 25);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -27, 16);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Text");
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }
}

void tick_user_widget_toggle(int startWidgetIndex) {
}


void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_message_box();
}

typedef void (*tick_screen_func_t)();

tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_message_box,
    0,
};

void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
