#include <stdio.h>
#include <kipr/wombat.h>
#include <stdlib.h>

#define L_MOTOR 2
#define R_MOTOR 0
#define L_SENSOR_PORT 1
#define R_SENSOR_PORT 0

#define L_THRESH 3650
#define R_THRESH 3250
#define LBLACK_IS_LOW 0
#define RBLACK_IS_LOW 0

#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;

// Line Sensor Wrappers
int is_left_black() {
    int val = analog(L_SENSOR_PORT);
    return LBLACK_IS_LOW ? (val < L_THRESH) : (val > L_THRESH);
}

int is_right_black() {
    int val = analog(R_SENSOR_PORT);
    return RBLACK_IS_LOW ? (val < R_THRESH) : (val > R_THRESH);
}

static int approach(int current, int target, int step) {
    if (current < target) {
        current += step;
        if (current > target) current = target;
    } else if (current > target) {
        current -= step;
        if (current < target) current = target;
    }
    return current;
}

void update_ramping() {
    cur_l = approach(cur_l, tar_l, ACCEL_STEP);
    cur_r = approach(cur_r, tar_r, ACCEL_STEP);
    
    motor(L_MOTOR, cur_l);
    motor(R_MOTOR, cur_r);
}

void drive_ramped(int left_target, int right_target, int instant) {
    // Safety limit to protect motors
    if (left_target > 100) left_target = 100;
    if (left_target < -100) left_target = -100;
    if (right_target > 100) right_target = 100;
    if (right_target < -100) right_target = -100;

    tar_l = left_target;
    tar_r = right_target;

    if (instant) {
        cur_l = tar_l;
        cur_r = tar_r;
        motor(L_MOTOR, cur_l);
        motor(R_MOTOR, cur_r);
    }
}

void stop_driving(int instant) {
    if (instant) {
        drive_ramped(0, 0, 1);
    } else {
        drive_ramped(0, 0, 0);
        // Block until fully stopped
        while (cur_l != 0 || cur_r != 0) {
            update_ramping();
            msleep(RAMP_INTERVAL_MS);
        }  
    }    
}

void drive_until_black(int speed) {
    drive_ramped(speed, speed, 0);
    // Keep driving while AT LEAST ONE sensor is white
    while (!is_left_black() || !is_right_black()) {
        update_ramping();
        msleep(RAMP_INTERVAL_MS);
    }
    stop_driving(1);
}

void drive_until_white(int speed) {
    drive_ramped(speed, speed, 0);
    // Keep driving while AT LEAST ONE sensor is black
    while (is_left_black() || is_right_black()) {
        update_ramping();
        msleep(RAMP_INTERVAL_MS);
    }
    stop_driving(1);
}

// Basic Movement
void drive(int left_speed, int right_speed) {
    // Kommentar: Sicherheits-Begrenzung, um Motoren zu schützen
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;

    motor(L_MOTOR, left_speed);
    motor(R_MOTOR, right_speed);
}

void stop_driving() {
    drive(0, 0);
}

void drive_until_black(int speed) {
    drive(speed, speed);
    // Kommentar: Fahre weiter, solange MINDESTENS ein Sensor weiß ist
    while (!is_left_black() || !is_right_black()) {
    	msleep(10);
    }
    stop_driving();
}

void drive_until_white(int speed) {
    drive(speed, speed);
    // Keep driving while AT LEAST ONE sensor is black
    while (is_left_black() || is_right_black()) {
        msleep(10);
    }
    stop_driving();
}

int main(void) {   
    // Test sequence
    drive_until_black(90);
    //msleep(500); // Small pause to observe behavior

    drive_until_white(90);
    //msleep(500);

    drive_until_black(90);
    //msleep(500);
    
    return 0;
}