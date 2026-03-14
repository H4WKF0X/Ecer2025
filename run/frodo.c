#include <stdio.h>
#include <kipr/wombat.h>
#include <stdlib.h>

#define L_MOTOR 2
#define R_MOTOR 0
#define L_SENSOR_PORT 1
#define R_SENSOR_PORT 0
#define L_THRESH 3419
#define R_THRESH 3719
#define LBLACK_IS_LOW 0
#define RBLACK_IS_LOW 0
#define SENSOR_OFFSET -1


#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;

/** 
 * This is the code running on the sero bot in the mechanical video, 
 * that was collecting pipes from the dispenser.
 * Refer to the lib folder for more information on functions
 */

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

// Rotation
void turn_manual(int left_speed, int right_speed, int duration_ms) {
    drive(left_speed, right_speed);
    msleep(duration_ms);
    stop_driving(0);
}

// Basic Movement
void drive(int left_speed, int right_speed) {
    // Safety clamp to prevent breaking motors
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;

    motor(L_MOTOR, left_speed);
    motor(R_MOTOR, right_speed);
}

int main(void) {   
    servo_enable_all();
	servo_set(0, 1217, 0, 1217); 
    // Test sequence
    drive_until_black(80);
    //msleep(500); // Small pause to observe behavior
	printf("I saw black");
    drive_until_white(80);
    //msleep(500);
	printf("I saw white");
    drive_until_black(80);
    //msleep(500);
    printf("I saw black");
    
    drive_ramped(20, 20, 10);
    msleep(1610);
    stop_driving(0);
    
    servo_move_smooth(0, 100, 15, 0, 1217);
    msleep(1000);
    servo_disable_all();
	ao();
    
	msleep(35000);
    for(int i = 0; i < 3; i++) {
        motor(1, 50);
        msleep(500);
        ao();
        msleep(250);
    }
    msleep(7000);
    motor(1, 50);
    msleep(500);
    ao();
    
    servo_enable_all();
    servo_move_smooth(0, 630, 15, 104, 1217);
    msleep(1000);
    servo_disable_all();
	msleep(850);
   	drive_until_black(-80);
    drive_until_white(-80);
    msleep(1200);
    stop_driving(0);
    ao();
    motor(1, 50);
    msleep(5950);
    ao();
    
    servo_enable_all();
	servo_set(0, 1217, 0, 1217); 
    
    return 0;
}