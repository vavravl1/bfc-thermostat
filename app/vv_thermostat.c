
#include "vv_thermostat.h"
#include "vv_display.h"
#include <bcl.h>
#include <radio.h>

void _vv_thermostat_task();
void _radio_pub_relay_state(struct vv_thermostat_self *self);
void _vv_thermostat_set_actual_state_internal(struct vv_thermostat_self *self, bool new_state);

void vv_thermostat_init(bc_module_relay_t* __relay) {
    vv_thermostat.relay = __relay;
    vv_thermostat.actual_value = 0;
    vv_thermostat.reference_value = 20;
    vv_thermostat.vv_thermostat_task_id = bc_scheduler_register(_vv_thermostat_task, (void*)&vv_thermostat, VV_THERMOSTAT_TASK_DELAY);
    vv_thermostat.local_controll = true;
}

bool vv_thermostat_get_actual_state(struct vv_thermostat_self *self) {
    return self->actual_state;
}

void vv_thermostat_set_actual_state(struct vv_thermostat_self *self, bool new_state) {
    if(!vv_thermostat_is_local_controll(self)) {
	_vv_thermostat_set_actual_state_internal(self, new_state);
	_radio_pub_relay_state(self);
    }
}

void vv_thermostat_set_local_controll(struct vv_thermostat_self *self, bool new_state) {
    self -> local_controll = new_state;
    vv_display_render();
}

bool vv_thermostat_is_local_controll(struct vv_thermostat_self *self) {
    return self -> local_controll;
}

float* vv_thermostat_get_actual_value(struct vv_thermostat_self *self) {
    return & self->actual_value;
}

void vv_thermostat_set_actual_value(struct vv_thermostat_self *self, float* new_actual_value) {
    self->actual_value = *new_actual_value;
    vv_display_render();
}

float* vv_thermostat_get_reference_value(struct vv_thermostat_self *self) {
    return & self->reference_value;
}

void vv_thermostat_set_reference_value(struct vv_thermostat_self *self, float* new_reference_value) {
    self->reference_value = *new_reference_value;
}

void _vv_thermostat_set_actual_state_internal(struct vv_thermostat_self *self, bool new_state) {
    if(new_state != self->actual_state) {
	self->actual_state = new_state;
	bc_module_relay_set_state(self->relay, vv_thermostat_get_actual_state(self));
	vv_display_render();
	if(new_state == true) {
	    vv_display_blink_red();
	} else {
	    vv_display_blink_green();
	}
    }
}

void _vv_thermostat_task(void* param) {
    struct vv_thermostat_self *self = (struct vv_thermostat_self*)param;
    if(vv_thermostat_is_local_controll(self)) {
	if(*vv_thermostat_get_actual_value(self) < *vv_thermostat_get_reference_value(self)) {
	    _vv_thermostat_set_actual_state_internal(self, true);
	} else if(*vv_thermostat_get_actual_value(self) > (1.05 * *vv_thermostat_get_reference_value(self))) {
	    _vv_thermostat_set_actual_state_internal(self, false);
	}
    }

    _radio_pub_relay_state(self);
    bc_scheduler_plan_relative(vv_thermostat.vv_thermostat_task_id, VV_THERMOSTAT_TASK_DELAY);
}

void _radio_pub_relay_state(struct vv_thermostat_self *self) {
    uint8_t buffer[2];
    buffer[0] = RADIO_RELAY_0;
    buffer[1] = vv_thermostat_get_actual_state(self);
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}
