#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include "bluetooth.h"
#include <avr/io.h>

void Bluetooth_Init(uint32_t baudrate) {
    uint16_t ubrr = (F_CPU / (16UL * baudrate)) - 1;
    
    /* Configuration des registres UBRR */
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    /* Activation de la réception (RX) et de la transmission (TX) */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    /* Format du frame : 8 bits de données, 1 bit d'arrêt, pas de parité */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

uint8_t Bluetooth_Available(void) {
    return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

char Bluetooth_ReadChar(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void Bluetooth_SendChar(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void Bluetooth_SendString(const char *str) {
    while (*str) {
        Bluetooth_SendChar(*str++);
    }
}