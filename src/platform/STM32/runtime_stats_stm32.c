#include <stdint.h>
#include "stm32f4xx.h"

// Uses the ARM DWT cycle counter running at the core clock (100 MHz).
// Wraps every ~42 seconds. 
uint32_t ulGetRunTimeCounterValue(void){
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
    }
    return DWT->CYCCNT;
}
