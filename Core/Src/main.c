#include "main.h"
#include <string.h>
#define GPIOD_BASE_ADDR 0x40020C00
#define GPIOA_BASE_ADDR 0x40020000
#define MODER_OFFSET 0x00
#define ODR_OFFSET 0x14
#define IDR_OFFSET 0x10
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

void EXTI0_IRQHandler()
{
	if (isPressed() == 1)
	{
		ctrlLEDs(BLUE, ON);
	}
	else
	{
		ctrlLEDs(BLUE, OFF);
	}

	uint32_t* EXTI_PR = EXTI_BASE_ADDR + 0x14;
	*EXTI_PR |= (0b1 << 0);
}

void custom_Handler()
{
	if (isPressed())
	{
		ctrlLEDs(GREEN, ON);
	}
	else
	{
		ctrlLEDs(GREEN, OFF);
	}

	uint32_t* EXTI_PR = EXTI_BASE_ADDR + 0x14;
	*EXTI_PR |= (0b1 << 0);
}
void EXTI_Init()
{
	// B1
	uint32_t* EXTI_RTSR = EXTI_BASE_ADDR + 0x08;
	uint32_t* EXTI_FTSR = EXTI_BASE_ADDR + 0x0C;
	*EXTI_RTSR |= (0b1 << 0);
	*EXTI_FTSR |= (0b1 << 0);

	// B2
	uint32_t* EXTI_IMR = EXTI_BASE_ADDR + 0x00;
	*EXTI_IMR |= (0b1 << 0);

	// B3
	uint32_t* NVIC_ISER0 = 0xE000E100;
	*NVIC_ISER0 |= (0b1 << 6);

	// copy data of vector table from FLASH to RAM so that we'll be able to R&W the data
	memcpy(0x20000000, 0x00, 0x198);

	// Let ARM know that the vector table has been offset into RAM
	uint32_t* VTOR = 0xE000ED08;
	*VTOR = 0x20000000; 			// start addr of RAM

	// the memory area contains the address of the default interrupt function (EXTI0_IRQHandler): 0x58
	// Because the vector table has been copied to RAM:
	// => the memory area contains the address of the EXTIO_IRQHandler function: 0x2000 0058
	uint32_t* fptr = 0x20000058;
	*fptr = custom_Handler; // change the default interrupt function (EXTIO_IRQHandler) to my own interrupt function (custom_Handler)
}

void LEDsInit()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = GPIOD_BASE_ADDR + MODER_OFFSET;
	// set PD12, PD13, PD14, PD15 OUTPUT
	*GPIOD_MODER &= ~(0b11111111 << 24);
	*GPIOD_MODER |= (0b01010101 << 24);
}
void ButtonInit()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* GPIOA_MODER = GPIOA_BASE_ADDR + MODER_OFFSET;
	// set PA0 as INPUT
	*GPIOA_MODER &= ~(0b11);
}

int isPressed()
{
	uint32_t* GPIOA_IDR = GPIOA_BASE_ADDR + IDR_OFFSET;

	if ((*GPIOA_IDR & 0b1) == 0b1)
	{
		return 1;
	}

	return 0;
}
void ctrlLEDs(LED_color color, LED_state state)
{
	uint32_t* GPIOD_ODR = GPIOD_BASE_ADDR + ODR_OFFSET;
	if (state == ON)
	{
		*GPIOD_ODR |= (1 << (color + 12));
	}
	else
	{
		*GPIOD_ODR &= ~(1 << (color + 12));
	}
}


int main()
{
	HAL_Init();
	LEDsInit();
	ButtonInit();
	EXTI_Init();

	while (1)
	{
		ctrlLEDs(RED, ON);
		HAL_Delay(500);
		ctrlLEDs(RED, OFF);
		HAL_Delay(500);
	}

	return 0;
}
