#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sensor_task.h"
#include "BME280.h"
#include "sensor_hub.h"

void sensor_task(void *pvParameters) {

    Sample_t sample;
    if(bme280_init() != BME_OK){
        vTaskDelete(NULL);
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1){
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SAMPLING_INTERVAL_MS));

        if(xSemaphoreTake(xSensorErrSem, 0) == pdTRUE){
            /* Watchdog fired — bus is stuck. Re-init and restart the timer. */
            xSemaphoreTake(xI2CMutex, portMAX_DELAY);
            bme280_init();
            xSemaphoreGive(xI2CMutex);
            xTimerReset(xSensorWatchdog, 0);
            continue;
        }

        xSemaphoreTake(xI2CMutex, portMAX_DELAY);
        if(bme280_read(&sample) == BME_OK){
            sample.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            xQueueSend(xRawQueue, &sample, 0);
            xTimerReset(xSensorWatchdog, 0);
        }
        xSemaphoreGive(xI2CMutex);
    }

}