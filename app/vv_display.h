
#ifndef VV_DISPLAY_H
#define VV_DISPLAY_H

#include <bc_common.h>
#include <bcl.h>

#define VV_DATA_COUNT 7
#define VV_CONTROLLERS_COUNT 1

#define VV_PAGES_COUNT (VV_DATA_COUNT + VV_CONTROLLERS_COUNT)
#define VV_VALUES_COUNT 15

#define VV_DATA_TYPE_L1_POWER 0
#define VV_DATA_TYPE_FVE_POWER 1
#define VV_DATA_TYPE_TEMPERATURE_LIVING_ROOM 2
#define VV_DATA_TYPE_TEMPERATURE_TERRACE 3
#define VV_DATA_TYPE_TEMPERATURE_BEDROOM 4
#define VV_DATA_TYPE_CO2 5
#define VV_DATA_TYPE_LOCAL_TEMPERATURE 6

struct vv_display_data_t {
    float values[VV_VALUES_COUNT];
    float min_value;
    float max_value;
};
typedef struct vv_display_data_t vv_display_data_t;

struct vv_display_controller_t {
    uint8_t selected_watering_section;
};
typedef struct vv_display_controller_t vv_display_controller_t;

struct vv_display_page_t {
    char* name;
    char* location;
    char *format;

    vv_display_data_t *data;
    vv_display_controller_t *controller;
};
typedef struct vv_display_page_t vv_display_page_t;

struct {
    int8_t actual_page_index;
    bc_led_t green_led;
    bc_led_t red_led;
    vv_display_data_t data[VV_DATA_COUNT];
    vv_display_controller_t controllers[VV_CONTROLLERS_COUNT];
    vv_display_page_t pages[VV_DATA_COUNT + VV_CONTROLLERS_COUNT];
} vv_display;

void vv_display_init();
void vv_display_render();
void vv_display_next_page();
void vv_display_prev_page();

void vv_display_push_new_value(uint8_t index, float new_value);

void vv_display_blink_red();
void vv_display_blink_green();

#endif
