# FreeRTOS Sensor Hub

FreeRTOS-based sensor hub on the STM32F411RE (NUCLEO-F411RE) reading temperature, pressure, and humidity from a Bosch BME280 over I2C. Includes a POSIX test to just see if things are somewhat working but it's not randomized or different yet.

## Hardware

| Component | Part |
|-----------|------|
| MCU | STM32F411RET6 @ 100 MHz |
| Board | NUCLEO-F411RE |
| Sensor | Bosch BME280 |

## Architecture

Three FreeRTOS tasks communicate through queues:

```
BME280 --I2C--> [sensor_task P3] --xRawQueue--> [filter_task P2] --xFiltQueue--> [uart_task P1] --> USART2
```

| Task | Priority | Period | Role |
|------|----------|--------|------|
| `sensor_task` | 3 | 100 ms (`vTaskDelayUntil`) | Reads BME280, pushes raw `Sample_t` |
| `filter_task` | 2 | event-driven | EMA filter (α=0.2, Q15 fixed-point) |
| `uart_task` | 1 | event-driven | Formats and transmits via DMA |

**Synchronisation primitives**
- `xI2CMutex` — guards I2C bus access
- `xTxDoneSem` — binary semaphore; given by `HAL_UART_TxCpltCallback` on DMA transfer complete
- `xSensorErrSem` — given by the watchdog timer callback on bus fault
- `xSensorWatchdog` — one-shot 300 ms software timer; reset on every successful read, fires after 3 missed samples

## BME280 Driver

Driver at `src/drivers/BME280.c`. Key datasheet references (BST-BME280-DS002):

| What | Datasheet ref | Register |
|------|--------------|---------|
| Chip ID check (expect `0x60`) | 4.2.1 | `0xD0` |
| Humidity oversampling ×1 | 3.5.1, Table 20 | `0xF2 = 0x01` |
| Temp/pressure oversampling ×16/×8, normal mode | 3.5.1, Table 22 | `0xF4 = 0xB7` |
| IIR filter coeff 16, standby 0.5 ms | 3.6, Table 27 | `0xF5 = 0xA0` |
| Calibration registers (T, P) | 4.2.2 | `0x88–0x9F` |
| Humidity calibration dig_H1 | 4.2.2 | `0xA1` |
| Humidity calibration dig_H2–H6 | 4.2.2 | `0xE1–0xE7` |
| Burst read — press/temp/hum ADC | 4.2.3 | `0xF7–0xFE` (8 bytes) |

Raw ADC values are 20-bit (temperature, pressure) and 16-bit (humidity), extracted by bit-shifting the burst-read buffer per 4.2.3.

Compensation formulas are the integer-only variants from 4.2.3 (32-bit temperature/humidity, 64-bit pressure). `t_fine` is computed by the temperature compensation and shared with pressure and humidity, as described in 4.2.3.

## EMA Filter

Q15 fixed-point exponential moving average to avoid floating-point arithmetic:

```
α     = 0.2  →  α_q15 = 6554  (= 0.2 × 2^15)
y[n]  = y[n-1] + ((α_q15 × (x[n] - y[n-1])) >> 15)
```

Applied per field (`temperature`, `pressure`, `humidity`) using `int64_t` intermediates to prevent overflow before the right-shift.

## Build

### POSIX simulator (WSL / Linux)

```bash
cmake -B build -S .
cmake --build build
./build/sensor_hub
```

Outputs readings every 100 ms. Runtime stats print every 50 samples (~5 s).

### STM32

Requires `arm-none-eabi-gcc` and the CubeMX-generated files in `cubemx/`.

```bash
cmake -B build-stm32 -S . -DPLATFORM=STM32 -DCMAKE_TOOLCHAIN_FILE=cubemx/freertos-sensor-hub/cmake/gcc-arm-none-eabi.cmake
cmake --build build-stm32
```

Flash with `st-flash` or OpenOCD.

## Project layout

```
.
├── config/                  FreeRTOSConfig.h
├── include/                 sensor_hub.h, i2c_hal.h, uart_hal.h
├── src/
│   ├── drivers/             BME280.c/h
│   ├── tasks/               sensor_task, filter_task, uart_task, watchdog_cb
│   ├── platform/
│   │   ├── POSIX/           i2c_posix, uart_posix, runtime_stats_posix
│   │   └── STM32/           i2c_stm32, uart_stm32, runtime_stats_stm32
│   └── main.c
└── cubemx/                  CubeMX-generated HAL, startup, linker script
```
