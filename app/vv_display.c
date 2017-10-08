

#include "vv_display.h"
#include <math.h>


#define _VV_GRAPH_TOP 80
#define _VV_GRAPH_BOTTOM 115
#define _VV_GRAPH_SIDE 14

//struct display_data actual_data[] = {
//    { .name = "L1 power", .location = "House", .format="%.1f kW", .values = {0.7, 1.1, 0.5, -0.1, -0.8} },
//    { .name = "Temperature", .location = "Living room", .format="%.1f \xb0", .values = {19.3, 18.7, 19.2, 19.6, 20.3} },
//};
//
//size_t _actual_data_size = 2;
//int8_t _actual_data_index = 0;

int _left_intend_to_center(char *str, uint8_t char_size) {
    return fmax(1, (128 - strlen(str) * char_size) / 2);
}

void _draw_header() {
    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(_left_intend_to_center(vv_display_data.actual_data[vv_display_data._actual_data_index].name, 8), 5, vv_display_data.actual_data[vv_display_data._actual_data_index].name, true);

    bc_module_lcd_set_font(&bc_font_ubuntu_24);
    bc_module_lcd_draw_string(_left_intend_to_center(vv_display_data.actual_data[vv_display_data._actual_data_index].location, 11), 20, vv_display_data.actual_data[vv_display_data._actual_data_index].location, true);
}

void _draw_last_value() {
    char str[32];
    bc_module_lcd_set_font(&bc_font_ubuntu_33);
    snprintf(str, sizeof(str), vv_display_data.actual_data[vv_display_data._actual_data_index].format, vv_display_data.actual_data[vv_display_data._actual_data_index].values[VV_VALUES_COUNT - 1]);

    bc_module_lcd_draw_string(_left_intend_to_center(str, 13), 45, str, true); 
}

void _draw_graph() {
    float_t min = 10e10;
    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	if(min > vv_display_data.actual_data[vv_display_data._actual_data_index].values[i]) {
	    min = vv_display_data.actual_data[vv_display_data._actual_data_index].values[i];
	}
    }

    float_t max = -10e10;
    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	if(max < vv_display_data.actual_data[vv_display_data._actual_data_index].values[i]) {
	    max = vv_display_data.actual_data[vv_display_data._actual_data_index].values[i];
	}
    }

    //(19.3, 20.3) -> (115 -> 80)
    //(19.3, 20.3) -> (35 -> 0)

    //19.3*x + y = 35
    //20.3*x + y = 0

    // x = -35 / (max - min)
    // y = - x * max

    float_t x = - (_VV_GRAPH_BOTTOM - _VV_GRAPH_TOP) / (max - min);
    float_t y = - x * max + _VV_GRAPH_TOP;

    int left_step = (128 - (2 * _VV_GRAPH_SIDE)) / VV_VALUES_COUNT;

    int last_top = x * vv_display_data.actual_data[vv_display_data._actual_data_index].values[0] + y;
    int last_left = _VV_GRAPH_SIDE;

    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	int top = (int)(x * vv_display_data.actual_data[vv_display_data._actual_data_index].values[i] + y);
	int left = _VV_GRAPH_SIDE + left_step * i;

        bc_module_lcd_set_font(&bc_font_ubuntu_11);
        bc_module_lcd_draw_char(left - 2, top - 6, 'o', true);
	

	bc_module_lcd_draw_line(last_left, last_top, left, top, true);
	last_top = top;
	last_left = left;
    }
}

void vv_display_init() {
    vv_display_data._actual_data_index = 0;

    vv_display_data.actual_data[0].name = "L1 power";
    vv_display_data.actual_data[0].location = "House";
    vv_display_data.actual_data[0].format = "%.1f kW";
    vv_display_data.actual_data[0].values[0] = 0.7;
    vv_display_data.actual_data[0].values[1] = 1.1;
    vv_display_data.actual_data[0].values[2] = 0.5;
    vv_display_data.actual_data[0].values[3] = -0.1;
    vv_display_data.actual_data[0].values[4] = -0.8;
    
    vv_display_data.actual_data[1].name = "Temperature";
    vv_display_data.actual_data[1].location = "Living room";
    vv_display_data.actual_data[1].format = "%.1f \xb0";
    vv_display_data.actual_data[1].values[0] = 19.3;
    vv_display_data.actual_data[1].values[1] = 18.7;
    vv_display_data.actual_data[1].values[2] = 19.2;
    vv_display_data.actual_data[1].values[3] = 19.6;
    vv_display_data.actual_data[1].values[4] = 20.3;
    
//    vv_display_data.actual_data = {
//	{ .name = "L1 power", .location = "House", .format="%.1f kW", .values = {0.7, 1.1, 0.5, -0.1, -0.8} },
//	{ .name = "Temperature", .location = "Living room", .format="%.1f \xb0", .values = {19.3, 18.7, 19.2, 19.6, 20.3} },
//    };
}

void vv_lcd_next_page() {
    vv_display_data._actual_data_index = (vv_display_data._actual_data_index + 1) % VV_DATA_COUNT;
}

void vv_lcd_prev_page() {
    vv_display_data._actual_data_index--;
    if(vv_display_data._actual_data_index < 0)  vv_display_data._actual_data_index = VV_DATA_COUNT - 1;    
}

void vv_lcd_page_render() {

    bc_module_core_pll_enable();
    bc_module_lcd_clear();

    _draw_header();
    _draw_last_value();
    _draw_graph();

    bc_module_core_pll_disable();
}

