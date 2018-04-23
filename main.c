#include "stm32f10x.h"
#include "stm32f10x_rcc.h" /* Reset & Clock Control */
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_usart.h"
#include "misc.h" /* NVIC configuration */
#include "semihosting.h"

#define DEBUG

#include "util.h"
#include "libws2811.h"




#ifdef DEBUG
	#include "stm32f10x_dbgmcu.h"

	void InitDBG(void){
		DBGMCU_Config(DBGMCU_TIM2_STOP, ENABLE); /* this will make TIM2 stop when core is halted during debug */
		DBGMCU_Config(DBGMCU_TIM3_STOP, ENABLE);
		DBGMCU_Config(DBGMCU_STOP, ENABLE);
	}
#endif





int main(void) {
	SystemInit();
	SystemCoreClockUpdate();

#ifdef DEBUG
	InitDBG();
#endif

	InitSysTick();

	DelayMs(500); /* wait 5 sec to avoid boot messages from esp8266. could be made MUCH prettier. */


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	InitGPIO();
	InitDMA();
	InitTIM();
	InitUSART1();



	while(1){}

}
