#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <stdint.h>

/* Définition des broches pour le capteur HC-SR04 / ULTRASONIC V2 */
#define TRIG_PIN  PD2
#define ECHO_PIN  PD3
#define US_PORT   PORTD
#define US_DDR    DDRD
#define US_PIN    PIND

/**
 * @brief Initialise les broches Trigger (sortie) et Echo (entrée) du capteur.
 */
void Ultrasonic_Init(void);

/**
 * @brief Mesure et renvoie la distance calculée en centimètres.
 * @return Distance en cm (retourne 400 en cas de dépassement/timeout).
 */
uint16_t Ultrasonic_ReadDistance(void);

#endif /* ULTRASONIC_H_ */