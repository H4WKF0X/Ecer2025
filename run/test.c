#include <stdio.h>
#include <kipr/wombat.h>
#include <stdlib.h>
#define L_MOTOR 2
#define R_MOTOR 0
#define L_SENSOR_PORT 1
#define R_SENSOR_PORT 0
#define L_THRESH 3540
#define R_THRESH 3067
#define LBLACK_IS_LOW 0
#define RBLACK_IS_LOW 0

// Line Sensor Wrappers
int is_left_black() {
    int val = analog(L_SENSOR_PORT);
    return LBLACK_IS_LOW ? (val < L_THRESH) : (val > L_THRESH);
}

int is_right_black() {
    int val = analog(R_SENSOR_PORT);
    return RBLACK_IS_LOW ? (val < R_THRESH) : (val > R_THRESH);
}

// Basic Movement
void drive(int left_speed, int right_speed) {
    // Kommentar: Sicherheits-Begrenzung, um Motoren zu schützen
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;

    motor(L_MOTOR, left_speed);
    motor(R_MOTOR, right_speed);
}

void stop_driving() {
    drive(0, 0);
}

void drive_until_black(int speed) {
    drive(speed, speed);
    // Kommentar: Fahre weiter, solange MINDESTENS ein Sensor weiß ist
    while (!is_left_black() || !is_right_black()) {
    	msleep(10);
    }
    stop_driving();
}

void drive_until_white(int speed) {
    drive(speed, speed);
    // Keep driving while AT LEAST ONE sensor is black
    while (is_left_black() || is_right_black()) {
        msleep(10);
    }
    stop_driving();
}

int main(void)
{   
    drive_until_black(90);
    
    drive_until_white(90);
    
    drive_until_black(90);
    
    return 0;
}