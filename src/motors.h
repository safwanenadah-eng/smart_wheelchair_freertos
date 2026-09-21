#ifndef MOTORS_H_
#define MOTORS_H_

#include <stdint.h>

/* PWM via Timer0 (OC0A/OC0B) — Timer1 est réservé au tick FreeRTOS */
#define ENA_DDR   DDRD
#define ENA_PIN   PD6   /* OC0A */
#define ENB_DDR   DDRD
#define ENB_PIN   PD5   /* OC0B */

/* Broches de direction */
#define IN1_PORT  PORTD
#define IN1_DDR   DDRD
#define IN1_PIN   PD4

#define IN2_PORT  PORTB
#define IN2_DDR   DDRB
#define IN2_PIN   PB0

#define IN3_PORT  PORTB
#define IN3_DDR   DDRB
#define IN3_PIN   PB3

#define IN4_PORT  PORTD
#define IN4_DDR   DDRD
#define IN4_PIN   PD7

void Motors_Init(void);
void Motors_SetSpeed(uint8_t speedA, uint8_t speedB);
void Motors_Forward(uint8_t speed);
void Motors_Backward(uint8_t speed);
void Motors_TurnLeft(uint8_t speed);
void Motors_TurnRight(uint8_t speed);
void Motors_Stop(void);

#endif /* MOTORS_H_ */