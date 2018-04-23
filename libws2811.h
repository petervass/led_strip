#ifndef _LIBWS2811_H

#define _LIBWS2811_H


#include "stm32f10x.h"
#include "stm32f10x_rcc.h" /* Reset & Clock Control */
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_usart.h"
#include "misc.h" /* NVIC configuration */
#include "semihosting.h"

#include "libws2811_anim.h"

#define DEBUG_LIBWS2811

/* high period times in counter ticks @72MHz */
/* 90 tick is equal with 1.25 us*/
/*ws2811 - ws2812b
 *  in fast mode
 *  T1H = 43
 *  T0H = 18
 *  T_TOTAL = 90*/
/*GS1903
 * in fast mode
 * T1H = 72
 * T0H = 18
 * T_TOTAL = 90*/
#define T1H 72
#define T0H 18
#define T_TOTAL 90 /* 1.25 us */

/* values of main colors for led chain */
#define R 		0x00ff00
#define B 		0xff0000
#define G 		0x0000ff
#define W 		0xffffff
#define NO_COL 	0x1000000 /* no output should be sent, i.e. reset */

/* constants for WS2811 communication buffers */
#define DOUBLE_BUF_LEN 4 /* number of colors in double buffer*/
#define TIM2_DMA_BUF_LEN (24*DOUBLE_BUF_LEN) /* 24 bits for each led */
#define CHAIN_LEN (52) /* rounded up to next multiple of DOUBLE_BUF_LEN */


#define USART1_DMA_BUF_LEN 8 /* lenght of USART1 circular buffer for color reception */

void sendColor(uint16_t nr);
void sendReset(void); /* NOT IMPLEMENTED */
void sendClear(uint16_t nr);

uint32_t packColor(uint8_t r,uint8_t g,uint8_t b);
uint32_t packColorRGB(uint8_t r,uint8_t g,uint8_t b);

/*
void DMA1_Channel7_IRQHandler(void);
void TIM3_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void USART1_IRQHandler(void);
*/

void InitUSART1(void);
void InitGPIO(void);
void InitDMA(void);
void InitTIM(void);

uint8_t gammaCor(uint8_t c);



#endif
