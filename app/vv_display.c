

#include "vv_display.h"
#include "vv_thermostat.h"

#define _VV_GRAPH_TOP 80
#define _VV_GRAPH_BOTTOM 110
#define _VV_GRAPH_SIDE 10

void _vv_display_init_single_data(int page_index, int data_index, char *name, char *location, char *format);

int _left_intend_to_center(char *str, uint8_t char_size) {
    return (int)fmax(1, (128 - strlen(str) * char_size) / 2);
}

void _draw_header(struct vv_display_page_t *page) {
    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(
            _left_intend_to_center(page -> name, 7),
            5,
            page -> name,
            true
    );

    bc_module_lcd_set_font(&bc_font_ubuntu_24);
    bc_module_lcd_draw_string(
            _left_intend_to_center(page -> location, 11),
            20,
            page -> location,
            true
    );
}

void _draw_value(char *format, const float *value, uint8_t top) {
    char str[32];
    bc_module_lcd_set_font(&bc_font_ubuntu_33);
    snprintf(str, sizeof(str), format, *value);

    bc_module_lcd_draw_string(_left_intend_to_center(str, 14), top, str, true);
}

void _draw_graph(vv_display_data_t *data) {
    float min = * data -> min_value;
    float max = * data -> max_value;

    float x;
    float y;

    if (max != min) {
        //(19.3, 20.3) -> (115 -> 80)
        //(19.3, 20.3) -> (35 -> 0)

        //19.3*x + y = 35
        //20.3*x + y = 0

        // x = -35 / (max - min)
        // y = - x * max

        x = -(_VV_GRAPH_BOTTOM - _VV_GRAPH_TOP) / (max - min);
        y = -x * max + _VV_GRAPH_TOP;
    } else {
        // (10, 10) -> (115 -> 80)
        // (10, 10) -> 195 2 = 98
        x = 0;
        y = 98;
    }

    int left_step = (128 - (2 * _VV_GRAPH_SIDE)) / VV_VALUES_COUNT;

    int last_top = (int)(x * (data -> values[0]) + y);
    int last_left = _VV_GRAPH_SIDE;

    for (uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
        int top = (int) (x * (data -> values[i]) + y);
        int left = _VV_GRAPH_SIDE + left_step * i;

        bc_module_lcd_set_font(&bc_font_ubuntu_11);
        bc_module_lcd_draw_char(left - 2, top - 6, 'o', true);

        bc_module_lcd_draw_line(last_left, last_top, left, top, true);
        last_top = top;
        last_left = left;
    }
}

void _draw_footer(vv_display_page_t *page) {
    float max = * page -> data -> max_value;
    char max_str[8];
    snprintf(max_str, sizeof(max_str), page -> format, max);

    float min = * page -> data -> min_value;
    char min_str[8];
    snprintf(min_str, sizeof(min_str), page -> format, min);

    char str[64];
    snprintf(str, sizeof(str), "%s / %s", min_str, max_str);

    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(_left_intend_to_center(str, 7), 113, str, true);
}

void _draw_data(vv_display_page_t *page) {
    _draw_header(page);
    _draw_value(
            page -> format,
            page -> data -> values + VV_VALUES_COUNT - 1, // Draw last element of array
            45
    );
    _draw_graph(page -> data);
    _draw_footer(page);
}

void _draw_controller(vv_display_page_t *page) {
    _draw_header(page);
    _draw_value(
            page -> format,
            vv_thermostat_get_actual_value(page -> controller -> thermostat),
            45
    );
    if (vv_thermostat_is_local_controll(page -> controller -> thermostat)) {
        _draw_value(
                page -> format,
                vv_thermostat_get_reference_value(page -> controller -> thermostat),
                80
        );
    }

    if (vv_thermostat_get_actual_state(page -> controller -> thermostat)) {
        char *str = "Heating";
        bc_module_lcd_set_font(&bc_font_ubuntu_15);
        bc_module_lcd_draw_string(_left_intend_to_center(str, 7), 113, str, true);
    }
}

void vv_display_blink_red() {
    bc_led_pulse(&vv_display.red_led, 1000);
}

void vv_display_blink_green() {
    bc_led_pulse(&vv_display.green_led, 1000);
}

void vv_display_init(struct vv_thermostat_self *_thermostat) {
    vv_display.actual_page_index = 0;

    _vv_display_init_single_data(0, VV_DATA_TYPE_L1_POWER, "L1 power [kW]", "House", "%.2f");
    _vv_display_init_single_data(1, VV_DATA_TYPE_FVE_POWER, "Fve power [W]", "House", "%.0f");
    _vv_display_init_single_data(2, VV_DATA_TYPE_TEMPERATURE_LIVING_ROOM, "Temperature [\xb0]", "Living room", "%.2f");
    _vv_display_init_single_data(3, VV_DATA_TYPE_TEMPERATURE_TERRACE, "Temperature [\xb0]", "Terrace", "%.2f");
    _vv_display_init_single_data(4, VV_DATA_TYPE_TEMPERATURE_BEDROOM, "Temperature [\xb0]", "Bedroom", "%.2f");
    _vv_display_init_single_data(5, VV_DATA_TYPE_CO2, "CO2 [ppm]", "    Bedroom", "%.0f");

    vv_display.pages[6].name = "Heating [\xb0]";
    vv_display.pages[6].location = "Home  ";
    vv_display.pages[6].format = "%.2f";
    vv_display.controllers[0].thermostat = _thermostat;
    vv_display.pages[6].data = NULL;
    vv_display.pages[6].controller = &vv_display.controllers[0];

    bc_led_init_virtual(&vv_display.green_led, BC_MODULE_LCD_LED_GREEN, bc_module_lcd_get_led_driver(), 1);
    bc_led_init_virtual(&vv_display.red_led, BC_MODULE_LCD_LED_RED, bc_module_lcd_get_led_driver(), 1);
}

void _vv_display_init_single_data(
        int page_index,
        int data_index,
        char *name,
        char *location,
        char *format
) {
    vv_display.pages[page_index].name = name;
    vv_display.pages[page_index].location = location;
    vv_display.pages[page_index].format = format;
    for (uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
        vv_display.data[data_index].values[i] = 0;
    }
    vv_display.data[data_index].min_value = 0;
    vv_display.data[data_index].max_value = 0;
    vv_display.pages[page_index].data = &vv_display.data[data_index];
    vv_display.pages[page_index].controller = NULL;
}

void vv_display_render() {
    if (!bc_module_lcd_is_ready()) {
        return;
    }

    bc_module_core_pll_enable();
    bc_module_lcd_clear();

    if(vv_display.pages[vv_display.actual_page_index].data != NULL) {
        _draw_data(&vv_display.pages[vv_display.actual_page_index]);
    } else if(vv_display.pages[vv_display.actual_page_index].controller != NULL) {
        _draw_controller(&vv_display.pages[vv_display.actual_page_index]);
    }

    bc_module_core_pll_disable();
    bc_module_lcd_update();
}

void vv_display_next_page() {
    vv_display.actual_page_index = (int8_t)((vv_display.actual_page_index + 1) % VV_PAGES_COUNT);
    vv_display_render();
}

void vv_display_prev_page() {
    vv_display.actual_page_index--;
    if (vv_display.actual_page_index < 0) vv_display.actual_page_index = VV_PAGES_COUNT - 1;
    vv_display_render();
}

void vv_display_push_new_value(uint8_t index, float_t new_value) {
    for (uint8_t i = 0; i < VV_VALUES_COUNT - 1; i++) {
        vv_display.data[index].values[i] = vv_display.data[index].values[i + 1];
    }
    vv_display.data[index].values[VV_VALUES_COUNT - 1] = new_value;
    if(new_value > * vv_display.data[index].max_value) {
        vv_display.data[index].max_value = &vv_display.data[index].values[VV_VALUES_COUNT - 1];
    }
    if(new_value < * vv_display.data[index].min_value) {
        vv_display.data[index].min_value = &vv_display.data[index].values[VV_VALUES_COUNT - 1];
    }
    vv_display_render();
}
