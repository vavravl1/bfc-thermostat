
#ifndef VV_RADIO_THERMOSTAT_H
#define VV_RADIO_THERMOSTAT_H

#define VV_RADIO_THERMOSTAT_POWER_SAVE (false)

#include "vv_radio.h"

void vv_radio_listening_init();
void process_incoming_packet(struct vv_radio_string_string_packet *packet);

#endif


