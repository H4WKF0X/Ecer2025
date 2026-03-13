void testServo() {
    servo_enable_all();

    servo_set(0, 104, 0, 1217);
    msleep(1000);

    servo_move_smooth(0, 1217, 15, 0, 1217);
    msleep(1000);

    servo_move_smooth(0, 104, 15, 0, 1217);
    msleep(1000);

    servo_disable_all();
}