#ifndef UART_HAL_H
#define UART_HAL_H

#include <stdint.h>
#include <stddef.h>

void uart_hal_init(void);

void uart_dma_transmit(const uint8_t *data, size_t len);

#endif /* UART_HAL_H */
