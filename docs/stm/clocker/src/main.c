#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "i2s.h"

int main(void)
{
  int i = 0;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitTypeDef gpio;
  gpio.GPIO_Pin = GPIO_Pin_13;	//blue led
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
  gpio.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOC, &gpio);

  I2SInit(44100, 24, 512);
  //GPIOB power enabled in I2SInit

  gpio.GPIO_Pin = GPIO_Pin_0;
  gpio.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOB, &gpio);

  gpio.GPIO_Pin = GPIO_Pin_1;
  GPIO_Init(GPIOB, &gpio);
  gpio.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOB, &gpio);
  gpio.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOB, &gpio);
  gpio.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOB, &gpio);
  gpio.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &gpio);

//  GPIOB->ODR &= ~GPIO_Pin_0;	//Audio PND On
//  GPIOB->ODR &= ~GPIO_Pin_1;	//+48 PND Off
//  GPIOB->ODR |= GPIO_Pin_2;	//RST On(High = enable)
//  GPIOB->ODR &= ~GPIO_Pin_3;	//FS0 = low
//  GPIOB->ODR &= ~GPIO_Pin_4;	//FS1 = low
//  GPIOB->ODR &= ~GPIO_Pin_5;	//Mute = low

  while (1)
  {
	i++;
	if(i == 30000000)
	{
		GPIOC->ODR ^= GPIO_Pin_13;
		i = 0;
	}
  }
}
