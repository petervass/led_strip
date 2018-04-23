#ifndef _UTIL_H

#define _UTIL_H

#include "stm32f10x.h"
#include "stm32f10x_rcc.h" /* Reset & Clock Control */

#define SYS_TICK_FREQ (1000) /* freq. of sys_tick event in Hz. Used by InitSystick func. */

void SysTick_Handler(void);
void InitSysTick(void);
void DelayMs(uint32_t t);


#endif
