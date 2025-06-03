#include "main.h"

#define GPIOD_BASE_ADDR 0x40020C00
#define GPIOA_BASE_ADDR 0x40020000
#define EXTI_BASE_ADDR 0x40013C00

typedef enum{
	OFF, ON
} LED_state;

void LED_Init();
void LED_ctrl(LED_state state);
void myDelay();
void custom_Handler();
void EXTI_Init();

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

void custom_Handler()
{
	// Reset the PR Register to turn off the interrupt flag
	uint32_t* EXTI_PR = (uint32_t*) (EXTI_BASE_ADDR + 0x14);
	*EXTI_PR |= (0b1 << 0);

	// Turn off Red LED before jump
	LED_ctrl(OFF);

	// Jump to Reset Handler function of App2
	uint32_t* ptr = (uint32_t*) 0x08008004;
	void (*pf)() = (void (*)()) *ptr;

	pf();
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

	memcpy(0x20000000, 0x08004000, 0x198);

	uint32_t* VTOR = (uint32_t*) 0xE000ED08;
	*VTOR = 0x20000000;

	uint32_t* fp = (uint32_t*) 0x20000058;
	*fp = custom_Handler;
}

void LED_Init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	// Declare GPIOD's MODER Register
	uint32_t* GPIOD_MODER = (uint32_t*) (GPIOD_BASE_ADDR + 0x00);
	// Clear pin PD14
	*GPIOD_MODER &= ~(0b11 << 28);
	// Set pin PD14 as OUTPUT
	*GPIOD_MODER |= (0b01 << 28);
}

void LED_ctrl(LED_state state)
{
	// Declare GPIOD's ODR Register
	uint32_t* GPIOD_ODR = (uint32_t*) (GPIOD_BASE_ADDR + 0x14);
	// Set output state for pin PD14
	if (state == ON)
	{
		*GPIOD_ODR |= (0b1 << 14);
	}
	else if (state == OFF)
	{
		*GPIOD_ODR &= ~(0b1 << 14);
	}
}

void myDelay()
{
	volatile int x = 0;
	for (int i = 0; i < 1e6; i++) {
		x++;
	}

	return;
}
