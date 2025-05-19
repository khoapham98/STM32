#include "main.h"
#include <stdlib.h>

#define GPIOD_BASE_ADDR 0x40020C00
#define GPIOA_BASE_ADDR 0x40020000
#define MODER_OFFSET 0x00
#define ODR_OFFSET 0x14
#define IDR_OFFSET 0x10

int arr[40] = {0};
void delay_us(uint32_t x) {
    for (int i = 0; i < (x * 8); i++) {
        __NOP();
    }
}

void DHT11Init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = GPIOD_BASE_ADDR + MODER_OFFSET;
	// set PD2 as OUTPUT to activate DHT11
	*GPIOD_MODER &= ~(0b11 << 4);
	*GPIOD_MODER |= (0b01 << 4);

	uint32_t* GPIOD_ODR = GPIOD_BASE_ADDR + ODR_OFFSET;
	// set PD2 out bit 0 for 20ms to activate DHT11
	*GPIOD_ODR &= ~(0b1 << 2);
	HAL_Delay(20);
	// set PD2 out bit 1 for 30us to end the activate command
	*GPIOD_ODR |= (0b1 << 2);
	delay_us(30);

	// set PD2 as INPUT to receive data from DHT11
	*GPIOD_MODER &= ~(0b11 << 4);

	// wait for 2 ACK phases
	delay_us(135);

	readDHT11(arr, 40);
}

int read_one_bit()
{
	uint32_t* GPIOD_IDR = GPIOD_BASE_ADDR + IDR_OFFSET;

	//wait for low
	while (((*GPIOD_IDR >> 2) & 0b1) == 0b1);

	// wait for high
	while (((*GPIOD_IDR >> 2) & 0b1) == 0b0);

	delay_us(30);

	return (((*GPIOD_IDR >> 2) & 0b1) == 0b1) ? 1 : 0;
}

void readDHT11(int* arr, int size)
{
	for (int i = 0; i < 40; i++) {
		int bit = read_one_bit();
		if (bit < 0) return;
		arr[i] = bit;
	}
}


int main()
{
	HAL_Init();
	DHT11Init();



	return 0;
}
