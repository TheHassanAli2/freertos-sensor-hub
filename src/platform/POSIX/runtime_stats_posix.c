#include <stdint.h>
#include <time.h>

// Returns microseconds since first call
uint32_t ulGetRunTimeCounterValue(void){
    static struct timespec base = {0, 0};
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    if (base.tv_sec == 0 && base.tv_nsec == 0)
        base = now;

    uint64_t us = (uint64_t)(now.tv_sec  - base.tv_sec)  * 1000000ULL + (uint64_t)(now.tv_nsec - base.tv_nsec) / 1000ULL;
    return (uint32_t)us;
}
