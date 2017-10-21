

#include "vv_display.h"
#include "vv_thermostat.h"
#include <math.h>

#define _VV_GRAPH_TOP 80
#define _VV_GRAPH_BOTTOM 110
#define _VV_GRAPH_SIDE 10

float _find_min(float* values, size_t count) {
    float min = 10e10;
    for(uint8_t i = 0; i < count; i++) {
	if(min > values[i]) {
	    min = values[i];
	}
    }
    return min;
}

float _find_max(float* values, size_t count) {
    float max = -10e10;
    for(uint8_t i = 0; i < count; i++) {
	if(max < values[i]) {
	    max = values[i];
	}
    }
    return max;
}

int _left_intend_to_center(char *str, uint8_t char_size) {
    return fmax(1, (128 - strlen(str) * char_size) / 2);
}

void _draw_header(char* name, char* location) {
    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(_left_intend_to_center(name, 7), 5, name, true);

    bc_module_lcd_set_font(&bc_font_ubuntu_24);
    bc_module_lcd_draw_string(_left_intend_to_center(location, 11), 20, location, true);
}

void _draw_value(char *format, float* value, uint8_t top) {
    char str[32];
    bc_module_lcd_set_font(&bc_font_ubuntu_33);
    snprintf(str, sizeof(str), format, *value);

    bc_module_lcd_draw_string(_left_intend_to_center(str, 14), top, str, true); 
}

void _draw_graph(float* values) {
    float min = _find_min(values, VV_VALUES_COUNT);
    float max = _find_max(values, VV_VALUES_COUNT);

    //(19.3, 20.3) -> (115 -> 80)
    //(19.3, 20.3) -> (35 -> 0)

    //19.3*x + y = 35
    //20.3*x + y = 0

    // x = -35 / (max - min)
    // y = - x * max

    float_t x = - (_VV_GRAPH_BOTTOM - _VV_GRAPH_TOP) / (max - min);
    float_t y = - x * max + _VV_GRAPH_TOP;

    int left_step = (128 - (2 * _VV_GRAPH_SIDE)) / VV_VALUES_COUNT;

    int last_top = x * values[0] + y;
    int last_left = _VV_GRAPH_SIDE;

    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	int top = (int)(x * values[i] + y);
	int left = _VV_GRAPH_SIDE + left_step * i;

        bc_module_lcd_set_font(&bc_font_ubuntu_11);
        bc_module_lcd_draw_char(left - 2, top - 6, 'o', true);

	bc_module_lcd_draw_line(last_left, last_top, left, top, true);
	last_top = top;
	last_left = left;
    }
}

void _draw_footer(char* format, float* values) {
    float max = _find_max(values, VV_VALUES_COUNT);    
    char max_str[8];
    snprintf(max_str, sizeof(max_str), format, max);    

    float min = _find_min(values, VV_VALUES_COUNT);
    char min_str[8];
    snprintf(min_str, sizeof(min_str), format, min);
    
    char str[64];
    snprintf(str, sizeof(str), "%s / %s", min_str, max_str);

    bc_module_lcd_set_font(&bc_font_ubuntu_15);    
    bc_module_lcd_draw_string(_left_intend_to_center(str, 7), 113, str, true);     
}

void _vv_display_push_new_value(uint8_t index, float_t new_value) {
    for(uint8_t i = 0; i < VV_VALUES_COUNT - 1; i++) {
	vv_display.actual_data[index].values[i] = vv_display.actual_data[index].values[i + 1];
    }
    vv_display.actual_data[index].values[VV_VALUES_COUNT - 1] = new_value;
}

void _draw_actual_data(uint8_t actual_data_index) {
    _draw_header(
	    vv_display.actual_data[actual_data_index].name,
	    vv_display.actual_data[actual_data_index].location
    );
    _draw_value(
	    vv_display.actual_data[actual_data_index].format,
	    vv_display.actual_data[actual_data_index].values + (VV_VALUES_COUNT - 1),
	    45
    );
    _draw_graph(
	    vv_display.actual_data[actual_data_index].values
    );
    _draw_footer(
	    vv_display.actual_data[actual_data_index].format,	    
	    vv_display.actual_data[actual_data_index].values
    );
}

void _draw_controller() {
    _draw_header(
	    vv_display.temperature_controller.name,
	    vv_display.temperature_controller.location
    );    
    _draw_value(
	    vv_display.temperature_controller.format,	    	    
	    vv_thermostat_get_actual_value(vv_display.temperature_controller.thermostat),
	    45
    );    
    _draw_value(
	    vv_display.temperature_controller.format,	    	    
	    vv_thermostat_get_reference_value(vv_display.temperature_controller.thermostat),
	    80
    );     
}

void _switch_thermostat_led() {
    if(vv_thermostat_get_actual_state(vv_display.temperature_controller.thermostat)) {
	bc_led_set_mode(&vv_display.green_led, BC_LED_MODE_ON);
    } else {
	bc_led_set_mode(&vv_display.green_led, BC_LED_MODE_OFF);
    }
}

void vv_display_init(struct vv_thermostat_self* _thermostat) {
    vv_display.actual_data_index = 0;

    vv_display.actual_data[0].name = "L1 power [kW]";
    vv_display.actual_data[0].location = "House";
    vv_display.actual_data[0].format = "%.2f";
    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	vv_display.actual_data[0].values[i] = 0;
    }
    
    vv_display.actual_data[1].name = "Temperature [\xb0]";
    vv_display.actual_data[1].location = "Living room";
    vv_display.actual_data[1].format = "%.2f";
    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	vv_display.actual_data[1].values[i] = 0;
    }

    vv_display.actual_data[2].name = "Temperature [\xb0]";
    vv_display.actual_data[2].location = "Terrace";
    vv_display.actual_data[2].format = "%.2f";
    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	vv_display.actual_data[2].values[i] = 0;
    }    

    vv_display.actual_data[3].name = "CO2 [ppm]";
    vv_display.actual_data[3].location = "Upstairs";
    vv_display.actual_data[3].format = "%.0f";
    for(uint8_t i = 0; i < VV_VALUES_COUNT; i++) {
	vv_display.actual_data[3].values[i] = 0;
    }        

    vv_display.temperature_controller.name = "Heating [\xb0]";
    vv_display.temperature_controller.location = "Home  ";
    vv_display.temperature_controller.format = "%.2f";
    vv_display.temperature_controller.thermostat = _thermostat;

    bc_led_init_virtual(&vv_display.green_led, BC_MODULE_LCD_LED_GREEN, bc_module_lcd_get_led_driver(), 1);
}

void vv_lcd_page_render() {
    bc_module_core_pll_enable();
    bc_module_lcd_clear();

    if(vv_display.actual_data_index < VV_DATA_COUNT) {
	_draw_actual_data(vv_display.actual_data_index);
    } else {
	_draw_controller();
    }
    _switch_thermostat_led();
    
    bc_module_core_pll_disable();
    bc_module_lcd_update();    
}

void vv_lcd_next_page() {
    vv_display.actual_data_index = (vv_display.actual_data_index + 1) % VV_PAGES_COUNT;
}

void vv_lcd_prev_page() {
    vv_display.actual_data_index--;
    if(vv_display.actual_data_index < 0)  vv_display.actual_data_index = VV_PAGES_COUNT - 1;    
}

#define _VV_DISPLAY_RADIO_TYPE             (0)
#define _VV_DISPLAY_RADIO_ADDRESS          (_VV_DISPLAY_RADIO_TYPE + sizeof(uint8_t))
#define _VV_DISPLAY_RADIO_DATA_TYPE_INDEX  (_VV_DISPLAY_RADIO_ADDRESS + sizeof(uint64_t))
#define _VV_DISPLAY_RADIO_NEW_VAL          (_VV_DISPLAY_RADIO_DATA_TYPE_INDEX + sizeof(uint8_t))
#define _VV_DISPLAY_RADIO_MESSAGE_SIZE     (_VV_DISPLAY_RADIO_NEW_VAL + sizeof(float))

void vv_display_parse_incoming_buffer(size_t *length, uint8_t *buffer) {
    if (*length != _VV_DISPLAY_RADIO_MESSAGE_SIZE) { //BUFFER_TYPE ADDRESS DATA_TYPE NEW_VALUE
	return;
    }
    uint8_t data_type_index = buffer[_VV_DISPLAY_RADIO_DATA_TYPE_INDEX];
    float new_val;
    memcpy(&new_val, buffer + _VV_DISPLAY_RADIO_NEW_VAL, sizeof(float));
    _vv_display_push_new_value(data_type_index, new_val);	    

    vv_lcd_page_render();
}

void vv_display_send_update(uint64_t *device_address, uint8_t data_type_index, float *new_val) {
    uint8_t buffer[_VV_DISPLAY_RADIO_MESSAGE_SIZE];    
    buffer[_VV_DISPLAY_RADIO_TYPE] = RADIO_VV_DISPLAY;
    memcpy(buffer + _VV_DISPLAY_RADIO_ADDRESS, device_address, sizeof(uint64_t));    
    buffer[_VV_DISPLAY_RADIO_DATA_TYPE_INDEX] = data_type_index;
    memcpy(buffer + _VV_DISPLAY_RADIO_NEW_VAL, new_val, sizeof(*new_val));    

    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

