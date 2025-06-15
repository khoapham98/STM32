#include "main.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GPIOC_BASE_ADDR 0x40020800
#define GPIOD_BASE_ADDR 0x40020C00
#define TIM2_BASE_ADDR 0x40000000
#define USART2_BASE_ADDR 0x40004400

void HCSR04_Init();
void USART2_Init();
void TIM2_Init();
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void trig_HCSR04();
void cal_distance();
void UART_send_char(char data);
void UART_send_string(char* str, ...);

uint32_t distance = 0;

int main()
{
	HAL_Init();
	HCSR04_Init();
	TIM2_Init();
	USART2_Init();
	while (1)
	{
		trig_HCSR04();
		cal_distance();
		UART_send_string("distance = %u cm\n", distance);
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

void UART_send_string(char* str, ...)
{
	va_list list;
	va_start(list, str);
	char print_buf[128] = { 0 };
	vsprintf(print_buf, str, list);
	int size = strlen(print_buf);
	for (int i = 0; i < size; i++)
	{
		UART_send_char(print_buf[i]);
	}
	va_end(list);
}

void UART_send_char(char data)
{
	uint32_t* USART2_DR = (uint32_t*) (USART2_BASE_ADDR + 0x04);
	*USART2_DR = data;

	uint32_t* USART2_SR = (uint32_t*) (USART2_BASE_ADDR + 0x00);
	while (((*USART2_SR >> 7) & 1) == 0);	// wait until the data is transferred to the Shift Register
}

void USART2_Init()
{
	/* USART2 - AF07 ~ PD5 - TX , PD6 - RX */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = (uint32_t*) (GPIOD_BASE_ADDR + 0x00);
	uint32_t* GPIOD_AFRL = (uint32_t*) (GPIOD_BASE_ADDR + 0x20);
	*GPIOD_MODER &= ~(0xf << 10);
	*GPIOD_MODER |= (0b1010 << 10); // configure PD5 and PD6 at AF mode
	*GPIOD_AFRL &= ~(0xff << 20);
	*GPIOD_AFRL |= (7 << 20) | (7 << 24);	// AF07 for PD5 and PD6

	__HAL_RCC_USART2_CLK_ENABLE();
	uint32_t* USART2_CR1 = (uint32_t*) (USART2_BASE_ADDR + 0x0C);
	uint32_t* USART2_BRR = (uint32_t*) (USART2_BASE_ADDR + 0x08);
	*USART2_CR1 |= (1 << 12);	// set word length = 9 data bits
	*USART2_CR1 |= (1 << 10); 	// enable parity control
	*USART2_CR1 |= (1 << 9); 	// select odd parity
	*USART2_CR1 |= (1 << 3); 	// enable transmitter
	*USART2_BRR = (69 << 4) | (0b0111 << 0);	// set baud rate at 14400 bps
	*USART2_CR1 |= (1 << 13); 	// enable USART
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
