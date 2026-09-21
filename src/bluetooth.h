#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include <stdint.h>

void Bluetooth_Init(uint32_t baudrate);
uint8_t Bluetooth_Available(void);
char Bluetooth_ReadChar(void);
void Bluetooth_SendChar(char data);
void Bluetooth_SendString(const char *str);

#endif /* BLUETOOTH_H_ */