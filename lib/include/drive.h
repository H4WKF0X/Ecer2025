#ifndef DRIVE_H
#define DRIVE_H

#include "config_parser.h"

#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

void drive_init(RobotConfig *config);

// Basic Sensor Wrappers 
int is_left_black();
int is_right_black();

// Movement
void drive(int left_speed, int right_speed);
void drive_ramped(int left_target, int right_target, int instant);
void stop_driving(int use_ramping);
void update_ramping();

// Orientation Movement
void drive_until_black(int speed, int use_ramping);
void drive_until_white(int speed, int use_ramping);

//Rotation
void turn_manual(int left_speed, int right_speed, int duration_ms, int use_ramping, int stop_at_end);
int turn_until_line(int speed, int direction, int use_ramping);

//Modular Line Following
// target_intersections: How many cross-lines to pass before returning
void follow_line(int fwd_speed, int fwd_turn, int back_turn, int target_intersections);

#endif