#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bluetooth.h"
#include "motors.h"
#include "ultrasonic.h"

/* --- Configurations FreeRTOS --- */
#define QUEUE_LENGTH            5
#define ITEM_SIZE               sizeof(char)

#define PRIORITY_MOTOR_TASK     (tskIDLE_PRIORITY + 2)
#define PRIORITY_BT_TASK        (tskIDLE_PRIORITY + 3)
#define PRIORITY_OBSTACLE_TASK  (tskIDLE_PRIORITY + 4) /* Priorité maximale */
#define MOTOR_MAX_SPEED_KMH_X10   60   /* 6.0 km/h, x10 pour éviter les flottants */

#define SPEED_STEP     5      /* incrément de vitesse à chaque cycle */
#define RAMP_PERIOD_MS 30     /* période de mise à jour de la rampe */

#define OBSTACLE_STOP_CM     30   /* arrêt si distance < 30cm */
#define OBSTACLE_RESUME_CM   40   /* reprise seulement si distance > 40cm (marge de sécurité) */
#define DISPLAY_THRESHOLD_CM 40   /* n'affiche que si obstacle proche */
#define SENSOR_SAMPLES        3

xQueueHandle xCmdQueue = NULL;

/* --- Prototypes des tâches --- */
void vTaskObstacle(void *pvParameters);
void vTaskBluetoothRx(void *pvParameters);
void vTaskMotorControl(void *pvParameters);

int main(void) {
    /* 1. Initialisation des composants matériels */
    Bluetooth_Init(9600);
    Motors_Init();
    Ultrasonic_Init();

    /* 2. Création de la file de messages (Queue) */
    xCmdQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

    if (xCmdQueue != NULL) {
        /* 3. Création des tâches */
        xTaskCreate(vTaskObstacle, 
                    (const signed char *)"Obstacle", 
                    configMINIMAL_STACK_SIZE, 
                    NULL, 
                    PRIORITY_OBSTACLE_TASK, 
                    NULL);

        xTaskCreate(vTaskBluetoothRx, 
                    (const signed char *)"BTRx", 
                    configMINIMAL_STACK_SIZE, 
                    NULL, 
                    PRIORITY_BT_TASK, 
                    NULL);

        xTaskCreate(vTaskMotorControl, 
                    (const signed char *)"Motors", 
                    configMINIMAL_STACK_SIZE, 
                    NULL, 
                    PRIORITY_MOTOR_TASK, 
                    NULL);

        /* 4. Démarrage de l'ordonnanceur FreeRTOS */
        vTaskStartScheduler();
    }

    while (1);
    return 0;
}

/* TÂCHE 1 : Détection d'Obstacle (Haute priorité / Sécurité) */
uint16_t Ultrasonic_ReadFiltered(void) {
    uint16_t s[SENSOR_SAMPLES];
    uint8_t i, j;

    for (i = 0; i < SENSOR_SAMPLES; i++) {
        s[i] = Ultrasonic_ReadDistance();
    }

    /* Tri simple (bubble sort, 3 éléments) */
    for (i = 0; i < SENSOR_SAMPLES - 1; i++) {
        for (j = 0; j < SENSOR_SAMPLES - 1 - i; j++) {
            if (s[j] > s[j + 1]) {
                uint16_t tmp = s[j];
                s[j] = s[j + 1];
                s[j + 1] = tmp;
            }
        }
    }
    return s[1]; /* valeur médiane */
}

void vTaskObstacle(void *pvParameters) {
    uint16_t distance = 0;
    const char stopCmd = 'S';
    char buf[8];
    uint8_t obstacleActive = 0;  /* état : obstacle détecté ou non */

    for (;;) {
        distance = Ultrasonic_ReadFiltered();

        /* Affichage seulement si obstacle proche */
        if (distance < DISPLAY_THRESHOLD_CM) {
            Bluetooth_SendString("D:");
            itoa(distance, buf, 10);
            Bluetooth_SendString(buf);
            Bluetooth_SendString("cm\r\n");
        }

        /* Hystérésis : évite les arrêts/reprises erratiques */
        if (!obstacleActive && distance < OBSTACLE_STOP_CM) {
            obstacleActive = 1;
            Motors_Stop();
            xQueueSendToFront(xCmdQueue, &stopCmd, 0);
            Bluetooth_SendString("OBSTACLE DETECTE - ARRET\r\n");
        }
        else if (obstacleActive && distance > OBSTACLE_RESUME_CM) {
            obstacleActive = 0;
            Bluetooth_SendString("VOIE LIBRE\r\n");
            /* Le fauteuil reste arrêté : l'utilisateur doit retaper F/B pour repartir,
               c'est le comportement de sécurité normal */
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


/* TÂCHE 2 : Réception Bluetooth */
void vTaskBluetoothRx(void *pvParameters) {
    char rxChar = 0;

    for (;;) {
        if (Bluetooth_Available()) {
            rxChar = Bluetooth_ReadChar();
            Bluetooth_SendChar(rxChar);   /* AJOUT TEMPORAIRE pour debug */
            xQueueSend(xCmdQueue, &rxChar, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* TÂCHE 3 : Contrôle Moteurs */
void vTaskMotorControl(void *pvParameters) {
    char command = 'S';
    char lastDirection = 'S';
    char currentDirection = 'S';

    uint8_t targetSpeed  = 0;
    uint8_t currentSpeed = 0;
    const uint8_t maxSpeed = 200;

    static uint8_t sendCounter = 0;   /* <-- compteur pour l'envoi périodique */

    for (;;) {
        /* Timeout court : la tâche tourne même sans nouvelle commande */
        if (xQueueReceive(xCmdQueue, &command, pdMS_TO_TICKS(RAMP_PERIOD_MS)) == pdPASS) {

            uint8_t isReversal =
                ((lastDirection == 'F' && command == 'B') ||
                 (lastDirection == 'B' && command == 'F'));

            if (isReversal) {
                Motors_Stop();
                currentSpeed = 0;
                vTaskDelay(pdMS_TO_TICKS(150));
            }

            switch (command) {
                case 'F': currentDirection = 'F'; targetSpeed = maxSpeed; lastDirection = 'F'; break;
                case 'B': currentDirection = 'B'; targetSpeed = maxSpeed; lastDirection = 'B'; break;
                case 'L': currentDirection = 'L'; targetSpeed = maxSpeed; break;
                case 'R': currentDirection = 'R'; targetSpeed = maxSpeed; break;
                case 'S':
                default:  currentDirection = 'S'; targetSpeed = 0; lastDirection = 'S'; break;
            }
        }

        /* --- Rampe : rapproche currentSpeed de targetSpeed --- */
        if (currentSpeed < targetSpeed) {
            currentSpeed = (targetSpeed - currentSpeed > SPEED_STEP) ?
                            currentSpeed + SPEED_STEP : targetSpeed;
        } else if (currentSpeed > targetSpeed) {
            currentSpeed = (currentSpeed - targetSpeed > SPEED_STEP) ?
                            currentSpeed - SPEED_STEP : targetSpeed;
        }

        /* Applique la vitesse rampée dans la bonne direction */
        switch (currentDirection) {
            case 'F': Motors_Forward(currentSpeed); break;
            case 'B': Motors_Backward(currentSpeed); break;
            case 'L': Motors_TurnLeft(currentSpeed); break;
            case 'R': Motors_TurnRight(currentSpeed); break;
            case 'S':
            default:
                if (currentSpeed == 0) Motors_Stop();
                break;
        }

        /* --- AJOUT : envoi périodique de la vitesse réelle via Bluetooth --- */
        sendCounter++;
        if (sendCounter >= 6) {   /* toutes les ~180ms (6 x 30ms) */
            SendSpeedInfo(currentSpeed);
            sendCounter = 0;
        }
    }
}

void SendSpeedInfo(uint8_t pwmValue) {
    uint16_t speed_x10 = ((uint32_t)pwmValue * MOTOR_MAX_SPEED_KMH_X10) / 255;
    uint8_t entier = speed_x10 / 10;
    uint8_t decimal = speed_x10 % 10;

    char buf[16];
    Bluetooth_SendString("V:");
    itoa(entier, buf, 10);
    Bluetooth_SendString(buf);
    Bluetooth_SendChar('.');
    itoa(decimal, buf, 10);
    Bluetooth_SendString(buf);
    Bluetooth_SendString("km/h\r\n");
}
/* Fonction d'arrière-plan FreeRTOS (exécutée quand le processeur est inactif) */
void vApplicationIdleHook(void) {
    /* Laisser vide ou mettre en mode veille si nécessaire */
}