#include "ultrasonic.h"
#include <avr/io.h>
#include <util/delay.h>

void Ultrasonic_Init(void) {
    /* Configurer TRIG_PIN en sortie */
    US_DDR |= (1 << TRIG_PIN);
    
    /* Configurer ECHO_PIN en entrée */
    US_DDR &= ~(1 << ECHO_PIN);
    
    /* Mettre la broche Trigger à l'état bas par défaut */
    US_PORT &= ~(1 << TRIG_PIN);
}

uint16_t Ultrasonic_ReadDistance(void) {
    uint32_t count = 0;
    uint32_t timeout = 10000;

    /* 1. Générer une impulsion de 10µs sur TRIG */
    US_PORT |= (1 << TRIG_PIN);
    _delay_us(10);
    US_PORT &= ~(1 << TRIG_PIN);

    /* 2. Attendre que la broche ECHO passe à l'état HIGH (avec sécurité anti-blocage) */
    while (!(US_PIN & (1 << ECHO_PIN))) {
        if (--timeout == 0) {
            return 400; /* Aucun obstacle ou hors de portée */
        }
    }

    /* 3. Mesurer la durée du signal HIGH sur ECHO */
    while (US_PIN & (1 << ECHO_PIN)) {
        count++;
        _delay_us(1);
        if (count > 25000) {
            break; /* Sécurité pour éviter de bloquer la tâche FreeRTOS (~4m) */
        }
    }

    /* 4. Conversion du temps en distance (cm)
       Vitesse du son ≈ 340 m/s -> Distance = (Temps * 0.034) / 2 = Temps / 58 */
    return (uint16_t)(count / 58);
}