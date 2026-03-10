#include <kipr/wombat.h>
#include "drive.h"

// Pointer to the configuration
static RobotConfig *cfg;

void drive_init(RobotConfig *config) {
    cfg = config;
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

// Basic Movement
void drive(int left_speed, int right_speed) {
    // Safety clamp to prevent breaking motors
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;

    motor(cfg->left_motor, left_speed);
    motor(cfg->right_motor, right_speed);
}

void stop_driving() {
    drive(0, 0);
}

// Orientation Movement
void drive_until_black(int speed) {
    drive(speed, speed);
    // Keep driving while AT LEAST ONE sensor is white
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

// Rotation
void turn_manual(int left_speed, int right_speed, int duration_ms) {
    drive(left_speed, right_speed);
    msleep(duration_ms);
    stop_driving();
}

// TODO: look over  
void turn_until_line(int speed, int direction) {
    int l_black = is_left_black();
    int r_black = is_right_black();

    if (l_black && r_black) {
        return 0; // FAIL
    }

    if (l_black && !r_black) {
        drive(-speed, speed); // Rotate left
        while(is_left_black()) { msleep(10); }
        stop_driving();
    } else if (r_black && !l_black) {
        drive(speed, -speed); // Rotate right
        while(is_right_black()) { msleep(10); }
        stop_driving();
    }

    int wiggle_timeout;

    drive(speed, -speed);
    wiggle_timeout = 0;
    while(!is_left_black() && wiggle_timeout < 50) { 
        msleep(10); wiggle_timeout++; 
    }
    if (wiggle_timeout >= 50) { stop_driving(); return 0; }

    drive(-speed, speed);
    wiggle_timeout = 0;
    while(!is_right_black() && wiggle_timeout < 100) { 
        msleep(10); wiggle_timeout++; 
    }
    if (wiggle_timeout >= 100) { stop_driving(); return 0; }

    drive(speed, -speed);
    while(is_right_black()) { msleep(10); }
    stop_driving();
    msleep(50);

    int l_pwr = (direction == 1) ? speed : -speed;
    int r_pwr = (direction == 1) ? -speed : speed;
    drive(l_pwr, r_pwr);


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
            drive(base_speed - turn_speed, base_speed + turn_speed);
        } 
        // Drifting left, correct right
        else if (r_black && !l_black) {    
            drive(base_speed + turn_speed, base_speed - turn_speed);
        } 
        else {
            // both white
            drive(base_speed, base_speed);
        }
        
        msleep(10);
    }
    stop_driving();
}