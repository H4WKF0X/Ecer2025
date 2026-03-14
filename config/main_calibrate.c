#include <stdio.h>
#include <stdlib.h>

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int main(void) {
    //Port variables
    int left_motor, right_motor, aux_motor;
    int left_sensor, right_sensor;
    int arm_servo, arm_min, arm_max;
    
    // Per-sensor variables
    int l_black, l_white, r_black, r_white;
    int l_thresh, r_thresh;
    int lblack_is_low, rblack_is_low;

    // Line Sensor location
    int sensor_offset, offset_choice;

    printf("MANUAL ROBOT CALIBRATION\n\n");

    // STEP 1: PORT CONFIGURATION
    printf("1. Hardware Ports\n");
    printf("Enter Left Motor Port (0-3): ");
    scanf("%d", &left_motor);
    
    printf("Enter Right Motor Port (0-3): ");
    scanf("%d", &right_motor);

    printf("Enter Auxiliary Motor Port (0-3): ");
    scanf("%d", &aux_motor);

    printf("Enter Left Sensor Port (analog 0-5): ");
    scanf("%d", &left_sensor);

    printf("Enter Right Sensor Port (analog 0-5): ");
    scanf("%d", &right_sensor);
    clear_input();

    printf("\nEnter Arm Servo Port (0-3): ");
    scanf("%d", &arm_servo);

    printf("Enter Arm Servo MIN position (e.g., 104): ");
    scanf("%d", &arm_min);

    printf("Enter Arm Servo MAX position (e.g., 1217): "); 
    scanf("%d", &arm_max);

    // STEP 2: SENSOR CALIBRATION
    printf("\n2. Sensor Calibration\n");
    printf("\nCalibrating LEFT Sensor (Port %d)\n", left_sensor);

    printf("\nType the BLACK value you saw for LEFT: ");
    scanf("%d", &l_black);
    printf("Type the WHITE value you saw for LEFT: ");
    scanf("%d", &l_white);

    printf("\nCalibrating RIGHT Sensor (Port %d)\n", right_sensor);

    printf("\nType the BLACK value you saw for RIGHt: ");
    scanf("%d", &r_black);
    printf("Type the WHITE value you saw for RIGHT: ");
    scanf("%d", &r_white);

    printf("\nSensor Position:\n [1] In Front of Wheels\n [2] Behind Wheels\n Choice: ");
    scanf("%d", &offset_choice);

    // Calculations
    l_thresh = (l_black + l_white) / 2;
    r_thresh = (r_black + r_white) / 2;
    lblack_is_low = (l_black < l_white);
    rblack_is_low = (r_black < r_white);
    sensor_offset = (offset_choice == 1) ? 1 : -1;

    // STEP 3: SAVE
    FILE *fp = fopen("../data/robot_config.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not save to data/robot_config.txt\n");
        return 1;
    }

    fprintf(fp, "L_MOTOR %d\nR_MOTOR %d\n", left_motor, right_motor);
    fprintf(fp, "AUX_MOTOR %d\n", aux_motor);

    fprintf(fp, "L_SENSOR_PORT %d\nR_SENSOR_PORT %d\n", left_sensor, right_sensor);
    fprintf(fp, "L_THRESH %d\nR_THRESH %d\n", l_thresh, r_thresh);
    fprintf(fp, "LBLACK_IS_LOW %d\nRBLACK_IS_LOW %d\n", lblack_is_low, rblack_is_low);
    fprintf(fp, "SENSOR_OFFSET %d\n", sensor_offset);

    fprintf(fp, "ARM_SERVO %d\n", arm_servo);
    fprintf(fp, "ARM_SERVO_MIN %d\n", arm_min);
    fprintf(fp, "ARM_SERVO_MAX %d\n", arm_max);

    fclose(fp);
    printf("\nCalibration saved successfully!\n");
    return 0;
}