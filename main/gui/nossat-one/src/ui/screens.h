#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *message_box;
    lv_obj_t *message_label;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_MESSAGE_BOX = 2,
    SCREEN_ID_TOGGLE = 3,
};

void create_screen_main();
void tick_screen_main();

void create_screen_message_box();
void tick_screen_message_box();

void create_user_widget_toggle(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_toggle(int startWidgetIndex);

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/