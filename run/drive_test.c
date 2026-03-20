#include <stdio.h>
#include <kipr/wombat.h>
#include <stdlib.h>

#define L_MOTOR 2
#define R_MOTOR 0
#define L_SENSOR_PORT 1
#define R_SENSOR_PORT 0
#define L_THRESH 3559
#define R_THRESH 3725
#define LBLACK_IS_LOW 0
#define RBLACK_IS_LOW 0
#define SENSOR_OFFSET -1

#define ACCEL_STEP 2
#define RAMP_INTERVAL_MS 10

#define SERVO_SET_DELAY_MS 300
#define SERVO_SMOOTH_STEP 10

static int cur_l = 0, cur_r = 0;
static int tar_l = 0, tar_r = 0;



/**
 * @brief Checks if the left sensor is currently detecting black.
 * @return 1 if black detected, 0 if white detected.
 */
int is_left_black() {
    int val = analog(L_SENSOR_PORT);
    return LBLACK_IS_LOW ? (val < L_THRESH) : (val > L_THRESH);
}

int is_right_black() {
    int val = analog(R_SENSOR_PORT);
    return RBLACK_IS_LOW ? (val < R_THRESH) : (val > R_THRESH);
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
    motor(L_MOTOR, left_speed);
    motor(R_MOTOR, right_speed);
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
    
    motor(L_MOTOR, cur_l);
    motor(R_MOTOR, cur_r);
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
        motor(L_MOTOR, cur_l);
        motor(R_MOTOR, cur_r);
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

/**
 * @brief Internal helper to ensure a servo position stays within hardware or safety limits.
 * @param position The requested position.
 * @param min_position The minimum allowed position.
 * @param max_position The maximum allowed position.
 * @return The clamped position within [min, max].
 */
static int clamp_servo_position(int position, int min_position, int max_position) {
    if (position < min_position) return min_position;
    if (position > max_position) return max_position;
    return position;
}

/**
 * @brief Enables power to all servo ports.
 */
void servo_enable_all() {
    enable_servos();
}

/**
 * @brief Disables power to all servo ports. Useful for manual positioning or saving battery.
 */
void servo_disable_all() {
    disable_servos();
}

/**
 * @brief Sets a servo to a specific position immediately.
 * Blocks for SERVO_SET_DELAY_MS to allow the physical movement to complete.
 * @param port The servo port index.
 * @param position The target position.
 * @param min_position Safety minimum.
 * @param max_position Safety maximum.
 */
void servo_set(int port, int position, int min_position, int max_position) {
    set_servo_position(port, clamp_servo_position(position, min_position, max_position));
    msleep(SERVO_SET_DELAY_MS);
}

void servo_move_smooth(int port, int target, int step_delay_ms, int min_position, int max_position) {
    target = clamp_servo_position(target, min_position, max_position);

    int current = get_servo_position(port);
    if (current < 0) current = target;
    current = clamp_servo_position(current, min_position, max_position);

    if (current < target) {
        while (current < target) {
            current += SERVO_SMOOTH_STEP;
            if (current > target) current = target;
            set_servo_position(port, current);
            msleep(step_delay_ms);
        }
    } else if (current > target) {
        while (current > target) {
            current -= SERVO_SMOOTH_STEP;
            if (current < target) current = target;
            set_servo_position(port, current);
            msleep(step_delay_ms);
        }
    }
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
        if (use_ramping) drive_ramped(speed, -speed, 1); 
        else drive(speed, -speed); 

        while(is_left_black()) { msleep(10); }
        stop_driving(0);
    } else if (r_black && !l_black) {
        // Rotate right
        if (use_ramping) drive_ramped(-speed, speed, 1); 
        else drive(-speed, speed);
        while(is_right_black()) { msleep(10); }
        stop_driving(0);
    }

    int wiggle_timeout;

    drive_ramped(speed, -speed, 1);
    wiggle_timeout = 0;
    while(!is_left_black() && wiggle_timeout < 200) { 
        msleep(10); wiggle_timeout++; 
    }
    if (wiggle_timeout >= 50) { stop_driving(0); return 0; }

    drive_ramped(-speed, speed, 1);
    wiggle_timeout = 0;
    while(!is_right_black() && wiggle_timeout < 400) { 
        msleep(10); wiggle_timeout++; 
    }
    if (wiggle_timeout >= 200) { stop_driving(0); return 0; }

    drive_ramped(speed, -speed, 1);
    while(is_right_black()) { msleep(10); }
    stop_driving(0);
    msleep(50);

    int l_pwr = (direction == 1) ? speed : -speed;
    int r_pwr = (direction == 1) ? -speed : speed;
    drive(l_pwr, r_pwr);


    while (!is_left_black() && !is_right_black()) { msleep(5); } 
    msleep(30);
    while (is_left_black() || is_right_black()) { msleep(5); } // Escape current line
    msleep(30);
    while (!is_left_black() && !is_right_black()) { msleep(5); } //find the line again
    msleep(100);
    while (is_left_black() || is_right_black()) { msleep(5); }

    stop_driving(1);
    return 1;
}

void drive_gyro_straight(int speed, int duration_ms, int use_ramping) {
    double current_angle = 0;
    int elapsed = 0;
    double kP = 2.0; // Proportional gain. Increase if bot is sluggish to correct, decrease if it wobbles.

    if (use_ramping) drive_ramped(speed, speed, 0);
    else drive(speed, speed);

    while (elapsed < duration_ms) {
        if (use_ramping) update_ramping();

        // Integrate the gyro Z-axis to get the current heading
        current_angle += gyro_z() * 0.01;

        // Calculate the speed correction based on how far we drifted
        int correction = (int)(current_angle * kP);

        // Apply correction to the base speeds (whether they are currently ramping or fixed)
        int base_l = use_ramping ? cur_l : speed;
        int base_r = use_ramping ? cur_r : speed;

        motor(L_MOTOR, base_l - correction);
        motor(R_MOTOR, base_r + correction);

        msleep(10);
        elapsed += 10;
    }
}


int main (void) {
    servo_enable_all();
    servo_move_smooth(0, 100, 15, 0, 1217);
    servo_disable_all();

    drive(80,80);
    msleep(5000);
    drive(0,0);
    msleep(3000);
    drive_gyro_straight(80, 5000, 1);

    stop_driving(1);
}