#include "uart_hal.h"
#include "sensor_hub.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

void uart_hal_init(void) {}

void uart_dma_transmit(const uint8_t *data, size_t len){
    printf("%.*s", (int)len, (const char *)data);
    fflush(stdout);
    xSemaphoreGive(xTxDoneSem);
}
