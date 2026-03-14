#include <stdio.h>
#include <kipr/wombat.h>
#include <stdlib.h>

#define L_MOTOR 2
#define R_MOTOR 0
#define L_SENSOR_PORT 1
#define R_SENSOR_PORT 0
#define LBLACK_IS_LOW 0
#define RBLACK_IS_LOW 0

#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

/*
 * This file was used during the making of the mechnical video, specifically for showing
 * paddle wheel and lever arm movement. Can also be used for testing the servo in general. 
 * Refer to the lib folder for more information on functions
 */
static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;

static int clamp_servo_position(int position, int min_position, int max_position) {
    if (position < min_position) return min_position;
    if (position > max_position) return max_position;
    return position;
}


void servo_enable_all() {
    enable_servos();
}

void servo_disable_all() {
    disable_servos();
}


void servo_set(int port, int position, int min_position, int max_position) {
    set_servo_position(port, clamp_servo_position(position, min_position, max_position));
    msleep(300);
}


void servo_move_smooth(int port, int target, int step_delay_ms, int min_position, int max_position) {
    target = clamp_servo_position(target, min_position, max_position);

    int current = get_servo_position(port);
    if (current < 0) current = target;
    current = clamp_servo_position(current, min_position, max_position);

    if (current < target) {
        while (current < target) {
            current += 5;
            if (current > target) current = target;
            set_servo_position(port, current);
            msleep(step_delay_ms);
        }
    } else {
        while (current > target) {
            current -= 10;
            if (current < target) current = target;
            set_servo_position(port, current);
            msleep(step_delay_ms);
        }
    }
}


int main(void) {   
    ao();
    
	msleep(3000);
    for(int i = 0; i < 7; i++) {
        motor(1, 50);
        msleep(500);
        ao();
        msleep(250);
    }

    ao();
    motor(1, 50);
    msleep(6000);
    ao();
    
    servo_enable_all();
	
    
    return 0;
}