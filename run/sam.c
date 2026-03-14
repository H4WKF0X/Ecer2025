#include <stdio.h>
#include <kipr/wombat.h>
#include <stdlib.h>

#define L_MOTOR 2
#define R_MOTOR 0
#define L_SENSOR_PORT 1
#define R_SENSOR_PORT 0

#define L_THRESH 3440
#define R_THRESH 1835
#define LBLACK_IS_LOW 0
#define RBLACK_IS_LOW 0

#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

#define FAST_FWD           100
#define HARD_TURN          80
#define HARD_TURN_BACK     50

static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;

/** 
 * This is the code running on the simple bot, that was pushing items off the black tape.
 * Refer to the lib folder for more information on functions
 */

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
    
    motor(L_MOTOR, cur_l - 1);
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

void drive(int left_speed, int right_speed) {
    // Safety clamp to prevent breaking motors
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;

    motor(L_MOTOR, left_speed);
    motor(R_MOTOR, right_speed);
}

void drive_until_black(int speed) {
    drive_ramped(speed, speed, 0);
    // Keep driving while AT LEAST ONE sensor is white
    while (!is_left_black() || !is_right_black()) {
        update_ramping();
        msleep(RAMP_INTERVAL_MS);
        msleep(10);
    }
    //stop_driving(0);
}

void drive_until_white(int speed) {
    drive_ramped(speed, speed, 0);
    // Keep driving while AT LEAST ONE sensor is black
    while (is_left_black() || is_right_black()) {
        update_ramping();
        msleep(RAMP_INTERVAL_MS);
        
    }
    //stop_driving(0);
}

void turn_manual(int left_speed, int right_speed, int duration_ms) {
    drive_ramped(left_speed, right_speed, 0);
    int current_duration_ms = 0;

    while (current_duration_ms < duration_ms/2) {
        update_ramping();
        current_duration_ms += RAMP_INTERVAL_MS;
        msleep(RAMP_INTERVAL_MS);
    }

    drive_ramped(0, 0, 0);

    while (current_duration_ms < duration_ms) {
        update_ramping();
        current_duration_ms += RAMP_INTERVAL_MS;
        msleep(RAMP_INTERVAL_MS);
    }

    stop_driving(1);
}

/* works extremly well on front mounted sensors, 
but needs a more robust implementation */

void followLine() {
    int elapsed = 0;
    int currentIntersection = 0;
    static int isAtFinalIntersection = 0;

    while (1) {
        int L_black = is_left_black();
        int R_black = is_right_black();

        if (!L_black && !R_black) {
            drive(FAST_FWD, FAST_FWD);

        } else if (L_black && !R_black) {
            drive(HARD_TURN_BACK, +HARD_TURN);

        } else if (R_black && !L_black) {
            drive(+HARD_TURN, HARD_TURN_BACK);

        } else {
            drive(0,0);

            if (currentIntersection < 2) {
                currentIntersection++;
            } else if (currentIntersection == 2){
                isAtFinalIntersection++;
                currentIntersection++;
            }

            drive(FAST_FWD, FAST_FWD);
            msleep(375);
        }
        
        if (isAtFinalIntersection == 1) {
            elapsed++;
            if (elapsed >= 75) {
                return;
            }
        }
        
        msleep(10);
    }
    
}

int main(void) {   
    drive_until_black(80);
    printf("I saw black");
    drive_until_white(80);
    printf("I saw white");
    drive_until_black(80);
    printf("I saw black");

    drive_ramped(20, 20, 10);
    msleep(1810);
    stop_driving(0);

    turn_manual(80,-80, 1850);
    
    followLine();

    turn_manual(-80,80,1100);

}