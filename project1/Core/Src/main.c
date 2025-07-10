#include "main.h"
#define TIM1_BASE_ADDR 0x40010000
void LED_Ctrl(char on);
void LED_Init();
void TIM1_Init();
void delay_1s();

int main()
{
	LED_Init();
	TIM1_Init();
	while (1)
	{
		LED_Ctrl(1);
		delay_1s();
		LED_Ctrl(0);
		delay_1s();
	}
	return 0;
}

void delay_1s()
{
	uint16_t* TIM1_SR = (uint16_t*) (TIM1_BASE_ADDR + 0x10);
	uint16_t* TIM1_CNT = (uint16_t*) (TIM1_BASE_ADDR + 0x24);
	*TIM1_CNT = 0;
	while ((*TIM1_SR & 1) == 0);
	*TIM1_SR &= ~(1 << 0);
}

void TIM1_Init()
{
	__HAL_RCC_TIM1_CLK_ENABLE();
	uint16_t* TIM1_CR1 = (uint16_t*) (TIM1_BASE_ADDR + 0x00);
	uint16_t* TIM1_PSC = (uint16_t*) (TIM1_BASE_ADDR + 0x28);
	uint16_t* TIM1_ARR = (uint16_t*) (TIM1_BASE_ADDR + 0x2C);
	/* set ARR */
	*TIM1_ARR = 1000;
	/* set prescaler */
	*TIM1_PSC = 15999;
	/* counter enable */
	*TIM1_CR1 |= 1 << 0;
}

void LED_Init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = (uint32_t*) (GPIOD_BASE + 0x00);
	*GPIOD_MODER &= ~(0b11 << (15 * 2));
	*GPIOD_MODER |= (0b01 << (15 * 2));
}

void LED_Ctrl(char on)
{
	uint32_t* GPIOD_ODR = (uint32_t*) (GPIOD_BASE + 0x14);
	if (on)
	{
		*GPIOD_ODR |= 1 << 15;
	}
	else
	{
		*GPIOD_ODR &= ~(1 << 15);
	}
}
