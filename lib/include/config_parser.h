#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

/**
 * @brief Structure containing all hardware ports and tuning values for the robot.
 */
typedef struct {
    int left_motor;
    int right_motor;
    int aux_motor;
    int left_sensor_port;
    int right_sensor_port;
    int left_threshold;
    int right_threshold;
    int left_black_is_low;
    int right_black_is_low;
    int sensor_offset; // 1 for Front, -1 for Back
    int arm_servo_port;
    int arm_servo_min;
    int arm_servo_max;
} RobotConfig;

/**
 * @brief Loads the config from the text file into the struct.
 * @param config Pointer to the RobotConfig struct.
 * @param filename Path to the configuration file.
 * @return 1 on success, 0 on failure.
 */
int load_config(RobotConfig *config, const char *filename);

#endif