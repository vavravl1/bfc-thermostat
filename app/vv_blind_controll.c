
#include <bc_module_power.h>

#include "vv_blind_controll.h"

void vv_blind_move() {
    switch(vv_blind_controller_self.state) {
	case GOING_UP:
	    bc_module_power_relay_set_state(true);
	    vv_blind_controller_self.state = STOPPED_AFTER_GOING_UP;
	    break;
	case STOPPED_AFTER_GOING_UP: 
	    // Not yet implemented
	    vv_blind_controller_self.state = GOING_DOWN;
	    break;
	case GOING_DOWN: 
	    bc_module_power_relay_set_state(false);
	    vv_blind_controller_self.state = STOPPED_AFTER_GOING_DOWN;
	    break;
	case STOPPED_AFTER_GOING_DOWN: 
	    // Not yet implemented
	    vv_blind_controller_self.state = GOING_UP;
	    break;
	default: break;
    }
}

void vv_blind_go_up() {
	vv_blind_controller_self.state = GOING_UP;
	vv_blind_move();
}
void vv_blind_go_down() {
	vv_blind_controller_self.state = GOING_DOWN;
	vv_blind_move();
}
