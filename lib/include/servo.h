/*
 * servo.h
 * Diese Header-Datei enthaelt die Deklarationen der Servo-Funktionen.
 * Sie wird eingebunden, damit andere Dateien diese Funktionen verwenden koennen.
 */

#ifndef SERVO_H
#define SERVO_H

/*
 * Diese Funktion schaltet alle angeschlossenen Servos ein, damit sie bewegt werden kC6nnen.
 * Es muessen keine Parameter uebergeben werden.
 */
void servo_enable_all();

/*
 * Diese Funktion schaltet alle angeschlossenen Servos wieder aus.
 * Es muessen keine Parameter uebergeben werden.
 */
void servo_disable_all();

/*
 * Diese Funktion setzt einen Servo direkt auf eine bestimmte Position.
 * In port kommt der Anschluss des Servos hinein, in position der gewuenschte Wert und in min_position sowie max_position der erlaubte Bereich.
 */
void servo_set(int port, int position, int min_position, int max_position);

/*
 * Diese Funktion bewegt einen Servo langsam zu einer Zielposition.
 * In port kommt der Anschluss hinein, in target die Zielposition, in step_delay_ms die Wartezeit zwischen den Schritten und in min_position sowie max_position die erlaubten Grenzen.
 */
void servo_move_smooth(int port, int target, int step_delay_ms, int min_position, int max_position);

#endif