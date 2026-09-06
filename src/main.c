#include "sensor_hub.h"
#include "filter_task.h"
#include "uart_task.h"
#include "sensor_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "i2c_hal.h"
#include "uart_hal.h"
#include <stdio.h>
#include <stdlib.h>

void watchdog_cb(TimerHandle_t xTimer);

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName){
    (void)xTask;
    fprintf(stderr, "STACK OVERFLOW in task: %s\n", pcTaskName);
    abort();
}

//Global vars
QueueHandle_t xRawQueue;
QueueHandle_t xFiltQueue;
SemaphoreHandle_t xTxDoneSem;
SemaphoreHandle_t xI2CMutex;
SemaphoreHandle_t xSensorErrSem;
TimerHandle_t xSensorWatchdog;

int main(){

    xRawQueue = xQueueCreate( RAW_QUEUE_DEPTH, sizeof(Sample_t) );
    xFiltQueue = xQueueCreate( FILT_QUEUE_DEPTH, sizeof(Sample_t) );
    xTxDoneSem = xSemaphoreCreateBinary();
    xI2CMutex = xSemaphoreCreateMutex();
    xSensorErrSem = xSemaphoreCreateBinary();
    xSensorWatchdog = xTimerCreate("Watchdog", pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS), pdFALSE, NULL, watchdog_cb);

    BaseType_t xReturned;
    TaskHandle_t sensorHandle = NULL;
    TaskHandle_t filterHandle = NULL;
    TaskHandle_t uartHandle = NULL;    

    xSemaphoreGive(xTxDoneSem);

    uart_hal_init();
    i2c_hal_init();

    xReturned = xTaskCreate(sensor_task, "SENSOR", SENSOR_TASK_STACK, ( void * ) 1, SENSOR_TASK_PRIORITY, &sensorHandle );
    xReturned = xTaskCreate(filter_task, "FILTER", FILTER_TASK_STACK, ( void * ) 1, FILTER_TASK_PRIORITY, &filterHandle );
    xReturned = xTaskCreate(uart_task, "UART", UART_TASK_STACK, ( void * ) 1, UART_TASK_PRIORITY, &uartHandle );

    xTimerStart(xSensorWatchdog, 0);

    vTaskStartScheduler();

    return 0;
}