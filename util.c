#include "util.h"



__IO uint32_t SysTickCnt=0; /* counts nr. of ticks since boot */



/* increment SysTick counter, useful for delay functions */
/* with systick f=1kHz, it overflows every 49 days */
void SysTick_Handler(void){
	SysTickCnt++;
}


void InitSysTick(void){
	RCC_ClocksTypeDef clkConfStruct;
	RCC_GetClocksFreq(&clkConfStruct);

	SysTick_Config(clkConfStruct.SYSCLK_Frequency/SYS_TICK_FREQ-1); /* 1ms/SysClk tick */
}

void DelayMs(uint32_t t){
	uint32_t startTick=SysTickCnt;

	while(SysTickCnt-startTick<=t){}
}

