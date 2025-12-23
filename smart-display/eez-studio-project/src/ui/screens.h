#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *date;
    lv_obj_t *weather;
    lv_obj_t *mon;
    lv_obj_t *tue;
    lv_obj_t *wed;
    lv_obj_t *thu;
    lv_obj_t *fri;
    lv_obj_t *sat;
    lv_obj_t *sun;
    lv_obj_t *mon_date;
    lv_obj_t *tue_date;
    lv_obj_t *wed_date;
    lv_obj_t *thu_date;
    lv_obj_t *fri_date;
    lv_obj_t *sat_date;
    lv_obj_t *sun_date;
    lv_obj_t *mon_degree;
    lv_obj_t *tue_degree;
    lv_obj_t *wed_degree;
    lv_obj_t *thu_degree;
    lv_obj_t *sat_degree;
    lv_obj_t *sun_degree;
    lv_obj_t *fri_degree;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void delete_screen_main();
void tick_screen_main();

void create_screen_by_id(enum ScreensEnum screenId);
void delete_screen_by_id(enum ScreensEnum screenId);
void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/