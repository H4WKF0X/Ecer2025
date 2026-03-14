#include <kipr/wombat.h>
#include "drive.h"

// Pointer to the configuration
static RobotConfig *cfg;

// Current actual speeds and target speeds
static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;

/**
 * @brief Initializes the drive library with the robot's configuration.
 * @param config Pointer to the RobotConfig struct containing ports and thresholds.
 */
void drive_init(RobotConfig *config) {
    cfg = config;
    cur_l = 0; cur_r = 0;
    tar_l = 0; tar_r = 0;
}

/**
 * @brief Checks if the left sensor is currently detecting black.
 * @return 1 if black detected, 0 if white detected.
 */
int is_left_black() {
    int val = analog(cfg->left_sensor_port);
    return cfg->left_black_is_low ? (val < cfg->left_threshold) : (val > cfg->left_threshold);
}

/**
 * @brief Checks if the right sensor is currently detecting black.
 * @return 1 if black detected, 0 if white detected.
 */
int is_right_black() {
    int val = analog(cfg->right_sensor_port);
    return cfg->right_black_is_low ? (val < cfg->right_threshold) : (val > cfg->right_threshold);
}

/**
 * @brief Directly sets motor powers with speed clamping.
 * Updates current speed variables to keep ramping logic in sync.
 * @param left_speed Target power for left motor (-100 to 100).
 * @param right_speed Target power for right motor (-100 to 100).
 */
void drive(int left_speed, int right_speed) {
    if (left_speed > 100) left_speed = 100; 
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100; 
    if (right_speed < -100) right_speed = -100;

    cur_l = left_speed; cur_r = right_speed; // Keep ramping variables in sync
    motor(cfg->left_motor, left_speed);
    motor(cfg->right_motor, right_speed);
}

/**
 * @brief Internal helper to incrementally move a value toward a target.
 * @param current The current value.
 * @param target The desired value.
 * @param step The maximum increment/decrement allowed per call.
 * @return The updated value closer to the target.
 */
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

/**
 * @brief Steps the motor powers toward the target speeds using ACCEL_STEP.
 * This must be called inside a loop to achieve smooth acceleration.
 */
void update_ramping() {
    cur_l = approach(cur_l, tar_l, ACCEL_STEP);
    cur_r = approach(cur_r, tar_r, ACCEL_STEP);
    
    motor(cfg->left_motor, cur_l);
    motor(cfg->right_motor, cur_r);
}

/**
 * @brief Sets new target speeds for the ramping logic.
 * @param left_target Desired speed for left motor.
 * @param right_target Desired speed for right motor.
 * @param instant If 1, bypasses ramping and sets power immediately.
 */
void drive_ramped(int left_target, int right_target, int instant) {
    if (left_target > 100) left_target = 100; 
    if (left_target < -100) left_target = -100;
    if (right_target > 100) right_target = 100; 
    if (right_target < -100) right_target = -100;

    tar_l = left_target;
    tar_r = right_target;

    if (instant) {
        cur_l = tar_l;
        cur_r = tar_r;
        motor(cfg->left_motor, cur_l);
        motor(cfg->right_motor, cur_r);
    }
}

/**
 * @brief Stops the robot.
 * @param use_ramping If 1, performs a smooth deceleration; if 0, stops instantly.
 */
void stop_driving(int use_ramping) {
    if (!use_ramping) {
        drive(0, 0);
    } else {
        drive_ramped(0, 0, 0);
        // Block until stopped
        while (cur_l != 0 || cur_r != 0) {
            update_ramping();
            msleep(RAMP_INTERVAL_MS);
        }  
    }    
}

/**
 * @brief Drives forward until both sensors detect a black line (intersection).
 * @param speed The target cruising speed.
 * @param use_ramping If 1, uses smooth acceleration to reach target speed.
 */
void drive_until_black(int speed, int use_ramping) {
    if (use_ramping) drive_ramped(speed, speed, 0);
    else drive(speed, speed);

    // Keep driving while AT LEAST ONE sensor is white
    while (!is_left_black() || !is_right_black()) {
        if (use_ramping) update_ramping();
        msleep(RAMP_INTERVAL_MS);
    }
}

/**
 * @brief Drives forward while sensors are on black until they hit a white surface.
 * @param speed The target cruising speed.
 * @param use_ramping If 1, uses smooth acceleration to reach target speed.
 */
void drive_until_white(int speed, int use_ramping) {
    if (use_ramping) drive_ramped(speed, speed, 0);
    else drive(speed, speed);
    // Keep driving while AT LEAST ONE sensor is white
    while (is_left_black() || is_right_black()) {
        if (use_ramping) update_ramping();
        msleep(RAMP_INTERVAL_MS);
    }
}

// Rotation
/**
 * TODO: make modular, add comment
 * */
void turn_manual(int left_speed, int right_speed, int duration_ms, int use_ramping, int stop_at_end) {
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

/**
 * @brief Executes a 90/180-degree turn using a state-machine logic to "hug" the line.
 * Includes initial alignment, a centering wiggle, and line handover detection.
 * @param speed The rotation speed.
 * @param direction 1 for Right, -1 for Left.
 * @param use_ramping If 1, uses smooth acceleration.
 * @return 1 on successful line acquisition, 0 if starting on a cross or line lost.
 */
int turn_until_line(int speed, int direction, int use_ramping) {
    int l_black = is_left_black();
    int r_black = is_right_black();

    if (l_black && r_black) return 0; // FAIL

    // Alignment
    if (l_black && !r_black) {
        // Rotate left
        if (use_ramping) drive_ramped(-speed, speed, 1); 
        else drive(-speed, speed); 

        while(is_left_black()) { msleep(10); }
        stop_driving(1);
    } else if (r_black && !l_black) {
        // Rotate right
        if (use_ramping) drive_ramped(speed, -speed, 1); 
        else drive(speed, -speed);
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
    drive(l_pwr, r_pwr);


    while (!is_left_black() && !is_right_black()) { msleep(10); } 
    while (is_left_black() || is_right_black()) { msleep(10); } // Escape current line
    while (!is_left_black() && !is_right_black()) { msleep(10); } //find the line again
    while (is_left_black() || is_right_black()) { msleep(10); }

    stop_driving(1);
    return 1;
}

/**
 * @brief Modular line follower that counts intersections.
 * Uses a straddle-logic (staying between the edges of the line).
 * @param fwd_speed Base speed when both sensors see white.
 * @param fwd_turn Speed for the outside motor during correction.
 * @param back_turn Speed for the inside motor during correction.
 * @param target_intersections Number of black cross-lines to encounter before returning.
 */
void follow_line(int fwd_speed, int fwd_turn, int back_turn, int target_intersections) {
    int intersections_crossed = 0;
    int currently_on_intersection = 0; // Prevents counting the same line multiple times

    while (intersections_crossed < target_intersections) {
        int L_black = is_left_black();
        int R_black = is_right_black();

        if (!L_black && !R_black) {
            drive(fwd_speed, fwd_speed);
            currently_on_intersection = 0;
        } 
        else if (L_black && !R_black) {
            drive(back_turn, fwd_turn);
            currently_on_intersection = 0;
        } 
        else if (R_black && !L_black) {
            drive(fwd_turn, back_turn);
            currently_on_intersection = 0;
        } 
        else {
            // Both black (Intersection)
            if (!currently_on_intersection) {
                intersections_crossed++;
                currently_on_intersection = 1;
            }
            
            drive(fwd_speed, fwd_speed); 
        }
        
        msleep(10);
    }
    // Note: returns WITHOUT stopping! 
}