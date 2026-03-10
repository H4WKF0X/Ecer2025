#include <kipr/wombat.h>
#include "drive.h"

// Pointer to the configuration
static RobotConfig *cfg;

// Current actual speeds and target speeds
static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;

void drive_init(RobotConfig *config) {
    cfg = config;

    cur_l = 0; cur_r = 0;
    tar_l = 0; tar_r = 0;
}

// Line Sensor Wrappers
int is_left_black() {
    int val = analog(cfg->left_sensor_port);
    return cfg->left_black_is_low ? (val < cfg->left_threshold) : (val > cfg->left_threshold);
}

int is_right_black() {
    int val = analog(cfg->right_sensor_port);
    return cfg->right_black_is_low ? (val < cfg->right_threshold) : (val > cfg->right_threshold);
}

// step current speed toward target
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
    
    motor(cfg->left_motor, cur_l);
    motor(cfg->right_motor, cur_r);
}

void drive_ramped(int left_target, int right_target, int instant) {
    tar_l = left_target;
    tar_r = right_target;

    if (instant) {
        cur_l = tar_l;
        cur_r = tar_r;
        motor(cfg->left_motor, cur_l);
        motor(cfg->right_motor, cur_r);
    }
}

void stop_driving(int instant) {
    if (instant) {
        drive_ramped(0,0,1);
    } else {
        drive_ramped(0, 0, 0);
        // Block until stopped
        while (cur_l != 0 || cur_r != 0) {
            update_ramping();
            msleep(RAMP_INTERVAL_MS);
        }  
    }    
}

// Orientation Movement
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
    // Keep driving while AT LEAST ONE sensor is white
    while (is_left_black() || is_right_black()) {
        update_ramping();
        msleep(RAMP_INTERVAL_MS);
    }
    stop_driving(1);
}

// Rotation
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

// TODO: look over  
void turn_until_line(int speed, int direction) {
    int l_black = is_left_black();
    int r_black = is_right_black();

    if (l_black && r_black) {
        return 0; // FAIL
    }

    if (l_black && !r_black) {
        drive_ramped(-speed, speed, 1); // Rotate left
        while(is_left_black()) { msleep(10); }
        stop_driving(1);
    } else if (r_black && !l_black) {
        drive_ramped(speed, -speed, 1); // Rotate right
        while(is_right_black()) { msleep(10); }
        stop_driving(1);
    }

    int wiggle_timeout;

    drive_ramped(speed, -speed, 1);
    wiggle_timeout = 0;
    while(!is_left_black() && wiggle_timeout < 50) { 
        msleep(10); wiggle_timeout++; 
    }
    if (wiggle_timeout >= 50) { stop_driving(1); return 0; }

    drive_ramped(-speed, speed, 1);
    wiggle_timeout = 0;
    while(!is_right_black() && wiggle_timeout < 100) { 
        msleep(10); wiggle_timeout++; 
    }
    if (wiggle_timeout >= 100) { stop_driving(1); return 0; }

    drive_ramped(speed, -speed, 1);
    while(is_right_black()) { msleep(10); }
    stop_driving(1);
    msleep(50);

    int l_pwr = (direction == 1) ? speed : -speed;
    int r_pwr = (direction == 1) ? -speed : speed;
    drive(l_pwr, r_pwr,1);


    while (!is_left_black() && !is_right_black()) { msleep(10); } 
    while (is_left_black() || is_right_black()) { msleep(10); } // Escape current line
    while (!is_left_black() && !is_right_black()) { msleep(10); } //find the line again
    while (is_left_black() || is_right_black()) { msleep(10); }
}

void line_follow_until_cross(int base_speed, int turn_speed) {
    while (1) {
        int l_black = is_left_black();
        int r_black = is_right_black();

        // Intersection hit
        if (l_black && r_black) {
            break;
        } 
        // Drifting right, correct left
        else if (l_black && !r_black) {
            drive_ramped(base_speed - turn_speed, base_speed + turn_speed, 1);
        } 
        // Drifting left, correct right
        else if (r_black && !l_black) {    
            drive_ramped(base_speed + turn_speed, base_speed - turn_speed, 1);
        } 
        else {
            // both white
            drive_ramped(base_speed, base_speed, 1);
        }
        
        msleep(10);
    }
    stop_driving(1);
}