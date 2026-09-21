MCU = atmega328p
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
PORT = COM5
BAUD = 115200

# Chemins FreeRTOS
FREERTOS_PATH = C:\Users\hp\Desktop\smart_wheelchair_freertos\FreeRTOS_Source

INCLUDES = -I"$(FREERTOS_PATH)/include" \
           -I"$(FREERTOS_PATH)/source/portable" \
           -I.

CFLAGS = -mmcu=$(MCU) -DF_CPU=16000000UL -Os $(INCLUDES)

APP_OBJ = main.o bluetooth.o motors.o ultrasonic.o
FREERTOS_OBJ = tasks.o queue.o list.o timers.o event_groups.o croutine.o heap_1.o port.o

# -------- BUILD --------

all: main.hex

main.hex: main.elf
	$(OBJCOPY) -R .eeprom -O ihex main.elf main.hex

main.elf: $(APP_OBJ) $(FREERTOS_OBJ)
	$(CC) -mmcu=$(MCU) -o main.elf $(APP_OBJ) $(FREERTOS_OBJ)
	avr-size --format=avr --mcu=$(MCU) main.elf

main.o: main.c
	$(CC) -c $(CFLAGS) main.c -o main.o

bluetooth.o: bluetooth.c
	$(CC) -c $(CFLAGS) bluetooth.c -o bluetooth.o

motors.o: motors.c
	$(CC) -c $(CFLAGS) motors.c -o motors.o

ultrasonic.o: ultrasonic.c
	$(CC) -c $(CFLAGS) ultrasonic.c -o ultrasonic.o

tasks.o: $(FREERTOS_PATH)/source/tasks.c
	$(CC) -c $(CFLAGS) $< -o $@

queue.o: $(FREERTOS_PATH)/source/queue.c
	$(CC) -c $(CFLAGS) $< -o $@

list.o: $(FREERTOS_PATH)/source/list.c
	$(CC) -c $(CFLAGS) $< -o $@

timers.o: $(FREERTOS_PATH)/source/timers.c
	$(CC) -c $(CFLAGS) $< -o $@

event_groups.o: $(FREERTOS_PATH)/source/event_groups.c
	$(CC) -c $(CFLAGS) $< -o $@

croutine.o: $(FREERTOS_PATH)/source/croutine.c
	$(CC) -c $(CFLAGS) $< -o $@

heap_1.o: $(FREERTOS_PATH)/source/portable/heap_1.c
	$(CC) -c $(CFLAGS) $< -o $@

port.o: $(FREERTOS_PATH)/source/portable/port.c
	$(CC) -c $(CFLAGS) $< -o $@

# -------- UPLOAD --------

upload: main.hex
	"C:/Users/hp/Desktop/Arduino/hardware/tools/avr/bin/avrdude" \
		-C"C:/Users/hp/Desktop/Arduino/hardware/tools/avr/etc/avrdude.conf" \
		-v -p$(MCU) -carduino -P$(PORT) -b$(BAUD) -D \
		-Uflash:w:main.hex:i

# -------- CLEAN --------

clean:
	del /Q *.elf *.hex *.o 2>nul || echo Clean complete