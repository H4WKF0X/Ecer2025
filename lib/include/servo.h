#ifndef SERVO_H
#define SERVO_H

#define SERVO_SET_DELAY_MS 300
#define SERVO_SMOOTH_STEP 10

void servo_enable_all();

void servo_disable_all();

void servo_set(int port, int position, int min_position, int max_position);

void servo_move_smooth(int port, int target, int step_delay_ms, int min_position, int max_position);

#endif