
#include "vv_night_light.h"

#include <bc_common.h>
#include <bcl.h>

void _vv_light_task();


static uint32_t _bc_module_power_led_strip_dma_buffer[LED_STRIP_COUNT * LED_STRIP_TYPE * 2];
const bc_led_strip_buffer_t led_strip_buffer = {
    .type = LED_STRIP_TYPE,
    .count = LED_STRIP_COUNT,
    .buffer = _bc_module_power_led_strip_dma_buffer
};

void vv_night_light_init(bc_tag_lux_meter_t *_lux_meter) {
    bc_module_power_init();

    bc_led_strip_init(&vv_night_light.led_strip, bc_module_power_get_led_strip_driver(), &led_strip_buffer);

    vv_night_light.light_task_id = bc_scheduler_register(_vv_light_task, NULL, 0); 

    vv_night_light.lux_meter = _lux_meter;

//    bc_led_strip_effect_test(&vv_night_light.led_strip);


//    uint32_t color = 0;
//    color |= ((uint32_t)200) << 24;
//    color |= ((uint32_t)200) << 16;
//    color |= ((uint32_t)200) << 8;
//    color |= ((uint32_t)200);
//
//    bc_led_strip_fill(&vv_night_light.led_strip, color); 
//    bc_led_strip_write(&vv_night_light.led_strip);
}


void _vv_light_task() {

    float illuminance;

    if (bc_tag_lux_meter_get_illuminance_lux(vv_night_light.lux_meter, &illuminance)) {

	uint32_t color = 0;
	if(illuminance > 50) {
	    color |= ((uint32_t)0) << 24; //r 
	    color |= ((uint32_t)0) << 16; //g
	    color |= ((uint32_t)0) << 8;  //b
	    color |= ((uint32_t)255);       //w
	}

	bc_led_strip_fill(&vv_night_light.led_strip, color);    
	bc_led_strip_set_brightness(&vv_night_light.led_strip, 10);    
	bc_led_strip_write(&vv_night_light.led_strip);
    }     

    bc_scheduler_plan_relative(vv_night_light.light_task_id, 100);
}
