
#include "vv_radio_thermostat.h"
#include "vv_radio.h"
#include "vv_display.h"
#include "vv_thermostat.h"

#include <bc_common.h>
#include <bcl.h>

void _vv_radio_listen_task();
bc_scheduler_task_id_t _vv_radio_listen_task_id;

void _vv_radio_sleep_task();
bc_scheduler_task_id_t _vv_radio_sleep_task_id;

void process_incoming_packet(struct vv_radio_single_float_packet *packet) {
    if(packet->type < VV_DATA_COUNT) {
	vv_display_push_new_value(packet->type, packet -> value);	    
    } else if(packet->type == VV_RADIO_DATA_TYPE_THERMOSTAT_REFERENCE_VALUE) {
	vv_thermostat_set_reference_value(&vv_thermostat, &packet -> value);
    }

    vv_display_render();
}

void vv_radio_listening_init() {
    _vv_radio_listen_task_id = bc_scheduler_register(_vv_radio_listen_task, NULL, 0);
    _vv_radio_sleep_task_id = bc_scheduler_register(_vv_radio_sleep_task, NULL, BC_TICK_INFINITY);
}

void _vv_radio_listen_task() {
    bc_radio_listen();
    uint16_t one = 1;
    bc_radio_pub_push_button(&one);
    bc_scheduler_plan_relative(_vv_radio_sleep_task_id, 1000);    
}

void _vv_radio_sleep_task() {
    bc_radio_sleep();    
    bc_scheduler_plan_relative(_vv_radio_listen_task_id, 10000);    
}
