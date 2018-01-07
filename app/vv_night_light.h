
#ifndef VV_NIGHT_LIGHT_H
#define VV_NIGHT_LIGHT_H

#include <bc_common.h>
#include <bcl.h>

struct vv_night_light_self {
   bc_scheduler_task_id_t light_task_id; 
   struct bc_led_strip_t led_strip;
   bc_tag_lux_meter_t *lux_meter;
} vv_night_light;

void vv_night_light_init(bc_tag_lux_meter_t *_lux_meter);

#endif
