#include "styles.h"
#include "images.h"
#include "fonts.h"

void apply_style_button(lv_obj_t *obj) {
    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
};

