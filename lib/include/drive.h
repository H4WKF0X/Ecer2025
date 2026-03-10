#ifndef DRIVE_H
#define DRIVE_H

#include "config_parser.h"

#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

void drive_init(RobotConfig *config);

// Basic Sensor Wrappers 
int is_left_black();
int is_right_black();

// Accalaration Movement
void drive_ramped(int left_target, int right_target, int instant);
void stop_driving();

void update_ramping();

// Orientation Movement
void drive_until_black(int speed);
void drive_until_white(int speed);

//Rotation
void turn_manual(int left_speed, int right_speed, int duration_ms);
void turn_until_line(int speed, int direction);

void line_follow_until_cross(int base_speed, int turn_speed);

#endif