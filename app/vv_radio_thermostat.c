
#include "vv_radio_thermostat.h"
#include "vv_display.h"
#include "vv_blind_controll.h"

void _vv_radio_listen_task();
bc_scheduler_task_id_t _vv_radio_listen_task_id;

void _vv_radio_sleep_task();
bc_scheduler_task_id_t _vv_radio_sleep_task_id;

void process_incoming_packet(struct vv_radio_string_string_packet *packet) {
    if(strncmp(packet->key, "fve", 3) == 0) {
        vv_display_push_new_value(VV_DATA_TYPE_FVE_POWER, atof(packet -> value));
    } else if(strncmp(packet->key, "power", 5) == 0) {
        vv_display_push_new_value(VV_DATA_TYPE_L1_POWER, atof(packet -> value));
    } else if(strncmp(packet->key, "liv-room", 8) == 0) {
        vv_display_push_new_value(VV_DATA_TYPE_TEMPERATURE_LIVING_ROOM, atof(packet -> value));
    } else if(strncmp(packet->key, "terrace", 7) == 0) {
        vv_display_push_new_value(VV_DATA_TYPE_TEMPERATURE_TERRACE, atof(packet -> value));
    } else if(strncmp(packet->key, "bedroom", 7) == 0) {
        vv_display_push_new_value(VV_DATA_TYPE_TEMPERATURE_BEDROOM, atof(packet -> value));
    } else if(strncmp(packet->key, "co2", 3) == 0) {
        vv_display_push_new_value(VV_DATA_TYPE_CO2, atof(packet -> value));
    } else if(strncmp(packet->key, "thermo", 6) == 0) {
    } else if(strncmp(packet->key, "blind", 8) == 0) {
        if(strncmp(packet -> value, "up", 2) == 0) {
            vv_blind_go_up();
        } else if(strncmp(packet -> value, "down", 4) == 0) {
            vv_blind_go_down();
        }
    }

    vv_display_render();
}

void vv_radio_listening_init() {
#if !VV_RADIO_THERMOSTAT_POWER_SAVE
    bc_radio_listen();
#endif
    _vv_radio_listen_task_id = bc_scheduler_register(_vv_radio_listen_task, NULL, 0);
    _vv_radio_sleep_task_id = bc_scheduler_register(_vv_radio_sleep_task, NULL, BC_TICK_INFINITY);
}

void _vv_radio_listen_task() {
#if VV_RADIO_THERMOSTAT_POWER_SAVE
    bc_radio_listen();
#endif
    uint16_t one = 1;
    bc_radio_pub_push_button(&one);
    bc_scheduler_plan_relative(_vv_radio_sleep_task_id, 1000);    
}

void _vv_radio_sleep_task() {
#if VV_RADIO_THERMOSTAT_POWER_SAVE
    bc_radio_sleep();
#endif
    bc_scheduler_plan_relative(_vv_radio_listen_task_id, 10000);    
}
