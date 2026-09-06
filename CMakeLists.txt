cmake_minimum_required(VERSION 3.20)
project(freertos_sensor_hub C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    freertos_kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG        V11.1.0
    GIT_SHALLOW    TRUE
)

# Must be set before FetchContent_MakeAvailable
set(FREERTOS_PORT GCC_POSIX CACHE INTERNAL "")
set(FREERTOS_HEAP 3        CACHE INTERNAL "")

# freertos_config supplies FreeRTOSConfig.h to the kernel
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/config)

FetchContent_MakeAvailable(freertos_kernel)

set(CORE_SOURCES
    src/tasks/sensor_task.c
    src/tasks/filter_task.c
    src/tasks/uart_task.c
    src/tasks/watchdog_cb.c
    src/drivers/bme280.c
)

set(POSIX_SOURCES
    src/platform/POSIX/i2c_posix.c
    src/platform/POSIX/uart_posix.c
    src/platform/POSIX/runtime_stats_posix.c
)

set(STM32_SOURCES
    src/platform/STM32/i2c_stm32.c
    src/platform/STM32/uart_stm32.c
    src/platform/STM32/runtime_stats_stm32.c
)

# Build for POSIX simulator by default; pass -DPLATFORM=STM32 for hardware.
if(NOT DEFINED PLATFORM)
    set(PLATFORM POSIX)
endif()

if(PLATFORM STREQUAL "STM32")
    set(PLATFORM_SOURCES ${STM32_SOURCES})
    set(PLATFORM_DEFINE  PLATFORM_STM32)
    set(PLATFORM_LIBS    freertos_kernel)
else()
    set(PLATFORM_SOURCES ${POSIX_SOURCES})
    set(PLATFORM_DEFINE  PLATFORM_POSIX)
    set(PLATFORM_LIBS    freertos_kernel pthread m)
endif()

add_executable(sensor_hub
    src/main.c
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(sensor_hub PRIVATE
    include
    config
    src/drivers
    src/tasks
)

target_link_libraries(sensor_hub PRIVATE ${PLATFORM_LIBS})
target_compile_definitions(sensor_hub PRIVATE ${PLATFORM_DEFINE})
target_compile_definitions(freertos_config INTERFACE ${PLATFORM_DEFINE})

enable_testing()
# add_subdirectory(tests)
