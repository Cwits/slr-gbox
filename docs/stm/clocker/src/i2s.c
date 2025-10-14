
#include "i2s.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_spi.h"
#include "misc.h"
#include "sample.h"

#define BUFFERSIZE 512

#define dmaStream DMA1_Stream4


uint16_t ourBuffer[BUFFERSIZE*2] = {0};		//dummy buffer
uint16_t scndOurBuffer[BUFFERSIZE*2] = {0};	//dummy buffer
volatile uint8_t bufferFlag = 0;
volatile uint8_t bEmpty = 1;
void swapVoid(uint16_t *ptr)
{
	uint8_t temp1, temp2;

	temp1 = *ptr & 0xff;
	temp2 = (*ptr >> 8) & 0xff;

	*ptr = temp2 | (temp1<<8);


}
void DMA1_Stream4_IRQHandler(void)
{
	if(DMA_GetFlagStatus(dmaStream, DMA_FLAG_TCIF4) != RESET) {
		DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
		bEmpty = 1;
	}

	if(DMA_GetFlagStatus(dmaStream, DMA_FLAG_HTIF4) != RESET) {
		DMA_ClearFlag(dmaStream, DMA_FLAG_HTIF4);
	}
}

static inline void I2S_DMAInit(uint32_t bufferSize)
{
	DMA_InitTypeDef dmaInit;
	NVIC_InitTypeDef nvicInit;

	DMA_DeInit(dmaStream);
	DMA_Cmd(dmaStream, DISABLE);
	while(DMA_GetCmdStatus(dmaStream) == ENABLE){};
	__ISB(); //what is this??

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	//To
	dmaInit.DMA_PeripheralBaseAddr = (uint32_t) &(SPI2->DR);
	//From
	dmaInit.DMA_Memory0BaseAddr = (uint32_t)&rawData[0];
	dmaInit.DMA_BufferSize = 1125;

	dmaInit.DMA_Channel = DMA_Channel_0;
	dmaInit.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//	dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//	dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dmaInit.DMA_Mode = DMA_Mode_Circular;
	dmaInit.DMA_Priority = DMA_Priority_High;
	dmaInit.DMA_FIFOMode = DMA_FIFOMode_Enable;
	dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; //is ir right??
	dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	while(DMA_GetCmdStatus(dmaStream) == ENABLE);

	DMA_Init(dmaStream, &dmaInit);

	DMA_DoubleBufferModeConfig(dmaStream, (uint32_t)&rawData, DMA_Memory_1);
	DMA_ITConfig(dmaStream, DMA_IT_TC | DMA_IT_HT /*| DMA_IT_TE | DMA_IT_DME | DMA_IT_FE*/, ENABLE);
	DMA_DoubleBufferModeCmd(dmaStream, ENABLE);

	//configure interrupt
	nvicInit.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInit.NVIC_IRQChannelSubPriority = 0;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInit);

}

void I2S_PPL_Config(uint32_t freq) {
	RCC_PLLI2SCmd(DISABLE);

	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	//for 44k N-271, R-2, for 48K N-258, R-3
	//N, R, M
	if(freq == 48000)
	{
		RCC_PLLI2SConfig(258, 3, 25);

//		RCC_PLLI2SConfig(289, 4, 21);	good but not enough
	}
	else if(freq == 44100)
	{
//		I2S_PPL_Config(271, 2); //271 2 result in 88,2
		RCC_PLLI2SConfig(271, 2, 25);
//		I2S_PPL_Config(168, 2);
	}
//	RCC_PLLI2SConfig(N, R, 25);
	RCC_PLLI2SCmd(ENABLE);

	while(RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == RESET){};
}

void I2SInit(uint32_t frequency, uint16_t bitRate, uint32_t bufferSize)
{
	GPIO_InitTypeDef gpioInit;
	I2S_InitTypeDef i2sInit;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
	I2S_PPL_Config(frequency);

	//WS - word clock output
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);		//for SPI2 pins: PB12
		GPIO_StructInit(&gpioInit);

		gpioInit.GPIO_Pin = GPIO_Pin_12;
		gpioInit.GPIO_Mode = GPIO_Mode_AF;
		gpioInit.GPIO_OType = GPIO_OType_PP;
		gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
		gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &gpioInit);

		//BCLK - shift clock output
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);		//PB10
		gpioInit.GPIO_Pin = GPIO_Pin_10;
		GPIO_Init(GPIOB, &gpioInit);

		//SD - serial audio data
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);		//PB15
		gpioInit.GPIO_Pin = GPIO_Pin_15;
		GPIO_Init(GPIOB, &gpioInit);

		//MCO - master clock output
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_SPI2);			//PC6
		gpioInit.GPIO_Pin = GPIO_Pin_3;
		GPIO_Init(GPIOA, &gpioInit);

	I2S_StructInit(&i2sInit);

//	i2sInit.I2S_CPOL = I2S_CPOL_Low;
	i2sInit.I2S_CPOL = I2S_CPOL_High;
	i2sInit.I2S_Mode = I2S_Mode_MasterTx;
	i2sInit.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	i2sInit.I2S_Standard = I2S_Standard_Phillips;
	i2sInit.I2S_AudioFreq = (uint32_t)frequency;
	if(bitRate == 16) {
		i2sInit.I2S_DataFormat = I2S_DataFormat_16b;
	} else if(bitRate == 17) {
		i2sInit.I2S_DataFormat = I2S_DataFormat_16bextended;
	} else if(bitRate == 24) {
		i2sInit.I2S_DataFormat = I2S_DataFormat_24b;
	} else if(bitRate == 32) {
		i2sInit.I2S_DataFormat = I2S_DataFormat_32b;
	}
	I2S_Init(SPI2, &i2sInit);										//SPI3		//SPI2
//	if(frequency == 48000) //in case if RCC_PLLI2SConfig(289, 4, 21);
//	{
//		SPI2->I2SPR |= (1 << 8); //ODD and I2SDIV works how???
//	}
	//maybe should do this in different place

	uint32_t * ttestPtr = &rawData[0];
	for(int i=0; i<(1125); ++i)
	{
		uint32_t tmp = *ttestPtr;
		uint16_t tmp1 = tmp & 0xFFFF;
		uint16_t tmp2 = (tmp >> 16) & 0xFFFF;

		uint32_t res = tmp2 | (tmp1 << 16);
		*ttestPtr = res;
		ttestPtr++;
	}

	I2S_DMAInit(bufferSize);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

	//maybe should do this in different place
	I2S_Cmd(SPI2, ENABLE);
	DMA_Cmd(dmaStream, ENABLE);									//DMA1_Stream7	//DMA1_Stream4
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_FEIF4);
	while(DMA_GetCmdStatus(dmaStream) != ENABLE){};
}

