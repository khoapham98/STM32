#include "main.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void UART_Init();
void button_Init();
char isPressed();
void USART_send(uint8_t data);
void USART_send_string(char* str, ...);
void DHT11_Init();
void request();
void response();
void DWT_Init(void);
void DWT_Delay_us(volatile uint32_t microseconds);
uint8_t receive_data();

int main()
{
	HAL_Init();
	UART_Init();
	button_Init();
	DHT11_Init();
	DWT_Init();
	uint8_t data[5] = {0};

	while (1)
	{
		// send start signal
		request();
		// wait for response
		response();
		// read data
		for (int i = 0; i < 5; i++)
		{
			data[i] = receive_data();
		}
		// check sum
		if ((data[0] + data[1] + data[2] + data[3]) != data[4])
		{
			USART_send_string("ERROR!\n");
		}
		else
		{
			USART_send_string("Humidity: %u,%u, ", data[0], data[1]);
			USART_send_string("Temperature: %u,%u\n", data[2],data[3]);
		}
		HAL_Delay(2000);
	}
	return 0;
}

uint8_t receive_data()
{
	uint8_t byte = 0;
	uint32_t* GPIOA_IDR = (uint32_t*) (GPIOA_BASE + 0x10);
	for (int i = 0; i < 8; i++)
	{
		uint32_t timeout = 100;
		while (((*GPIOA_IDR >> 9) & 1) == 1 && timeout--)
		{
//			DWT_Delay_us(1);
		}
		if (timeout == 0) {return 0xff;}


		timeout = 100;
		while (((*GPIOA_IDR >> 9) & 1) == 0 && timeout--)
		{
//			DWT_Delay_us(1);
		}
		if (timeout == 0) {return 0xff;}

		DWT_Delay_us(35);
		if (((*GPIOA_IDR >> 9) & 1) == 1)
		{
			byte = (byte << 1) | 0x01;
		}
		else
		{
			byte = byte << 1;
		}

		timeout = 100;
		while (((*GPIOA_IDR >> 9) & 1) == 1 && timeout--)
		{
			DWT_Delay_us(1);
		}
		if (timeout == 0) {return 0xff;}
	}
	return byte;
}

void response()
{
	uint32_t* GPIOA_IDR = (uint32_t*) (GPIOA_BASE + 0x10);
	uint32_t timeout = 1000;
	while (((*GPIOA_IDR >> 9) & 1) == 1 && timeout--) { DWT_Delay_us(1); }
	if (timeout == 0)
	{
		USART_send_string("DHT11 not response!\n");
		return;
	}

	timeout = 1000;
	while (((*GPIOA_IDR >> 9) & 1) == 0 && timeout--) { DWT_Delay_us(1); }
	if (timeout == 0)
	{
		USART_send_string("DHT11 not response!\n");
		return;
	}
}

void request()
{
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE + 0x00);
	*GPIOA_MODER &= ~(0b11 << 18);
	*GPIOA_MODER |= (0b01 << 18);	// set pin PA9 as OUTPUT

	uint32_t* GPIOA_ODR = (uint32_t*) (GPIOA_BASE + 0x14);

	// set PA9 to LOW
	*GPIOA_ODR &= ~(1 << 9);
	HAL_Delay(20);

	// set PA9 to HIGH
	*GPIOA_ODR |= (1 << 9);
	DWT_Delay_us(30);

	*GPIOA_MODER &= ~(0b11 << 18);	// set pin PA9 as INPUT
}

void DHT11_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
}

void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Bật Trace
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Enable DWT
}

void DWT_Delay_us(volatile uint32_t microseconds)
{
    uint32_t clk_cycle_start = DWT->CYCCNT;
    microseconds *= (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}


void USART_send_string(char* str, ...)
{
	va_list list;
	va_start(list, str);
	char print_buf[128] = { 0 };
	vsprintf(print_buf, str, list);
	int len = strlen(print_buf);
	for(int i = 0; i < len; i++)
	{
		USART_send(print_buf[i]);
	}
	va_end(list);
}

void USART_send(uint8_t data)
{
	uint32_t* USART6_DR = (uint32_t*) (USART6_BASE + 0x04);
	uint32_t* USART6_SR = (uint32_t*) (USART6_BASE + 0x00);
	*USART6_DR = data;
	while (((*USART6_SR >> 7) & 1) == 0);
}

char isPressed()
{
	uint32_t* GPIOA_IDR = (uint32_t*) (GPIOA_BASE + 0x10);
	if ((*GPIOA_IDR & 1) == 1) { return 1;}
	return 0;
}

void button_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE + 0x00);
	*GPIOA_MODER &= ~(0b11 << 0);
}

void UART_Init()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	uint32_t* GPIOC_MODER = (uint32_t*) (GPIOC_BASE + 0x00);
	uint32_t* GPIOC_AFRL = (uint32_t*) (GPIOC_BASE + 0x20);
	*GPIOC_MODER &= ~(0b1111 << 12);
	*GPIOC_MODER |= (0b1010 << 12);
	*GPIOC_AFRL &= ~(0xff << 24);
	*GPIOC_AFRL = (0b1000 << 24) | (0b1000 << 28);

	__HAL_RCC_USART6_CLK_ENABLE();
	uint32_t* USART6_CR1 = (uint32_t*) (USART6_BASE + 0x0C);
	uint32_t* USART6_BRR = (uint32_t*) (USART6_BASE + 0x08);
	*USART6_CR1 &= ~(0b1 << 12); // set word length = 8 data bits
	*USART6_CR1 &= ~(0b1 << 10); // disable parity control
	*USART6_CR1 |= (0b11 << 2);  // enable transmitter and receiver
	*USART6_BRR = (104 << 4) | (3 << 0); // set baud rate = 9600bps
	*USART6_CR1 |= (0b1 << 13);  // enable USART
}
