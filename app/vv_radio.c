
#include "vv_radio.h"
#include "vv_display.h"
#include "vv_thermostat.h"

#include <bc_common.h>
#include <bcl.h>

#include <jsmn.h>

void _vv_radio_listen_task();
bc_scheduler_task_id_t _vv_radio_listen_task_id;

void _vv_radio_sleep_task();
bc_scheduler_task_id_t _vv_radio_sleep_task_id;

struct _json_radio_t {
    jsmn_parser parser;
} _json_radio;


void vv_radio_listening_init() {
    _vv_radio_listen_task_id = bc_scheduler_register(_vv_radio_listen_task, NULL, 0);
    _vv_radio_sleep_task_id = bc_scheduler_register(_vv_radio_sleep_task, NULL, BC_TICK_INFINITY);

    jsmn_init(&_json_radio.parser);
}

void vv_radio_parse_incoming_buffer(size_t *length, uint8_t *buffer) {
    if (*length != VV_RADIO_MESSAGE_SIZE) { 
	return;
    }
    uint8_t data_type_index = buffer[VV_RADIO_DATA_TYPE_INDEX];
    float new_val;
    memcpy(&new_val, buffer + VV_RADIO_NEW_VAL, sizeof(float));

    if(data_type_index < VV_DATA_COUNT) {
	vv_display_push_new_value(data_type_index, new_val);	    
    } else if(data_type_index == VV_RADIO_DATA_TYPE_THERMOSTAT_REFERENCE_VALUE) {
	vv_thermostat_set_reference_value(&vv_thermostat, &new_val);
    }

    vv_display_render();
}

void vv_radio_send_update(uint64_t *device_address, uint8_t data_type_index, float *new_val) {
    uint8_t buffer[VV_RADIO_MESSAGE_SIZE];    
    buffer[VV_RADIO_TYPE] = VV_RADIO_THERMOSTAT;
    memcpy(buffer + VV_RADIO_ADDRESS, device_address, sizeof(uint64_t));    
    buffer[VV_RADIO_DATA_TYPE_INDEX] = data_type_index;
    memcpy(buffer + VV_RADIO_NEW_VAL, new_val, sizeof(*new_val));    

    bc_radio_pub_buffer(buffer, sizeof(buffer));
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


#define MAX_NAME_SIZE 30


static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && 
	    (int) strlen(s) == tok->end - tok->start &&
	    strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
	return 0;
    } else {
	return -1;
    }
}

static void jsoncpy(const char* json, jsmntok_t *tokens, uint8_t *i, const char* key, char* target) {
    if (jsoneq(json, tokens + *i, key) == 0) {
	target[MAX_NAME_SIZE - 1] = 0;
	int max_chars = tokens[*i + 1].end - tokens[*i + 1].start > MAX_NAME_SIZE - 1 ? MAX_NAME_SIZE - 1 : tokens[*i + 1].end - tokens[*i + 1].start;
	strncpy(target, json, max_chars);
	(*i)++;
    }
}

bool json_radio_parse_incoming_buffer(uint8_t *buffer) {
    jsmntok_t tokens[30];
    char json[128];
    memset(json, 0, 128);

    size_t json_length = buffer[JSON_RADIO_SIZE_POS];
    if(json_length > 128) {
	return false;
    }

    for(uint8_t i = 0; i < json_length; i++) {
	json[i] = (char)buffer[i + JSON_RADIO_SIZE_POS];
    }
    
    bc_log_debug("Received message, %s", json);

    uint8_t r = jsmn_parse(&_json_radio.parser, json, json_length, tokens, sizeof(tokens)/sizeof(tokens[0]));

    if (r < 1 || tokens[0].type != JSMN_OBJECT || r >= 30) {
	return false;
    }

    char name[30];
    char location[30];
    char value_as_string[30];
    for (uint8_t i = 1; i < r; i++) {
	jsoncpy(json, tokens, &i, "name", name);
	jsoncpy(json, tokens, &i, "location", location);
	jsoncpy(json, tokens, &i, "value", value_as_string);
    }

    vv_display_set_values(location, name, value_as_string);

    return true;
}
