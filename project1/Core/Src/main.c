#include "main.h"

#define GPIOC_BASE_ADDR 0x40020800
#define TIM2_BASE_ADDR 0x40000000

void HCSR04_Init();
void TIM2_Init();
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void trig_HCSR04();
void cal_distance();

float distance = 0;

int main()
{
	HAL_Init();
	HCSR04_Init();
	TIM2_Init();
	while (1)
	{
		trig_HCSR04();
		cal_distance();
		delay_ms(1000);
	}
	return 0;
}

void cal_distance()
{
	volatile uint32_t* GPIOC_IDR = (uint32_t*) (GPIOC_BASE_ADDR + 0x10);
	while (((*GPIOC_IDR >> 6) & 1) == 0);	// wait until ECHO is HIGH

	volatile uint32_t* TIM2_CNT = (uint32_t*) (TIM2_BASE_ADDR + 0x24);
	*TIM2_CNT = 0;	// set counter counting back from 0
	while (((*GPIOC_IDR >> 6) & 1) == 1);	// Keep counting until ECHO is LOW
	uint32_t time = *TIM2_CNT;	// save the time
	distance = time / 58;	// use equation 3 in datasheet to calculate distance
}

void trig_HCSR04()
{
	uint32_t* GPIOC_ODR = (uint32_t*) (GPIOC_BASE_ADDR + 0x14);
	*GPIOC_ODR |= (1 << 7);		// set TRIG to HIGH
	delay_us(11);
	*GPIOC_ODR &= ~(1 << 7);	// clear TRIG to LOW
}

void delay_us(uint32_t us)
{
	uint32_t* TIM2_CNT = (uint32_t*) (TIM2_BASE_ADDR + 0x24);
	*TIM2_CNT = 0;
	while (*TIM2_CNT < us);
}

void delay_ms(uint32_t ms)
{
	uint32_t* TIM2_CNT = (uint32_t*) (TIM2_BASE_ADDR + 0x24);
	*TIM2_CNT = 0;
	while (*TIM2_CNT < (ms * 1000));
}

void TIM2_Init()
{
	__HAL_RCC_TIM2_CLK_ENABLE();
	uint32_t* TIM2_CR1 = (uint32_t*) (TIM2_BASE_ADDR + 0x00);
	*TIM2_CR1 &= ~(1 << 4);	// up-counter
	*TIM2_CR1 &= ~(1 << 0); // make sure CEN = 0 (stop timer)

	uint32_t* TIM2_PSC = (uint32_t*) (TIM2_BASE_ADDR + 0x28);
	*TIM2_PSC = 16 - 1;		// PSC = N - 1, set CLK_TIM2 = 1 MHz

	uint32_t* TIM2_EGR = (uint32_t*) (TIM2_BASE_ADDR + 0x14);
	*TIM2_EGR |= (1 << 0); 	// enable update generation

	*TIM2_CR1 |= (1 << 0);	// enable counter
}

void HCSR04_Init()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	uint32_t* GPIOC_MODER = (uint32_t*) (GPIOC_BASE_ADDR + 0x00);
	*GPIOC_MODER &= ~(0b1111 << 12);	// set PC6 as INPUT ~ ECHO
	*GPIOC_MODER |= (0b01 << 14);	// set PC7 as OUTPUT ~ TRIG
}

