#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart_task.h"
#include "sensor_hub.h"
#include "uart_hal.h"
#include "semphr.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void uart_task(void *pvParameters) {
    Sample_t sample;
    uint32_t sample_count = 0;

    if(xTxDoneSem == NULL){
        vTaskDelete(NULL);
    }

    while(1){
        xQueueReceive(xFiltQueue, &sample, portMAX_DELAY);
        if(xSemaphoreTake(xTxDoneSem, portMAX_DELAY) == pdTRUE){
            uint8_t buffer[256];
            int len = snprintf((char *)buffer, sizeof(buffer),
                "Temperature: %d.%02d C, Pressure: %u.%02u Pa, Humidity: %u.%02u %%\n",
                sample.temperature / 100, abs(sample.temperature % 100),
                sample.pressure / 256, (sample.pressure * 100 / 256) % 100,
                sample.humidity / 1024, (sample.humidity * 100 / 1024) % 100);
            uart_dma_transmit(buffer, len);
        }

        sample_count++;
        if(sample_count % 50 == 0){
            static char stats_buf[512];
            vTaskGetRunTimeStats(stats_buf);
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);

            if(xSemaphoreTake(xTxDoneSem, portMAX_DELAY) == pdTRUE){
                uint8_t hdr[128];
                int hdr_len = snprintf((char *)hdr, sizeof(hdr),
                    "\n--- Runtime stats (UART HWM: %u words) ---\n", (unsigned)hwm);
                uart_dma_transmit(hdr, hdr_len);
            }
            if(xSemaphoreTake(xTxDoneSem, portMAX_DELAY) == pdTRUE){
                uart_dma_transmit((uint8_t *)stats_buf, strlen(stats_buf));
            }
        }
    }

}