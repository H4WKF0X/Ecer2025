#include "config_parser.h"
#include <stdio.h>
#include <string.h>

int load_config(RobotConfig *config, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return 0;

    char key[50];
    int val;
    
    // Read the file line-by-line and map keys to the struct
    while (fscanf(fp, "%s %d", key, &val) != EOF) {
        if (strcmp(key, "L_MOTOR") == 0) config->left_motor = val;
        else if (strcmp(key, "R_MOTOR") == 0) config->right_motor = val;
        else if (strcmp(key, "AUX_MOTOR") == 0) config->aux_motor = val;
        else if (strcmp(key, "L_SENSOR_PORT") == 0) config->left_sensor_port = val;
        else if (strcmp(key, "R_SENSOR_PORT") == 0) config->right_sensor_port = val;
        else if (strcmp(key, "L_THRESH") == 0) config->left_threshold = val;
        else if (strcmp(key, "R_THRESH") == 0) config->right_threshold = val;
        else if (strcmp(key, "LBLACK_IS_LOW") == 0) config->left_black_is_low = val;
        else if (strcmp(key, "RBLACK_IS_LOW") == 0) config->right_black_is_low = val;
        else if (strcmp(key, "SENSOR_OFFSET") == 0) config->sensor_offset = val;
        else if (strcmp(key, "ARM_SERVO") == 0) config->arm_servo_port = val;
        else if (strcmp(key, "ARM_SERVO_MIN") == 0) config->arm_servo_min = val;
        else if (strcmp(key, "ARM_SERVO_MAX") == 0) config->arm_servo_max = val;
    }

    fclose(fp);
    return 1;
}