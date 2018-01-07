
#include "vv_radio.h"

#include <bc_common.h>
#include <bcl.h>

void vv_radio_send_update(struct vv_radio_single_float_packet *source) {
    static uint8_t buffer[VV_RADIO_MESSAGE_SIZE];    
    buffer[VV_RADIO_TYPE] = VV_RADIO_SINGLE_FLOAT;
    memcpy(buffer + VV_RADIO_ADDRESS, &source -> device_address, sizeof(uint64_t));    
    buffer[VV_RADIO_DATA_TYPE] = source -> type;
    memcpy(buffer + VV_RADIO_VALUE, &source -> value, sizeof(float));    

    bc_radio_pub_buffer(buffer, sizeof(buffer));
}


void vv_radio_parse_incoming_string_buffer(size_t length, uint8_t *buffer, struct vv_radio_string_string_packet *target) {
    if (length != VV_RADIO_STRING_MESSAGE_SIZE) { 
	return;
    }

    memcpy(& target -> device_address, buffer + VV_RADIO_STRING_ADDRESS, sizeof(uint64_t));
    strncpy(target -> key, (char*)(buffer + VV_RADIO_STRING_KEY), VV_RADIO_STRING_KEY_SIZE);
    strncpy(target -> value, (char*)(buffer + VV_RADIO_STRING_VALUE), VV_RADIO_STRING_VALUE_SIZE);
}

void vv_radio_send_string(struct vv_radio_string_string_packet *source) {
    static uint8_t buffer[VV_RADIO_STRING_MESSAGE_SIZE];    
    buffer[VV_RADIO_STRING_TYPE] = VV_RADIO_STRING_STRING;
    memcpy(buffer + VV_RADIO_ADDRESS, &source -> device_address, sizeof(uint64_t));    

    strncpy((char*)(buffer + VV_RADIO_STRING_KEY), source -> key, VV_RADIO_STRING_KEY_SIZE);
    strncpy((char*)(buffer + VV_RADIO_STRING_VALUE), source -> value, VV_RADIO_STRING_VALUE_SIZE);

    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

