/*
 * servo.c
 */

//greifarm:
//position 104 ist ganz unten
//position 1217 ist ganz oben

#include <kipr/wombat.h>
#include "servo.h"

/*
 * Diese Funktion prueft, ob die gewuenschte Servo-Position innerhalb des erlaubten Bereichs liegt.
 * In position kommt der Zielwert hinein, und min_position sowie max_position geben die untere und obere Grenze an.
 */
static int clamp_servo_position(int position, int min_position, int max_position) {
    if (position < min_position) return min_position;
    if (position > max_position) return max_position;
    return position;
}

/*
 * Diese Funktion schaltet alle angeschlossenen Servos ein, damit sie verwendet werden kC6nnen.
 * Es muessen keine Parameter uebergeben werden.
 */
void servo_enable_all() {
    enable_servos();
}

/*
 * Diese Funktion schaltet alle angeschlossenen Servos wieder aus.
 * Es muessen keine Parameter uebergeben werden.
 */
void servo_disable_all() {
    disable_servos();
}

/*
 * Diese Funktion setzt einen Servo direkt auf eine bestimmte Position.
 * In port kommt der Anschluss des Servos hinein, in position der gewuenschte Wert und in min_position sowie max_position der erlaubte Bewegungsbereich.
 */
void servo_set(int port, int position, int min_position, int max_position) {
    set_servo_position(port, clamp_servo_position(position, min_position, max_position));
    msleep(300);
}

/*
 * Diese Funktion bewegt einen Servo langsam und schrittweise zu einer Zielposition.
 * In port kommt der Servo-Anschluss hinein, in target die Zielposition, in step_delay_ms die Wartezeit pro Schritt und in min_position sowie max_position die erlaubten Grenzen.
 */
void servo_move_smooth(int port, int target, int step_delay_ms, int min_position, int max_position) {
    target = clamp_servo_position(target, min_position, max_position);

    int current = get_servo_position(port);
    if (current < 0) current = target;
    current = clamp_servo_position(current, min_position, max_position);

    if (current < target) {
        while (current < target) {
            current += 10;
            if (current > target) current = target;
            set_servo_position(port, current);
            msleep(step_delay_ms);
        }
    } else {
        while (current > target) {
            current -= 10;
            if (current < target) current = target;
            set_servo_position(port, current);
            msleep(step_delay_ms);
        }
    }
}