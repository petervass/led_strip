#include "libws2811.h"

/* gamma correction table */
const uint8_t gam[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
        2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
        5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
       10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
       17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
       25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
       37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
       51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
       69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
       90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
      115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
      144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
      177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
      215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };




__IO uint16_t idx=0; /* number of half buffers processed */

__IO uint8_t txOn=0; /* sending in progress flag */

__IO uint16_t txLen=0; /* bytes to be sent */

__IO uint16_t buf[TIM2_DMA_BUF_LEN]; /* DMA double buffer for TIM2 oc values */

__IO uint32_t col[CHAIN_LEN]; /* color data: 0x00GGRRBB - size should be multiple of DOUBLE_BUF_LEN */
/* @TODO: pack color data to save 1 byte/color */

void DMA1_Channel7_IRQHandler(void){
#ifdef DEBUG_LIBWS2811
		GPIOA->ODR ^= 0b100; /* toggle PIN2 for debug purposes */
#endif

	uint16_t offset=0; /* which half of double buffer is currently used? */

	if(DMA_GetITStatus(DMA1_IT_TC7)!= RESET){
		DMA_ClearITPendingBit(DMA1_IT_TC7);

		offset=DOUBLE_BUF_LEN/2;
	}
	if(DMA_GetITStatus(DMA1_IT_HT7)!= RESET){
		DMA_ClearITPendingBit(DMA1_IT_HT7);

		offset=0;
	}

	/* fill one half of buffer */
	idx++;

	if((idx+1)*DOUBLE_BUF_LEN/2<=txLen){
		/*fill up first half of buf*/

		for(uint16_t i=offset;i<DOUBLE_BUF_LEN/2+offset;i++){
			for(uint16_t j=0;j<24;j++){
				uint32_t c=col[(idx+1)*DOUBLE_BUF_LEN/2+i-offset];
				if(c!=NO_COL)
					buf[23-j+24*i]=(c&(1<<j))?T1H:T0H;
				else
					buf[23-j+24*i]=0; /* current value is not a color, keep output low */
			}
		}
	}
	else{
		for(uint16_t i=offset;i<DOUBLE_BUF_LEN/2+offset;i++)
			for(uint16_t j=0;j<24;j++)
				buf[j+24*i]=0;
	}

	if(idx*DOUBLE_BUF_LEN/2>=txLen){
		/* stop sending, clear buf */
		DMA_Cmd(DMA1_Channel7, DISABLE);
		TIM_Cmd(TIM2, DISABLE);

		for(uint16_t i=0;i<TIM2_DMA_BUF_LEN;i++)
			buf[i]=0;
		txOn=0; /* not sending currently */
	}
}


__IO uint32_t usartCol=R; /* gamma corrected color received via usart1 */
__IO uint8_t usartMod=0; /* mode byte, to set animation, etc. */

void TIM3_IRQHandler(void){ /* 30Hz refresh rate */
#ifdef DEBUG_LIBWS2811
        GPIOA->ODR ^= 0b1000;
#endif

	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM3,  TIM_IT_Update);

        anim(usartMod,usartCol,col,CHAIN_LEN);

        col[CHAIN_LEN-1]=col[CHAIN_LEN-2]=NO_COL;

        sendColor(CHAIN_LEN-2);
   }
}



//__IO uint8_t frameIdx=0; /* current byte in buffer */
//void USART1_IRQHandler(void){
//  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
//	  USART_ClearITPendingBit(USART1,USART_IT_RXNE);
//
//	  uint8_t tmp=USART_ReceiveData(USART1);
//
//	 printf("%x\n",tmp);
//
//	  if((tmp&(1<<7))==0){
//		  if(frameIdx>=3){ /* ok! bytes are in order, we have a full frame */
//			  frameIdx=0;
//
//			  /* extract color values */
//
//			  uint32_t c=(tmp&0x7f) | ((usartBuf[2]&0x7f)<<7) | ((usartBuf[1]&0x7f)<<(2*7)) | ((usartBuf[0]&0x7f)<<(3*7));
//
//			  uint8_t m= c&0xf;
//
//			  uint8_t b= (c&((uint32_t)0xff<<4))>>4;
//			  uint8_t g= (c&((uint32_t)0xff<<(4+8)))>>(4+8);
//			  uint8_t r= (c&((uint32_t)0xff<<(4+16)))>>(4+16);
//
//			  usartCol=packColor(gam[r],gam[g],gam[b]);
//
//			  printf("%x\n",c);
//			  printf("%x %x %x %x\n",usartBuf[0],usartBuf[1],usartBuf[2],tmp);
//
//			  for(int i=0;i<4;i++)
//				  usartBuf[i]=0;
//
//		  }
//		  else{ /* our buffer is misaligned, some packets were skipped */
//		  }
//
//		  frameIdx=0; /* prepare for next frame */
//		  for(int i=0;i<4;i++)
//			  usartBuf[i]=0;
//	  }
//	  else{
//		  if(frameIdx<3){ /* ok! */
//			  usartBuf[frameIdx]=tmp;
//			  frameIdx++;
//		  }
//		  else{ /* misaligned buffer, byte should start with a 1 */
//			  frameIdx=0; /* prepare for next frame */
//
//			  for(int i=0;i<4;i++)
//				  usartBuf[i]=0;
//		  }
//	  }
//  }
//
//  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET){
//	  USART_ClearITPendingBit(USART1,USART_IT_TXE);
//  }
//}

__IO uint8_t usartBuf[USART1_DMA_BUF_LEN]={0}; /* circular buffer for usart */
/* USART1 RX DMA handler, uses circular buffer */
void DMA1_Channel5_IRQHandler(void){
	uint8_t usartOffset=0; /* determines which half of double buf to use */


	if(DMA_GetFlagStatus(DMA1_FLAG_TC5)!= RESET){
		DMA_ClearFlag(DMA1_FLAG_TC5);

		usartOffset=USART1_DMA_BUF_LEN/2;
	}

	if(DMA_GetFlagStatus(DMA1_FLAG_HT5)!= RESET){
		DMA_ClearFlag(DMA1_FLAG_HT5);

		usartOffset=0;
	}
	/* extract raw color data/for specific animations first byte is palette number*/
	uint8_t r_t=usartBuf[0+usartOffset]; /* also used for palette data for certain anims */
	uint8_t g_t=usartBuf[1+usartOffset];
	uint8_t b_t=usartBuf[2+usartOffset];

	usartMod=usartBuf[3+usartOffset];
	usartCol=packColorRGB(r_t,g_t,b_t); /* ready to be applied to chain */


#ifdef DEBUG_LIBWS2811
	char flag;
	if(!usartOffset)
		flag='H'; /* half transfer */
	else
		flag='C'; /* complete transfer */

	printf("%d %d %d %c\n",r_t,b_t,g_t,flag);
#endif

}

void InitUSART1(void){
	USART_InitTypeDef USARTInitStruct;
	USART_StructInit(&USARTInitStruct);

	/* Enalbe clock for USART1, AFIO and GPIOA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);


	USARTInitStruct.USART_BaudRate = 9600;
	USARTInitStruct.USART_WordLength = USART_WordLength_8b;
	USARTInitStruct.USART_StopBits = USART_StopBits_1;
	USARTInitStruct.USART_Parity = USART_Parity_No ;
	USARTInitStruct.USART_Mode = USART_Mode_Rx; /* | USART_Mode_Tx */
	USARTInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USARTInitStruct);



	/* Enable the USART1 Interrupt */


	/* Configure the NVIC Preemption Priority Bits */
	/*NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);*/
	/*NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(USART1_IRQn);*/
	/* Enable USART1 global interrupt */



	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); /* Enable RXNE interrupt */
	//USART_ITConfig(USART1, USART_IT_TXE, ENABLE); /* Enable TXNE interrupt */

	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); /* Enable USART1 DMA Rx and TX request */
	USART_Cmd(USART1, ENABLE); /* Enable USART1 */
}





void InitGPIO(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIOInitStruct;
	GPIO_StructInit(&GPIOInitStruct);

	GPIOInitStruct.GPIO_Pin = GPIO_Pin_1; /* OC output of TIM2 to comm. with WS2811 */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_10MHz;

	GPIO_Init(GPIOA, &GPIOInitStruct);

#ifdef DEBUG_LIBWS2811
	GPIOInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; /* used for debugging */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIOInitStruct);
#endif

	GPIOInitStruct.GPIO_Pin = GPIO_Pin_9; /* USART Tx */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIOInitStruct);


	GPIOInitStruct.GPIO_Pin = GPIO_Pin_10; /* USART Rx */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIOInitStruct);
}

void InitTIM(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); /* OC pin used to comm. with chain */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); /* used to refresh led chain */

	/* set up timebase*/
	/* Tuev= TIM_CLK/((PSC+1)*(ARR+1)*(RCR+1)) */
	TIM_TimeBaseInitTypeDef timInitStruct;
	TIM_TimeBaseStructInit(&timInitStruct); /* init time base to default */

	timInitStruct.TIM_Prescaler = 0;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timInitStruct.TIM_Period = T_TOTAL; /* 72MHz/(PSC=0+1)*1.25us=90 == ARR*/

	TIM_TimeBaseInit(TIM2, &timInitStruct);

	timInitStruct.TIM_Prescaler = 36;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0;
	timInitStruct.TIM_Period = 64850/*64850*/; /* 30Hz refresh rate */

	TIM_TimeBaseInit(TIM3, &timInitStruct);


	/* set up output compare*/
	TIM_OCInitTypeDef OCInitStruct;
	TIM_OCStructInit(&OCInitStruct); /* init to default value*/

	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; /* enable PA1 */

	/* set up timer 3 interupt */
	NVIC_InitTypeDef nvicStructure;

	nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

	/* ********* */

	TIM_OC2Init(TIM2, &OCInitStruct);

	TIM_SetCompare2(TIM2,0);

	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable); /* must be enabled, otherwise glitches */

	TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);

	TIM_Cmd(TIM3, ENABLE); /* enable refreshing of chain */
	/* TIM2 is only enabled when ready to send */


}

void InitDMA(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);


	DMA_InitTypeDef DMAInitStruct;
	DMA_StructInit(&DMAInitStruct);

	/* init TIM2 DMA */
	DMAInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&TIM2->CCR2;
	DMAInitStruct.DMA_MemoryBaseAddr = (uint32_t)buf; /*DMAy_Channelx->CMAR*/
	DMAInitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMAInitStruct.DMA_BufferSize = TIM2_DMA_BUF_LEN;
	DMAInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMAInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMAInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMAInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMAInitStruct.DMA_Mode =DMA_Mode_Circular;
	DMAInitStruct.DMA_Priority = DMA_Priority_High;
	DMAInitStruct.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel7, &DMAInitStruct);
	/* ************** */

	/* init USART1 Rx DMA */
	DMA_StructInit(&DMAInitStruct);

	DMAInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMAInitStruct.DMA_MemoryBaseAddr = (uint32_t)usartBuf;
	DMAInitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMAInitStruct.DMA_BufferSize = USART1_DMA_BUF_LEN;
	DMAInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMAInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMAInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMAInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMAInitStruct.DMA_Mode = DMA_Mode_Circular;
	DMAInitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	DMAInitStruct.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel5, &DMAInitStruct);
	/* ************* */

	/* configure TIM2 DMA IRQ */
	NVIC_InitTypeDef nvicStructure;

	nvicStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvicStructure);
	/* ***************** */

	/* configure USART1 DMA IRQ */
	nvicStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvicStructure);
	/* ***************** */


	/* init TIM2 DMA buffer */
	for(uint16_t i=0;i<TIM2_DMA_BUF_LEN;i++)
		buf[i]=0;

	/* init USART1 DMA buffer */
	for(uint16_t i=0;i<USART1_DMA_BUF_LEN;i++)
		usartBuf[i]=0;

	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE); /* TIM2 */
	DMA_ITConfig(DMA1_Channel7, DMA_IT_HT, ENABLE); /* TIM2 */

	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); /* USART1 Rx */
	DMA_ITConfig(DMA1_Channel5, DMA_IT_HT, ENABLE); /* USART1 Rx */
	DMA_Cmd(DMA1_Channel5, ENABLE);


}

/* send reset synchronously */
void sendReset(void){
/* resetss automatically after sending is finished*/

}

/* clear first nr number leds, equivalent with sendColor(nr_of_leds) with col[i]=0  */
void sendClear(uint16_t nr){
	if(nr==0)
		return;

	TIM_Cmd(TIM2, DISABLE);
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel7,TIM2_DMA_BUF_LEN);

	for(uint16_t i=0;i<nr*24;i++){
		buf[i]=T0H;
	}

	for(uint16_t i=nr*24;i<TIM2_DMA_BUF_LEN;i++){
		buf[i]=0;
	}


	txLen=nr;
	idx=0;


	DMA_Cmd(DMA1_Channel7, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

}



void sendColor(uint16_t nr){
	TIM_Cmd(TIM2, DISABLE);
	DMA_Cmd(DMA1_Channel7, DISABLE);

	TIM_SetCounter(TIM2,0);

	DMA_SetCurrDataCounter(DMA1_Channel7,TIM2_DMA_BUF_LEN);

	uint16_t m=(nr<DOUBLE_BUF_LEN)?nr:DOUBLE_BUF_LEN;

	for(uint16_t i=0;i<m;i++){
		for(uint16_t j=0;j<24;j++){
			buf[23-j+i*24]=(col[i]&(1<<j))?T1H:T0H;
		}
	}

	for(uint16_t i=m*24;i<TIM2_DMA_BUF_LEN;i++){ /* blank out remaining leds */
		buf[i]=0;
	}

	txLen=nr;
	idx=0;

#ifdef DEBUG_LIBWS2811
		GPIOA->ODR ^= 0b100; /* toggle PIN2 for debug purposes */
#endif

	DMA_Cmd(DMA1_Channel7, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	txOn=1;
}


uint32_t packColor(uint8_t r,uint8_t g,uint8_t b){
	return (((uint32_t)r<<8)|((uint32_t)b)|(uint32_t)g<<16)&W;
}

uint32_t packColorRGB(uint8_t r,uint8_t g,uint8_t b){
	return (((uint32_t)r<<16)|((uint32_t)b)|(uint32_t)g<<8)&W;
}


uint8_t gammaCor(uint8_t c){
	return gam[c];
}


























