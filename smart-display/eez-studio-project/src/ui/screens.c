#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.date = obj;
            lv_obj_set_pos(obj, 454, 9);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "MON 22.22.2222 22:22:22");
        }
        {
            // weather
            lv_obj_t *obj = lv_chart_create(parent_obj);
            objects.weather = obj;
            lv_obj_set_pos(obj, 14, 9);
            lv_obj_set_size(obj, 193, 139);
        }
        {
            // mon
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.mon = obj;
            lv_obj_set_pos(obj, 14, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Mon");
        }
        {
            // tue
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tue = obj;
            lv_obj_set_pos(obj, 96, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Tue");
        }
        {
            // wed
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wed = obj;
            lv_obj_set_pos(obj, 167, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Wed");
        }
        {
            // thu
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.thu = obj;
            lv_obj_set_pos(obj, 245, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Thu");
        }
        {
            // fri
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fri = obj;
            lv_obj_set_pos(obj, 316, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Fri");
        }
        {
            // sat
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.sat = obj;
            lv_obj_set_pos(obj, 383, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Sat");
        }
        {
            // sun
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.sun = obj;
            lv_obj_set_pos(obj, 454, 356);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Sun");
        }
        {
            // mon_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.mon_date = obj;
            lv_obj_set_pos(obj, 21, 333);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
        {
            // tue_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tue_date = obj;
            lv_obj_set_pos(obj, 109, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
        {
            // wed_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wed_date = obj;
            lv_obj_set_pos(obj, 184, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
        {
            // thu_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.thu_date = obj;
            lv_obj_set_pos(obj, 257, 333);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
        {
            // fri_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fri_date = obj;
            lv_obj_set_pos(obj, 317, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
        {
            // sat_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.sat_date = obj;
            lv_obj_set_pos(obj, 395, 335);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
        {
            // sun_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.sun_date = obj;
            lv_obj_set_pos(obj, 468, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_jersey_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "22");
        }
    }
    
    tick_screen_main();
}

void delete_screen_main() {
    lv_obj_del(objects.main);
    objects.main = 0;
    objects.date = 0;
    objects.weather = 0;
    objects.mon = 0;
    objects.tue = 0;
    objects.wed = 0;
    objects.thu = 0;
    objects.fri = 0;
    objects.sat = 0;
    objects.sun = 0;
    objects.mon_date = 0;
    objects.tue_date = 0;
    objects.wed_date = 0;
    objects.thu_date = 0;
    objects.fri_date = 0;
    objects.sat_date = 0;
    objects.sun_date = 0;
}

void tick_screen_main() {
}



typedef void (*create_screen_func_t)();
create_screen_func_t create_screen_funcs[] = {
    create_screen_main,
};
void create_screen(int screen_index) {
    create_screen_funcs[screen_index]();
}
void create_screen_by_id(enum ScreensEnum screenId) {
    create_screen_funcs[screenId - 1]();
}

typedef void (*delete_screen_func_t)();
delete_screen_func_t delete_screen_funcs[] = {
    delete_screen_main,
};
void delete_screen(int screen_index) {
    delete_screen_funcs[screen_index]();
}
void delete_screen_by_id(enum ScreensEnum screenId) {
    delete_screen_funcs[screenId - 1]();
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
