#include "main.h"
#include <strings.h>
#define GPIOD_BASE_ADDR 0x40020C00
#define GPIOA_BASE_ADDR 0x40020000
#define EXTI_BASE_ADDR 0x40013C00

typedef enum{
	OFF, ON
} LED_state;

void LED_Init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	// Declare GPIOD's MODER Register
	uint32_t* GPIOD_MODER = (uint32_t*) (GPIOD_BASE_ADDR + 0x00);
	// Clear pin PD15
	*GPIOD_MODER &= ~(0b11 << 30);
	// Set pin PD15 as OUTPUT
	*GPIOD_MODER |= (0b01 << 30);
}

void LED_ctrl(LED_state state)
{
	// Declare GPIOD's ODR Register
	uint32_t* GPIOD_ODR = (uint32_t*) (GPIOD_BASE_ADDR + 0x14);
	// Set output state for pin PD15
	if (state == ON)
	{
		*GPIOD_ODR |= (0b1 << 15);
	}
	else if (state == OFF)
	{
		*GPIOD_ODR &= ~(0b1 << 15);
	}
}

void EXTI_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	// Declare GPIOA's MODER Register
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE_ADDR + 0x00);

	// Set pin PA0 as INPUT
	*GPIOA_MODER &= ~(0b11 << 0);

	// Declare EXTI line 0's FTSR Register and set to HIGH
	uint32_t* EXTI_FTSR = (uint32_t*) (EXTI_BASE_ADDR + 0x0C);
	*EXTI_FTSR |= (0b1 << 0);

	// Declare EXTI line 0's IMR Register and set to HIGH
	uint32_t* EXTI_IMR = (uint32_t*) (EXTI_BASE_ADDR + 0x00);
	*EXTI_IMR |= (0b1 << 0);

	// Set the NVIC ISER0 at position 6 to HIGH
	uint32_t* NVIC_ISER0 = (uint32_t*) 0xE000E100;
	*NVIC_ISER0 |= (0b1 << 6);
}

void EXTI0_IRQHandler()
{
	// Reset the PR Register to turn off the interrupt flag
	uint32_t* EXTI_PR = (uint32_t*) (EXTI_BASE_ADDR + 0x14);
	*EXTI_PR |= (0b1 << 0);

	// Turn off Blue LED before jump
	LED_ctrl(OFF);

	// Let ARM know that the vector table has been moved to 0x0800 4000 (beginning off App1 firmware)
	uint32_t* VTOR = (uint32_t*) 0xE000ED08;
	*VTOR = 0x08004000;

	// Jump to Reset Handler function of App1
	uint32_t* ptr = (uint32_t*) 0x08004004;
	void (*pf)() = (void (*)()) *ptr;

	pf();
}

void myDelay()
{
	volatile int x = 0;
	for (int i = 0; i < 1e6; i++) {
		x++;
	}

	return;
}

int main()
{
	HAL_Init();
	LED_Init();
	EXTI_Init();

	while (1)
	{
		LED_ctrl(ON);
		myDelay();
		LED_ctrl(OFF);
		myDelay();
	}

	return 0;
}
