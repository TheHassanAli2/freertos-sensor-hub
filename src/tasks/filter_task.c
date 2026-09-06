#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "filter_task.h"
#include "sensor_hub.h"

void filter_task(void *pvParameters) {

    uint8_t isFirstSample = 1;
    Sample_t filtered = {0};

    while(1){
        Sample_t sample;
        xQueueReceive(xRawQueue, &sample, portMAX_DELAY);

        if(isFirstSample){
            filtered = sample;
            isFirstSample = 0;
        }
        else{
            // This applies an EMA filter of alpha = 0.2 (1/5) to the sample. 0.2 * 2^15 = 6554. Which is why I multiply by 6554 and
            // then shift right by 15 to get the same result as multiplying by 0.2 without using floating point math.
            filtered.temperature = filtered.temperature + (int32_t)(((int64_t)6554 * (int64_t)(sample.temperature - filtered.temperature)) >> 15);
            filtered.pressure = filtered.pressure + (uint32_t)(((int64_t)6554 * (int64_t)((int64_t)sample.pressure - (int64_t)filtered.pressure)) >> 15);
            filtered.humidity = filtered.humidity + (uint32_t)(((int64_t)6554 * (int64_t)((int64_t)sample.humidity - (int64_t)filtered.humidity)) >> 15);
            filtered.timestamp_ms = sample.timestamp_ms;
        }
        xQueueSend(xFiltQueue, &filtered, 0);
    }

}