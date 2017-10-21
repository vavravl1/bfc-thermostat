#include <application.h>
#include <radio.h>

#include "vv_radio.h"
#include "vv_display.h"
#include "vv_thermostat.h"

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.1f
#define TEMPERATURE_TAG_UPDATE_INTERVAL (30 * 1000)

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define HUMIDITY_TAG_PUB_VALUE_CHANGE 1.0f
#define HUMIDITY_TAG_UPDATE_INTERVAL (1 * 60 * 1000)

#define LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define LUX_METER_TAG_PUB_VALUE_CHANGE 5.0f
#define LUX_METER_TAG_UPDATE_INTERVAL (1 * 60 * 1000)

#define BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define BAROMETER_TAG_PUB_VALUE_CHANGE 10.0f
#define BAROMETER_TAG_UPDATE_INTERVAL (1 * 60 * 1000)

bc_led_t led;
bool led_state = false;

static uint64_t my_device_address;

static bc_module_relay_t relay_0_0;
static bc_module_relay_t relay_0_1;

static void radio_event_handler(bc_radio_event_t event, void *event_param);
static void _radio_pub_state(uint8_t type, bool state);
static void battery_event_handler(bc_module_battery_event_t event, void *event_param);

static void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag);
static void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag);
static void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag);
static void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag);

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param);
void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param);
void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param);
void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param);
void encoder_event_handler(bc_module_encoder_event_t event, void *event_param);
static void _radio_pub_u16(uint8_t type, uint16_t value);

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_radio_init();

    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    //----------------------------

    static temperature_tag_t temperature_tag_0_0;
    temperature_tag_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &temperature_tag_0_0);

    static temperature_tag_t temperature_tag_0_1;
    temperature_tag_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE, &temperature_tag_0_1);

    static temperature_tag_t temperature_tag_1_0;
    temperature_tag_init(BC_I2C_I2C1, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &temperature_tag_1_0);

    static temperature_tag_t temperature_tag_1_1;
    temperature_tag_init(BC_I2C_I2C1, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE, &temperature_tag_1_1);

    //----------------------------

    static humidity_tag_t humidity_tag_0_0;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C0, &humidity_tag_0_0);

    static humidity_tag_t humidity_tag_0_2;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C0, &humidity_tag_0_2);

    static humidity_tag_t humidity_tag_0_4;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C0, &humidity_tag_0_4);

    static humidity_tag_t humidity_tag_1_0;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C1, &humidity_tag_1_0);

    static humidity_tag_t humidity_tag_1_2;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C1, &humidity_tag_1_2);

    static humidity_tag_t humidity_tag_1_4;
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C1, &humidity_tag_1_4);

    //----------------------------

    static lux_meter_tag_t lux_meter_0_0;
    lux_meter_tag_init(BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux_meter_0_0);

    static lux_meter_tag_t lux_meter_0_1;
    lux_meter_tag_init(BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE, &lux_meter_0_1);

    static lux_meter_tag_t lux_meter_1_0;
    lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux_meter_1_0);

    static lux_meter_tag_t lux_meter_1_1;
    lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE, &lux_meter_1_1);

    //----------------------------

    static barometer_tag_t barometer_tag_0_0;
    barometer_tag_init(BC_I2C_I2C0, &barometer_tag_0_0);

    static barometer_tag_t barometer_tag_1_0;
    barometer_tag_init(BC_I2C_I2C1, &barometer_tag_1_0);

    //----------------------------

    bc_module_lcd_init(&_bc_module_lcd_framebuffer);

    static bc_button_t lcd_left;
    bc_button_init_virtual(&lcd_left, BC_MODULE_LCD_BUTTON_LEFT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_left, lcd_button_event_handler, NULL);

    static bc_button_t lcd_right;
    bc_button_init_virtual(&lcd_right, BC_MODULE_LCD_BUTTON_RIGHT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_right, lcd_button_event_handler, NULL);

    //----------------------------
    
    bc_radio_set_event_handler(radio_event_handler, NULL);

    //----------------------------

    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);    

    //----------------------------
    
    bc_module_relay_init(&relay_0_0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_module_relay_init(&relay_0_1, BC_MODULE_RELAY_I2C_ADDRESS_ALTERNATE);

    //----------------------------
    
    bc_radio_pub_info(FIRMWARE);

    //----------------------------
    
    bc_module_encoder_init();
    bc_module_encoder_set_event_handler(encoder_event_handler, NULL);

    vv_radio_listening_init();
    vv_thermostat_init(&relay_0_0);
    vv_display_init(&vv_thermostat);
}

void application_task(void)
{
    vv_display_render();
}

static void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.number = (i2c_channel << 7) | i2c_address;

    bc_tag_temperature_init(&tag->self, i2c_channel, i2c_address);

    bc_tag_temperature_set_update_interval(&tag->self, TEMPERATURE_TAG_UPDATE_INTERVAL);

    bc_tag_temperature_set_event_handler(&tag->self, temperature_tag_event_handler, &tag->param);
}

static void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag)
{
    uint8_t address;

    memset(tag, 0, sizeof(*tag));

    if (revision == BC_TAG_HUMIDITY_REVISION_R1)
    {
        address = 0x5f;
    }
    else if (revision == BC_TAG_HUMIDITY_REVISION_R2)
    {
        address = 0x40;
    }
    else if (revision == BC_TAG_HUMIDITY_REVISION_R3)
    {
        address = 0x40 | 0x0f; // 0x0f - hack
    }
    else
    {
        return;
    }

    tag->param.number = (i2c_channel << 7) | address;

    bc_tag_humidity_init(&tag->self, revision, i2c_channel, BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);

    bc_tag_humidity_set_update_interval(&tag->self, HUMIDITY_TAG_UPDATE_INTERVAL);

    bc_tag_humidity_set_event_handler(&tag->self, humidity_tag_event_handler, &tag->param);
}

static void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.number = (i2c_channel << 7) | i2c_address;

    bc_tag_lux_meter_init(&tag->self, i2c_channel, i2c_address);

    bc_tag_lux_meter_set_update_interval(&tag->self, LUX_METER_TAG_UPDATE_INTERVAL);

    bc_tag_lux_meter_set_event_handler(&tag->self, lux_meter_event_handler, &tag->param);
}

static void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.number = (i2c_channel << 7) | 0x60;

    bc_tag_barometer_init(&tag->self, i2c_channel);

    bc_tag_barometer_set_update_interval(&tag->self, BAROMETER_TAG_UPDATE_INTERVAL);

    bc_tag_barometer_set_event_handler(&tag->self, barometer_tag_event_handler, &tag->param);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();

        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        bc_led_pulse(&led, 1000);
    }
}

void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) event_param;

    if (event != BC_BUTTON_EVENT_CLICK)
    {
        return;
    }

    if (self->_channel.virtual_channel == BC_MODULE_LCD_BUTTON_LEFT) {
	vv_display_prev_page();
    } else if(self->_channel.virtual_channel == BC_MODULE_LCD_BUTTON_RIGHT) {
	vv_display_next_page();	
    } else {
	return;
    }

//        static uint16_t left_event_count = 0;
//        _radio_pub_u16(RADIO_LCD_BUTTON_LEFT, left_event_count++);
    bc_scheduler_plan_now(0);
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_temperature_get_temperature_celsius(self, &value))
    {
	vv_thermostat_set_actual_value(&vv_thermostat, &value);
        if ((fabs(value - param->value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_thermometer(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;

            bc_scheduler_plan_now(0);
        }
    }
}

void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_HUMIDITY_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_humidity_get_humidity_percentage(self, &value))
    {
        if ((fabs(value - param->value) >= HUMIDITY_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_humidity(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL;

            bc_scheduler_plan_now(0);
        }
    }
}

void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_LUX_METER_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_lux_meter_get_illuminance_lux(self, &value))
    {
        if ((fabs(value - param->value) >= LUX_METER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_luminosity(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL;

            bc_scheduler_plan_now(0);
        }
    }
}

void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param)
{
    float pascal;
    float meter;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_BAROMETER_EVENT_UPDATE)
    {
        return;
    }

    if (!bc_tag_barometer_get_pressure_pascal(self, &pascal))
    {
        return;
    }

    if ((fabs(pascal - param->value) >= BAROMETER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
    {

        if (!bc_tag_barometer_get_altitude_meter(self, &meter))
        {
            return;
        }

        bc_radio_pub_barometer(param->number, &pascal, &meter);
        param->value = pascal;
        param->next_pub = bc_scheduler_get_spin_tick() + BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL;

        bc_scheduler_plan_now(0);
    }
}


static void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_DETACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_INIT_DONE)
    {
        my_device_address = bc_radio_get_device_address();
    }
}

void bc_radio_on_buffer(uint64_t *peer_device_address, uint8_t *buffer, size_t *length)
{
    (void) peer_device_address;
    if (*length < (1 + sizeof(uint64_t)))
    {
        return;
    }

    uint64_t device_address;
    uint8_t *pointer = buffer + sizeof(uint64_t) + 1;

    memcpy(&device_address, buffer + 1, sizeof(device_address));

    if (device_address != my_device_address)
    {
        return;
    }

    switch (buffer[0]) {
        case RADIO_LED_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }
            led_state = buffer[sizeof(uint64_t) + 1];
            bc_led_set_mode(&led, led_state ? BC_LED_MODE_ON : BC_LED_MODE_OFF);
            _radio_pub_state(RADIO_LED, led_state);
            break;
        }
        case RADIO_RELAY_0_SET:
        case RADIO_RELAY_1_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }
            bc_module_relay_set_state(buffer[0] == RADIO_RELAY_0_SET ? &relay_0_0 : &relay_0_1, buffer[sizeof(uint64_t) + 1]);
            _radio_pub_state(buffer[0] == RADIO_RELAY_0_SET ? RADIO_RELAY_0 : RADIO_RELAY_1, buffer[sizeof(uint64_t) + 1]);
            break;
        }
        case RADIO_RELAY_0_PULSE_SET:
        case RADIO_RELAY_1_PULSE_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1 + 4))
            {
                return;
            }
            uint32_t duration; // Duration is 4 byte long in a radio packet, but 8 bytes as a bc_relay_pulse parameter.
            memcpy(&duration, &buffer[sizeof(uint64_t) + 2], sizeof(uint32_t));
            bc_module_relay_pulse(buffer[0] == RADIO_RELAY_0_PULSE_SET ? &relay_0_0 : &relay_0_1, buffer[sizeof(uint64_t) + 1], (bc_tick_t)duration);
            break;
        }
        case RADIO_RELAY_0_GET:
        case RADIO_RELAY_1_GET:
        {
            bc_module_relay_state_t state = bc_module_relay_get_state(buffer[0] == RADIO_RELAY_0_GET ? &relay_0_0 : &relay_0_1);
            if (state != BC_MODULE_RELAY_STATE_UNKNOWN)
            {
                _radio_pub_state(buffer[0] == RADIO_RELAY_0_GET ? RADIO_RELAY_0 : RADIO_RELAY_1, state == BC_MODULE_RELAY_STATE_TRUE ? true : false);
            }
            break;
        }
        case RADIO_RELAY_POWER_SET:
        {
            if (*length != (1 + sizeof(uint64_t) + 1))
            {
                return;
            }
            bc_module_power_relay_set_state(*pointer);
            _radio_pub_state(RADIO_RELAY_POWER, *pointer);
            break;
        }
        case RADIO_RELAY_POWER_GET:
        {
            _radio_pub_state(RADIO_RELAY_POWER, bc_module_power_relay_get_state());
            break;
        }
	case VV_RADIO_THERMOSTAT:
	{
	    vv_radio_parse_incoming_buffer(length, buffer);
	    break;
	}
        default:
        {
            break;
        }
    }
}

static void _radio_pub_state(uint8_t type, bool state)
{
    uint8_t buffer[2];
    buffer[0] = type;
    buffer[1] = state;
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}


void battery_event_handler(bc_module_battery_event_t event, void *event_param) {
    (void) event;
    (void) event_param;

    float voltage;

    if (bc_module_battery_get_voltage(&voltage)) {
        bc_radio_pub_battery(0, &voltage);
    }
}

static void _radio_pub_u16(uint8_t type, uint16_t value)
{
    uint8_t buffer[1 + sizeof(value)];
    buffer[0] = type;
    memcpy(buffer + 1, &value, sizeof(value));
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

void encoder_event_handler(bc_module_encoder_event_t event, void *event_param) {
    (void) event_param;

    if(event == BC_MODULE_ENCODER_EVENT_ROTATION) {
	float new_reference_value = *vv_thermostat_get_reference_value(&vv_thermostat) + bc_module_encoder_get_increment() / 10.0;
	vv_thermostat_set_reference_value(&vv_thermostat, &new_reference_value);
	vv_display_render();
    }
}

