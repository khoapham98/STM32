#include "main.h"

#define GPIOD_BASE_ADDR 0x40020C00
#define GPIOA_BASE_ADDR 0x40020000
#define MODER_OFFSET 0x00
#define ODR_OFFSET 0x14
#define IDR_OFFSET 0x10

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
char isPressed()
{
	uint32_t* GPIOA_IDR = GPIOA_BASE_ADDR + IDR_OFFSET;

	if ((*GPIOA_IDR & 0b1) == 1)
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

#define EXTI_BASE_ADDR 0x40013C00
void EXTI0Init()
{
// set Rising and falling register enable
	uint32_t* EXTI_RTSR = EXTI_BASE_ADDR + 0x08;
	uint32_t* EXTI_FTSR = EXTI_BASE_ADDR + 0x0C;
	*EXTI_RTSR |= 0b1;
	*EXTI_FTSR |= 0b1;

// set Interrupt Mask register enable
	uint32_t* EXTI_IMR = EXTI_BASE_ADDR + 0x00;
	*EXTI_IMR |= 0b1;

// set NVIC_ISER0 register bit 6
	uint32_t* NVIC_ISER0 = 0xE000E100;
	*NVIC_ISER0 |= (0b1 << 6);
}

void EXTI0_IRQHandler()
{
	if (isPressed())
	{
		while(isPressed())
		{
			ctrlLEDs(BLUE, ON);
		}
		ctrlLEDs(BLUE, OFF);
	}
//	else
//	{
//		ctrlLEDs(BLUE, OFF);
//	}
	uint32_t* EXTI_PR = EXTI_BASE_ADDR + 0x14;
	*EXTI_PR |= 0b1;
}

int cnt, time;
int main()
{
	HAL_Init();
	LEDsInit();
	ButtonInit();
	EXTI0Init();

	while (1)
	{
		ctrlLEDs(RED, ON);
		HAL_Delay(1000);
		ctrlLEDs(RED, OFF);
		HAL_Delay(1000);
	}
	return 0;
}
