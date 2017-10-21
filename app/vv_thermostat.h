#ifndef VV_THERMOSTAT_H
#define VV_THERMOSTAT_H

#include <bcl.h>

#define VV_THERMOSTAT_TASK_DELAY 10000

struct vv_thermostat_self {
    bc_scheduler_task_id_t vv_thermostat_task_id; 
    bc_module_relay_t* relay;
    float actual_value;
    float reference_value;
} vv_thermostat;

void vv_thermostat_init();
void vv_thermostat_set_actual_value(struct vv_thermostat_self *self, float* new_actual_value);
float vv_thermostat_get_reference_value(struct vv_thermostat_self *self);
void vv_thermostat_set_reference_value(struct vv_thermostat_self *self, float* new_reference_value);


#endif
