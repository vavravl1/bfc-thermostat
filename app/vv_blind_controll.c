
#include <bc_module_power.h>
#include <bc_gpio.h>

#include "vv_blind_controll.h"
#include "vv_radio.h"
#include "vv_display.h"

static void _vv_blind_controller_task();

static void _vv_blind_send_message(char *how);

void vv_blind_init() {
    vv_blind_controller.task = bc_scheduler_register(_vv_blind_controller_task, NULL, BC_TICK_INFINITY);
    bc_gpio_init(BLIND_POWER_RELAY_GPIO);
    bc_gpio_set_mode(BLIND_POWER_RELAY_GPIO, BC_GPIO_MODE_OUTPUT);
}

void vv_blind_move() {
    switch (vv_blind_controller.state) {
        case GOING_UP:
            vv_display_blink_green();
            bc_module_power_relay_set_state(true);
            bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, true);
            vv_blind_controller.state = STOPPED_AFTER_GOING_UP;
            bc_scheduler_plan_relative(vv_blind_controller.task, BLIND_STOP_TIMEOUT);
            _vv_blind_send_message("up");
            break;
        case STOPPED_AFTER_GOING_UP:
            bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, false);
            vv_blind_controller.state = GOING_DOWN;
            _vv_blind_send_message("up-stop");
            break;
        case GOING_DOWN:
            vv_display_blink_red();
            bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, true);
            bc_module_power_relay_set_state(false);
            vv_blind_controller.state = STOPPED_AFTER_GOING_DOWN;
            bc_scheduler_plan_relative(vv_blind_controller.task, BLIND_STOP_TIMEOUT);
            _vv_blind_send_message("down");
            break;
        case STOPPED_AFTER_GOING_DOWN:
            bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, false);
            vv_blind_controller.state = GOING_UP;
            _vv_blind_send_message("down-stop");
            break;
        default:
            break;
    }
}

void vv_blind_go_up() {
    vv_blind_controller.state = GOING_UP;
    vv_blind_move();
}

void vv_blind_go_down() {
    vv_blind_controller.state = GOING_DOWN;
    vv_blind_move();
}

static void _vv_blind_controller_task() {
    switch (vv_blind_controller.state) {
        case STOPPED_AFTER_GOING_UP:
        case STOPPED_AFTER_GOING_DOWN:
            vv_blind_move();
            break;
        case GOING_UP:
        case GOING_DOWN:
        default:
            break;
    }
}

static void _vv_blind_send_message(char *how) {
    struct vv_radio_string_string_packet message;
    strncpy(message.key, "move", 4);
    strncpy(message.value, how, 4);
    vv_radio_send_string(&message);
}
