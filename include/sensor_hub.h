#ifndef SENSOR_HUB_H
#define SENSOR_HUB_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#define SENSOR_TASK_PRIORITY    3
#define FILTER_TASK_PRIORITY    2
#define UART_TASK_PRIORITY      1

#define SENSOR_TASK_STACK       512
#define FILTER_TASK_STACK       512
#define UART_TASK_STACK         512

#define SAMPLING_INTERVAL_MS        100

#define RAW_QUEUE_DEPTH         8
#define FILT_QUEUE_DEPTH        8

typedef struct {
    int32_t  temperature;
    uint32_t pressure;
    uint32_t humidity;
    uint32_t timestamp_ms;
} Sample_t;

extern QueueHandle_t xRawQueue;
extern QueueHandle_t xFiltQueue;
extern SemaphoreHandle_t xTxDoneSem;
extern SemaphoreHandle_t xI2CMutex;
extern SemaphoreHandle_t xSensorErrSem;
extern TimerHandle_t xSensorWatchdog;

#define WATCHDOG_TIMEOUT_MS (300)

#endif /* SENSOR_HUB_H */
