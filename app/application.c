#include "application.h"
#include "radio.h"
#include "sensors.h"
#include "vv_radio_thermostat.h"
#include "vv_display.h"
#include "vv_blind_controll.h"

bc_led_t led;
bool led_state = false;

static uint64_t my_id;

static bc_module_relay_t relay_0_0;
static bc_module_relay_t relay_0_1;

static void radio_event_handler(bc_radio_event_t event, void *event_param);

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

void encoder_event_handler(bc_module_encoder_event_t event, void *event_param);

struct {
    volatile bool lcd_button_clicked;
    bc_scheduler_task_id_t task_id;
} main_button_clicked_detector;
void main_button_clicked_task();

void application_init(void) {
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING);

    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    //----------------------------

    bc_module_lcd_init(&_bc_module_lcd_framebuffer);

    static bc_button_t lcd_left;
    bc_button_init_virtual(&lcd_left, BC_MODULE_LCD_BUTTON_LEFT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_left, lcd_button_event_handler, NULL);

    static bc_button_t lcd_right;
    bc_button_init_virtual(&lcd_right, BC_MODULE_LCD_BUTTON_RIGHT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_right, lcd_button_event_handler, NULL);

    main_button_clicked_detector.task_id = bc_scheduler_register(main_button_clicked_task, NULL, BC_TICK_INFINITY);
    main_button_clicked_detector.lcd_button_clicked = false;

    //----------------------------
    bc_module_power_init();

    //----------------------------
    bc_radio_set_event_handler(radio_event_handler, NULL);

    //----------------------------

    bc_module_relay_init(&relay_0_0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_module_relay_init(&relay_0_1, BC_MODULE_RELAY_I2C_ADDRESS_ALTERNATE);

    //----------------------------

    bc_radio_pairing_request("Thermostat", "1.0.0");

    //----------------------------
    sensors_init_all();

    //----------------------------
    bc_module_encoder_init();
    bc_module_encoder_set_event_handler(encoder_event_handler, NULL);

    vv_radio_listening_init();
    vv_display_init();
    vv_blind_init();
}

void application_task(void) {}


void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param) {
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS) {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);
    }
}

void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param) {
    (void) event_param;

    if (event == BC_BUTTON_EVENT_HOLD) {
    } else if (event == BC_BUTTON_EVENT_CLICK) {
        if (self->_channel.virtual_channel == BC_MODULE_LCD_BUTTON_LEFT) {
            vv_display_prev_page();
        } else if (self->_channel.virtual_channel == BC_MODULE_LCD_BUTTON_RIGHT) {
            vv_display_next_page();
        } else {
            return;
        }
    }

    main_button_clicked_detector.lcd_button_clicked = true;

    bc_scheduler_plan_now(0);
}

static void radio_event_handler(bc_radio_event_t event, void *event_param) {
    (void) event_param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    if (event == BC_RADIO_EVENT_ATTACH) {
        bc_led_pulse(&led, 1000);
    } else if (event == BC_RADIO_EVENT_DETACH) {
        bc_led_pulse(&led, 1000);
    } else if (event == BC_RADIO_EVENT_INIT_DONE) {
        my_id = bc_radio_get_my_id();
    }
}

void bc_radio_pub_on_buffer(uint64_t *peer_device_address, uint8_t *buffer, size_t length) {
    switch (buffer[0]) {
        case VV_RADIO_STRING_STRING: {
            struct vv_radio_string_string_packet packet;
            if(vv_radio_parse_incoming_string_buffer(length, buffer, &packet)) {
                if (packet.device_address == my_id) {
                    process_incoming_packet(&packet);
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void encoder_event_handler(bc_module_encoder_event_t event, void *event_param) {
    (void) event_param;

    if (event == BC_MODULE_ENCODER_EVENT_ROTATION) {
        //bc_module_encoder_get_increment()
    } else if(event == BC_MODULE_ENCODER_EVENT_CLICK) {
        bc_scheduler_plan_relative(main_button_clicked_detector.task_id, 10);
    }
}

void main_button_clicked_task() {
    if(main_button_clicked_detector.lcd_button_clicked) {
        main_button_clicked_detector.lcd_button_clicked = false;
    } else {
        vv_blind_move();
    }
}

