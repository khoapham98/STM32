#include "main.h"
#include <string.h>
#define GPIOD_BASE_ADDR 0x40020C00
#define GPIOA_BASE_ADDR 0x40020000
#define GPIOB_BASE_ADDR 0x40020400
#define EXTI_BASE_ADDR 0x40013C00

typedef enum{
	GREEN,
	ORANGE,
	RED,
	BLUE
} LED_color;

typedef enum{
	OFF,
	ON
} LED_state;

void LEDsInit()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = GPIOD_BASE_ADDR + 0x00;
	// set PD12, PD13, PD14, PD15 OUTPUT
	*GPIOD_MODER &= ~(0b11111111 << 24);
	*GPIOD_MODER |= (0b01010101 << 24);
}
void ButtonInit()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	uint32_t* GPIOB_MODER = GPIOB_BASE_ADDR + 0x00;
	// set PB4 as INPUT
	*GPIOB_MODER &= ~(0b11 << 8);
}
int isPressed()
{
	uint32_t* GPIOB_IDR = GPIOB_BASE_ADDR + 0x10;

	if (((*GPIOB_IDR >> 4) & 0b1) == 0b1)
	{
		return 1;
	}

	return 0;
}
void ctrlLEDs(LED_color color, LED_state state)
{
	uint32_t* GPIOD_ODR = GPIOD_BASE_ADDR + 0x14;
	if (state == ON)
	{
		*GPIOD_ODR |= (1 << (color + 12));
	}
	else
	{
		*GPIOD_ODR &= ~(1 << (color + 12));
	}
}

void EXTI_Init()
{
	uint32_t* EXTI4_FTSR = EXTI_BASE_ADDR + 0x0C;
	uint32_t* EXTI4_RTSR = EXTI_BASE_ADDR + 0x08;
	*EXTI4_FTSR |= (0b1 << 4);
	*EXTI4_RTSR |= (0b1 << 4);

	uint32_t* EXTI4_IMR = EXTI_BASE_ADDR + 0x00;
	*EXTI4_IMR |= (0b1 << 4);

	uint32_t* NVIC_ISER0 = 0xE000E100;
	*NVIC_ISER0 |= (0b1 << 10);

	uint32_t* VTOR = 0xE000ED08;
}

void EXTI4_IRQHandler()
{
	if (isPressed())
	{
		ctrlLEDs(BLUE, ON);
	}
	else
	{
		ctrlLEDs(BLUE, OFF);
	}

	uint32_t* EXTI4_PR = EXTI_BASE_ADDR + 0x14;
	*EXTI4_PR |= (0b1 << 4);
}

int main()
{
	HAL_Init();
	LEDsInit();
	ButtonInit();
	EXTI_Init();

	void (*fptr)() = EXTI4_IRQHandler;

	while (1)
	{
		ctrlLEDs(RED, ON);
		HAL_Delay(500);
		ctrlLEDs(RED, OFF);
		HAL_Delay(500);
//		if (isPressed())
//		{
//			ctrlLEDs(RED, ON);
//		}
//		else
//		{
//			ctrlLEDs(RED, OFF);
//		}
	}

	return 0;
}
