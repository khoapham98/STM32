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

int cnt, time;
int main()
{
	HAL_Init();
	LEDsInit();
	ButtonInit();
	LED_state state = OFF;
	ctrlLEDs(RED, state);
	ctrlLEDs(BLUE, state);

	while (1)
	{
		cnt = 0;
		while (isPressed())
		{
			HAL_Delay(50);
			cnt++;
		}

		if (cnt > 0 && cnt <= 6)
		{
			time++;
			if (time == 1)
			{
				ctrlLEDs(RED, ON);
			}
			else if (time == 3)
			{
				ctrlLEDs(RED, OFF);
				time = 0;
			}
		}
		else if (cnt > 6)
		{
			state = (state == ON) ? OFF : ON;
			ctrlLEDs(BLUE, state);
		}

	}

	return 0;
}
