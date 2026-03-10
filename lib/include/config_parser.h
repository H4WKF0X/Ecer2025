#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

typedef struct {
    int left_motor;
    int right_motor;
    int left_sensor_port;
    int right_sensor_port;
    int left_threshold;
    int right_threshold;
    int left_black_is_low;
    int right_black_is_low;
    int sensor_offset; // 1 for Front, -1 for Back
} RobotConfig;

// Loads the config from the text file into the struct
int load_config(RobotConfig *config, const char *filename);

#endif