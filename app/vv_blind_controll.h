#ifndef VV_BLIND_CONTROLL_H
#define VV_BLIND_CONTROLL_H

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

