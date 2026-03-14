//greifarm:
//position 104 ist ganz unten
//position 1217 ist ganz oben

#include <kipr/wombat.h>
#include "servo.h"

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

/**
 * @brief Moves a servo to a target position using incremental steps for a smoother motion.
 * Useful for preventing the robot from jerking or for handling fragile game pieces.
 * @param port The servo port index.
 * @param target The final desired position.
 * @param step_delay_ms Time to wait between each increment (higher = slower).
 * @param min_position Safety minimum.
 * @param max_position Safety maximum.
 */
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