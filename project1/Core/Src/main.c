#include "main.h"
#define TIM1_BASE_ADDR 0x40010000
void LED_Ctrl(char on);
void LED_Init();
void TIM1_Init();
void delay_second(uint32_t time);
void delay_microsecond(uint32_t time);
void delay_milisecond(uint32_t time);
void TIM1_UP_TIM10_IRQHandler();
void INT_delay_second(uint32_t time);
void systick_init();
void systick_delay_1s();
uint32_t cnt = 0;

int main()
{
	LED_Init();
	TIM1_Init();
	systick_init();
	while (1)
	{
		LED_Ctrl(1);
		systick_delay_1s();
//		INT_delay_second(2);
		LED_Ctrl(0);
		systick_delay_1s();
//		INT_delay_second(4);
	}
	return 0;
}

void systick_delay_1s()
{
	uint32_t* STK_CTRL  = (uint32_t*) (0xE000E010);
//	*STK_CTRL &= ~(1 << 16);
	while (((*STK_CTRL >> 16) & 1) == 0);
	*STK_CTRL &= ~(1 << 16);
}

void systick_init()
{
	uint32_t* STK_CTRL  = (uint32_t*) (0xE000E010);
	uint32_t* STK_LOAD  = (uint32_t*) (0xE000E014);
	/* select clock = 16MHz & enable interrupt */
	*STK_CTRL |= 1 << 1;
	/* reload every 1 second */
	*STK_LOAD = 2000 * 1000 - 1;
	/* enable counter */
	*STK_CTRL |= 1 << 0;
}

void INT_delay_second(uint32_t time)
{
	cnt = 0;
	while (cnt < time);
}

void TIM1_UP_TIM10_IRQHandler()
{
	uint16_t* TIM1_SR = (uint16_t*) (TIM1_BASE_ADDR + 0x10);
	*TIM1_SR &= ~(1 << 0);
	cnt++;
}

/* min: 1us -> max: 4095us */
void delay_microsecond(uint32_t time)
{
	if (time == 0) return;
	uint16_t* TIM1_SR = (uint16_t*) (TIM1_BASE_ADDR + 0x10);
	uint16_t* TIM1_EGR = (uint16_t*) (TIM1_BASE_ADDR + 0x14);
	uint16_t* TIM1_ARR = (uint16_t*) (TIM1_BASE_ADDR + 0x2C);
	uint16_t* TIM1_PSC = (uint16_t*) (TIM1_BASE_ADDR + 0x28);
	uint16_t* TIM1_CNT = (uint16_t*) (TIM1_BASE_ADDR + 0x24);
	*TIM1_ARR = time * 16;
	*TIM1_PSC = 1 - 1;
	*TIM1_CNT = 0;
	*TIM1_EGR |= 1 << 0;
	*TIM1_SR &= ~(1 << 0);
	while ((*TIM1_SR & 1) == 0);
	*TIM1_SR &= ~(1 << 0);
}

/* min: 1ms -> max: 131ms */
void delay_milisecond(uint32_t time)
{
	if (time == 0) return;
	uint16_t* TIM1_SR = (uint16_t*) (TIM1_BASE_ADDR + 0x10);
	uint16_t* TIM1_EGR = (uint16_t*) (TIM1_BASE_ADDR + 0x14);
	uint16_t* TIM1_ARR = (uint16_t*) (TIM1_BASE_ADDR + 0x2C);
	uint16_t* TIM1_PSC = (uint16_t*) (TIM1_BASE_ADDR + 0x28);
	uint16_t* TIM1_CNT = (uint16_t*) (TIM1_BASE_ADDR + 0x24);
	if (time <= 65)
	{
		*TIM1_ARR = time * 1000;
		*TIM1_PSC = 16 - 1;
	}
	else if (time > 65)
	{
		*TIM1_ARR = (time / 2) * 1000;
		*TIM1_PSC = 32 - 1;
	}
	*TIM1_CNT = 0;
	*TIM1_EGR |= 1 << 0;
	*TIM1_SR &= ~(1 << 0);
	while ((*TIM1_SR & 1) == 0);
	*TIM1_SR &= ~(1 << 0);
}

/* min: 1s -> max: 131s */
void delay_second(uint32_t time)
{
	if (time == 0) return;
	uint16_t* TIM1_SR = (uint16_t*) (TIM1_BASE_ADDR + 0x10);
	uint16_t* TIM1_EGR = (uint16_t*) (TIM1_BASE_ADDR + 0x14);
	uint16_t* TIM1_ARR = (uint16_t*) (TIM1_BASE_ADDR + 0x2C);
	uint16_t* TIM1_PSC = (uint16_t*) (TIM1_BASE_ADDR + 0x28);
	if (time <= 65)
	{
		*TIM1_ARR = time * 1000;
		*TIM1_PSC = 16000 - 1;
	}
	else if (time > 65)
	{
		*TIM1_ARR = (time / 2) * 1000;
		*TIM1_PSC = 32000 - 1;
	}
	*TIM1_EGR |= 1 << 0;
	*TIM1_SR &= ~(1 << 0);
	while ((*TIM1_SR & 1) == 0);
	*TIM1_SR &= ~(1 << 0);
}

void TIM1_Init()
{
	__HAL_RCC_TIM1_CLK_ENABLE();
	uint16_t* TIM1_CR1  = (uint16_t*) (TIM1_BASE_ADDR + 0x00);
	uint16_t* TIM1_DIER = (uint16_t*) (TIM1_BASE_ADDR + 0x0C);
	uint16_t* TIM1_ARR = (uint16_t*) (TIM1_BASE_ADDR + 0x2C);
	uint16_t* TIM1_PSC = (uint16_t*) (TIM1_BASE_ADDR + 0x28);
	uint32_t* NVIC_ISER0 = (uint32_t*) (0xE000E100);
	*TIM1_ARR = 1000;
	*TIM1_PSC = 16000 - 1;
	/* enable interrupt */
	*TIM1_DIER |= 1 << 0;
	*NVIC_ISER0 |= 1 << 25;
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
