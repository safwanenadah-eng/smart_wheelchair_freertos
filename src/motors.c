#include "motors.h"
#include <avr/io.h>
#include <util/delay.h>

void Motors_Init(void) {
    /* Broches de direction en sortie */
    IN1_DDR |= (1 << IN1_PIN);
    IN2_DDR |= (1 << IN2_PIN);
    IN3_DDR |= (1 << IN3_PIN);
    IN4_DDR |= (1 << IN4_PIN);

    /* Broches PWM en sortie */
    ENA_DDR |= (1 << ENA_PIN);
    ENB_DDR |= (1 << ENB_PIN);

    /* Timer0 : Fast PWM 8 bits, sorties non-inversées sur OC0A/OC0B */
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    /* Prescaler 64 -> fréquence PWM ~1kHz (16MHz / 64 / 256) */
    TCCR0B = (1 << CS01) | (1 << CS00);

    Motors_Stop();
}

void Motors_SetSpeed(uint8_t speedA, uint8_t speedB) {
    OCR0A = speedA;
    OCR0B = speedB;
}

void Motors_Forward(uint8_t speed) {
    IN1_PORT |= (1 << IN1_PIN);
    IN2_PORT &= ~(1 << IN2_PIN);
    IN3_PORT |= (1 << IN3_PIN);
    IN4_PORT &= ~(1 << IN4_PIN);
    Motors_SetSpeed(speed, speed);
}

void Motors_Backward(uint8_t speed) {
    IN1_PORT &= ~(1 << IN1_PIN);
    IN2_PORT |= (1 << IN2_PIN);
    IN3_PORT &= ~(1 << IN3_PIN);
    IN4_PORT |= (1 << IN4_PIN);
    Motors_SetSpeed(speed, speed);
}

void Motors_TurnLeft(uint8_t speed) {
    IN1_PORT &= ~(1 << IN1_PIN);
    IN2_PORT &= ~(1 << IN2_PIN);
    IN3_PORT |= (1 << IN3_PIN);
    IN4_PORT &= ~(1 << IN4_PIN);
    Motors_SetSpeed(0, speed);
}

void Motors_TurnRight(uint8_t speed) {
    IN1_PORT |= (1 << IN1_PIN);
    IN2_PORT &= ~(1 << IN2_PIN);
    IN3_PORT &= ~(1 << IN3_PIN);
    IN4_PORT &= ~(1 << IN4_PIN);
    Motors_SetSpeed(speed, 0);
}

void Motors_Stop(void) {
    /* Freinage actif (brake) : les deux entrées de chaque canal à HIGH
       court-circuite le moteur via le pont en H pour un arrêt rapide */
    IN1_PORT |= (1 << IN1_PIN);
    IN2_PORT |= (1 << IN2_PIN);
    IN3_PORT |= (1 << IN3_PIN);
    IN4_PORT |= (1 << IN4_PIN);
    Motors_SetSpeed(255, 255);  /* PWM maximal pendant le freinage */

    _delay_ms(100);

    /* Coupure complète après le freinage */
    IN1_PORT &= ~(1 << IN1_PIN);
    IN2_PORT &= ~(1 << IN2_PIN);
    IN3_PORT &= ~(1 << IN3_PIN);
    IN4_PORT &= ~(1 << IN4_PIN);
    Motors_SetSpeed(0, 0);
}