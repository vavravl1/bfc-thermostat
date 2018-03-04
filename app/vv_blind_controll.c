
#include <bc_module_power.h>
#include <bc_gpio.h>

#include "vv_blind_controll.h"

static void _vv_blind_controller_task();

void vv_blind_init() {
    vv_blind_controller.task = bc_scheduler_register(_vv_blind_controller_task, NULL, BC_TICK_INFINITY);
    bc_gpio_init(BLIND_POWER_RELAY_GPIO);
    bc_gpio_set_mode(BLIND_POWER_RELAY_GPIO, BC_GPIO_MODE_OUTPUT);
}

void vv_blind_move() {
    switch(vv_blind_controller.state) {
	case GOING_UP:
	    bc_module_power_relay_set_state(true);
        bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, true);
	    vv_blind_controller.state = STOPPED_AFTER_GOING_UP;
        bc_scheduler_plan_relative(vv_blind_controller.task, BLIND_STOP_TIMEOUT);
        break;
	case STOPPED_AFTER_GOING_UP: 
        bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, false);
	    vv_blind_controller.state = GOING_DOWN;
	    break;
	case GOING_DOWN:
        bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, true);
	    bc_module_power_relay_set_state(false);
	    vv_blind_controller.state = STOPPED_AFTER_GOING_DOWN;
        bc_scheduler_plan_relative(vv_blind_controller.task, BLIND_STOP_TIMEOUT);
	    break;
	case STOPPED_AFTER_GOING_DOWN: 
        bc_gpio_set_output(BLIND_POWER_RELAY_GPIO, false);
	    vv_blind_controller.state = GOING_UP;
	    break;
	default: break;
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
    switch(vv_blind_controller.state) {
        case STOPPED_AFTER_GOING_UP:
        case STOPPED_AFTER_GOING_DOWN:
            vv_blind_move();
            break;
        default: break;
    }
}