#include "FreeRTOS.h"
#include "timers.h"
#include "sensor_hub.h"

void watchdog_cb(TimerHandle_t xTimer){
    (void)xTimer;
    xSemaphoreGive(xSensorErrSem);
}
