#ifndef VV_BLIND_CONTROLL_H
#define VV_BLIND_CONTROLL_H

static const bc_gpio_channel_t BLIND_POWER_RELAY_GPIO = BC_GPIO_P8;
static const int BLIND_STOP_TIMEOUT = 30000;
struct {
    bc_scheduler_task_id_t task;
    enum {
        GOING_UP,
        STOPPED_AFTER_GOING_UP,
        GOING_DOWN,
        STOPPED_AFTER_GOING_DOWN,
    } state;

} vv_blind_controller;
void vv_blind_init();
void vv_blind_move();
void vv_blind_go_up();
void vv_blind_go_down();


#endif

