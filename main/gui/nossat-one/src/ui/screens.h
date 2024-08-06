#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *clock;
    lv_obj_t *settings;
    lv_obj_t *message_box;
    lv_obj_t *sound_recorder;
    lv_obj_t *date_label;
    lv_obj_t *message_label;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *sound_chart;
    lv_obj_t *time_label;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_CLOCK = 2,
    SCREEN_ID_SETTINGS = 3,
    SCREEN_ID_MESSAGE_BOX = 4,
    SCREEN_ID_SOUND_RECORDER = 5,
};

void create_screen_main();
void tick_screen_main();

void create_screen_clock();
void tick_screen_clock();

void create_screen_settings();
void tick_screen_settings();

void create_screen_message_box();
void tick_screen_message_box();

void create_screen_sound_recorder();
void tick_screen_sound_recorder();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/