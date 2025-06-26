#include "main.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define USART1_BASE_ADDR 0x40011000
#define GPIOB_BASE_ADDR 0x40020400
#define GPIOD_BASE_ADDR 0x40020C00
#define DMA2_BASE_ADDR 0x40026400
#define FLASH_R_BASE_ADDR 0x40023C00

typedef enum{
	GREEN, ORANGE, RED, BLUE, DATA, UPDATE, NONE
} cmd_t;

typedef enum{
	OFF, ON
} state_t;

char rx_buf[6000] = {0};
int _index = 0;
char* color[4] = { "Green", "Orange", "Red", "Blue" };
char* s[2] = { "OFF", "ON" };
char recv_new_fw_complete = 0;

void USART1_Init();
void USART_send_char(char data);
void USART_send_string(char* str, ...);
void LEDs_Init();
void ctrl_LED(cmd_t color, state_t state);
cmd_t get_data();
state_t get_state();
char is_done();
void USART1_IRQHandler();
void DMA2_Init();
void DMA2_Stream2_IRQHandler();
void flash_erase_sector(int sec_num);
void flash_program(uint8_t* addr, uint8_t val);
void unlock_flash_CR();
void update(int fw_size);
void clear_rx_buf();
int get_size();

int main()
{
	HAL_Init();
	USART1_Init();
	LEDs_Init();
	DMA2_Init();
	USART_send_string("Day la firmware version 0.0\n");
	while(1)
	{
		if (is_done())
		{
			if (get_data() == UPDATE)
			{
				clear_rx_buf();
				USART_send_string("Enter the size of firmware (byte): ");
				while (!is_done());
				int fw_size = get_size();
				if (fw_size > sizeof(rx_buf))
				{
					USART_send_string("The size of this firmware is too large\n");
					break;
				}
				clear_rx_buf();
				if (rx_buf[fw_size / 2] == 0)
				{
					USART_send_string("Please send the firmware to complete the upload process!\n");
					while (rx_buf[fw_size - 61] == 0);
					USART_send_string("Firmware upload successfully!\n");
				}
				update(fw_size);
			}
			else if (get_data() != NONE)
			{
				ctrl_LED(get_data(), get_state());
				USART_send_string("%s LED is %s\n", color[get_data()], s[get_state()]);
				clear_rx_buf();
			}
		}
	}
	return 0;
}

int get_size()
{
	int index = 0;
	int size = 0;
	while (rx_buf[index] >= '0' && rx_buf[index] <= '9')
	{
		size *= 10;
		size += (rx_buf[index++] - 48);
	}
	return size;
}

void clear_rx_buf()
{
	memset(rx_buf, 0, sizeof(rx_buf));
	uint32_t* DMA_S2M0AR = (uint32_t*) (DMA2_BASE_ADDR + 0x1C + 0x18 * 2);
	uint32_t* DMA_S2CR = (uint32_t*) (DMA2_BASE_ADDR + 0x10 + 0x18 * 2);
	*DMA_S2CR &= ~(1 << 0);	// stream disabled
	*DMA_S2M0AR = (uint32_t) rx_buf;	// reset memory 0 address
	*DMA_S2CR |= 1 << 0;	// stream enabled
}

__attribute__((section(".ham_tren_RAM"))) void update(int fw_size)
{
	__asm("CPSID i");
	flash_erase_sector(0);
	for (int i = 0; i < fw_size; i++)
	{
		flash_program(0x08000000 + i, rx_buf[i]);
	}
	uint32_t* AIRCR = (uint32_t*) 0xE000ED0C;
	*AIRCR = (0x5FA << 16) | (1 << 2);
}

__attribute__((section(".ham_tren_RAM"))) void unlock_flash_CR()
{
	uint32_t* FLASH_CR = (uint32_t*) (FLASH_R_BASE_ADDR + 0x10);
	uint32_t* FLASH_KEYR = (uint32_t*) (FLASH_R_BASE_ADDR + 0x04);

	if (((*FLASH_CR >> 31) & 1) == 1)
	{
		*FLASH_KEYR = 0x45670123;	// unlock the FLASH CR
		*FLASH_KEYR = 0xCDEF89AB;
	}
}

__attribute__((section(".ham_tren_RAM"))) void flash_erase_sector(int sec_num)
{
	if (sec_num > 7) { return; }
	unlock_flash_CR();

	uint32_t* FLASH_SR = (uint32_t*) (FLASH_R_BASE_ADDR + 0x0C);
	while (((*FLASH_SR >> 16) & 1) == 1);	// check the BSY bit

	uint32_t* FLASH_CR = (uint32_t*) (FLASH_R_BASE_ADDR + 0x10);
	*FLASH_CR |= (1 << 1);	// set the SER bit
	*FLASH_CR |= (sec_num << 3);	// select the section as sec_num
	*FLASH_CR |= (1 << 16);	// set the STRT bit
	while (((*FLASH_SR >> 16) & 1) == 1);	// wait for BSY bit to be cleared
}

__attribute__((section(".ham_tren_RAM"))) void flash_program(uint8_t* addr, uint8_t val)
{
	unlock_flash_CR();

	uint32_t* FLASH_SR = (uint32_t*) (FLASH_R_BASE_ADDR + 0x0C);
	while (((*FLASH_SR >> 16) & 1) == 1);	// check the BSY bit

	uint32_t* FLASH_CR = (uint32_t*) (FLASH_R_BASE_ADDR + 0x10);
	*FLASH_CR |= (1 << 0);	// set the PG bit
	*addr = val;
	while (((*FLASH_SR >> 16) & 1) == 1);	// wait for BSY bit to be cleared
}

char is_done()
{
	int size = sizeof(rx_buf);
	for (int i = 0; i < size; i++)
	{
		if (rx_buf[i] == '\n') { return 1; }
	}
	return 0;
}

void DMA2_Stream2_IRQHandler()
{
	recv_new_fw_complete = 1;
	uint32_t* DMA2_LIFCR = (uint32_t*) (DMA2_BASE_ADDR + 0x08);
	*DMA2_LIFCR |= (1 << 21);	// clear transfer complete interrupt flag
}
void USART1_IRQHandler()
{
	uint32_t* USART1_DR = (uint32_t*) (USART1_BASE_ADDR + 0x04);
	rx_buf[_index++] = *USART1_DR;
}

cmd_t get_data()
{
	if (strstr(rx_buf, "green") != NULL) { return GREEN;}
	else if (strstr(rx_buf, "orange") != NULL) { return ORANGE; }
	else if (strstr(rx_buf, "red") != NULL) { return RED; }
	else if (strstr(rx_buf, "blue") != NULL) { return BLUE; }
	else if (strstr(rx_buf, "update") != NULL) { return UPDATE; }
	else { return NONE; }
}

state_t get_state()
{
	if (strstr(rx_buf, "on") != NULL) { return ON; }
	else { return OFF; }
}

void USART_send_char(char data)
{
	uint32_t* USART1_DR = (uint32_t*) (USART1_BASE_ADDR + 0x04);
	*USART1_DR = data;	// write data to DR register

	uint32_t* USART1_SR = (uint32_t*) (USART1_BASE_ADDR + 0x00);
	while (((*USART1_SR >> 7) & 1) == 0);	// wait until the data has been transferred
}

void USART_send_string(char* str, ...)
{
	va_list list;
	va_start(list, str);
	char print_buf[128] = {0};
	vsprintf(print_buf, str, list);
	int size = strlen(print_buf);
	for (int i = 0; i < size; i++)
	{
		USART_send_char(print_buf[i]);
	}
	va_end(list);
}

void ctrl_LED(cmd_t color, state_t state)
{
	uint32_t* GPIOD_ODR = (uint32_t*) (GPIOD_BASE_ADDR + 0x14);
	 if (state == ON)
	 {
		 *GPIOD_ODR |= (0b1 << (12 + color));
	 }
	 else
	 {
		 *GPIOD_ODR &= ~(0b1 << (12 + color));
	 }
}

/* ==  DMA2 channel 4  ==
	- Source: UART1 DR
	- Destination: rx_buf
 */
void DMA2_Init()
{
	__HAL_RCC_DMA2_CLK_ENABLE();
	uint32_t* DMA2_S2PAR = (uint32_t*) (DMA2_BASE_ADDR + (0x18 + 0x18 * 2));
	*DMA2_S2PAR = (USART1_BASE_ADDR + 0x04);	// set USART1_DR register as Source

	uint32_t* DMA2_S2M0AR = (uint32_t*) (DMA2_BASE_ADDR + (0x1C + 0x18 * 2));
	*DMA2_S2M0AR = (uint32_t) rx_buf;		// set rx_buffer address as Destination

	uint32_t* DMA_S2NDTR = (uint32_t*) (DMA2_BASE_ADDR + (0x14 + 0x18 * 2));
	*DMA_S2NDTR = sizeof(rx_buf);	// set number of bytes to transfer

	uint32_t* DMA_S2CR = (uint32_t*) (DMA2_BASE_ADDR + (0x10 + 0x18 * 2));
	*DMA_S2CR |= (0b100 << 25);	// select channel 4
	*DMA_S2CR |= (1 << 10);	// enable memory increment mode
	*DMA_S2CR |= (1 << 8);	// enable circular mode
	*DMA_S2CR |= (1 << 4); 	// enable transfer complete interrupt
	*DMA_S2CR |= (1 << 0);	// enable Stream --- ** ENABLE THE PERIPHERAL IS ALWAYS THE LAST STEP **

	uint32_t* NVIC_ISER1 = (uint32_t*) 0xE000E104;
	*NVIC_ISER1 |= (1 << 26);	// accept interrupt signal from DMA2
}

void LEDs_Init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = (uint32_t*) (GPIOD_BASE_ADDR + 0x00);
	*GPIOD_MODER &= ~(0xff << 24);		// clear pin PD12, PD13, PD14, PD15
	*GPIOD_MODER |= (0b01010101 << 24);	// set PD12, PD13, PD14, PD15 as OUTPUT
}

/*
 	 - PB6: UART1 TX
 	 - PB7: UART1 RX
 	 - Baud rate: 9600bps
 	 - Parity: even
 */
void USART1_Init()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	uint32_t* GPIOB_MODER = (uint32_t*) (GPIOB_BASE_ADDR + 0x00);
	uint32_t* GPIOB_AFRL = (uint32_t*) (GPIOB_BASE_ADDR + 0x20);
	*GPIOB_MODER &= ~(0b1111 << 12);// clear bit pin PB6 and PB7
	*GPIOB_MODER |= (0b1010 << 12);	// set PB6 and PB7 in AF mode
	*GPIOB_AFRL &= ~(0xff << 24);	// clear bit pin AFRL6 and AFRL7
	*GPIOB_AFRL |= (0b0111 << 24) | (0b0111 << 28); // select AF7 for AFRL6 and AFRL7

	__HAL_RCC_USART1_CLK_ENABLE();
	uint32_t* USART1_CR1 = (uint32_t*) (USART1_BASE_ADDR + 0x0C);
	uint32_t* USART1_BRR = (uint32_t*) (USART1_BASE_ADDR + 0x08);
	*USART1_CR1 |= (0b1 << 12);	// set word length
	*USART1_CR1 |= (0b1 << 10); // enable parity bit
	*USART1_CR1 &= ~(0b1 << 9); // select Even parity
	*USART1_CR1 |= (0b11 << 2); // enable transmitter & receiver
	*USART1_CR1 |= (0b1 << 13); // enable USART1
	*USART1_BRR = (104 << 4) | (3 << 0); // set baud rate at 9600bps

	uint32_t* USART1_CR3 = (uint32_t*) (USART1_BASE_ADDR + 0x14);
	*USART1_CR3 |= (1 << 6);	// enable DMA receiver

//	*USART1_CR1 |= (0b1 << 5); 	// generate interrupt
//	uint32_t* NVIC_ISER1 = (uint32_t*) 0xE000E104;
//	*NVIC_ISER1 |= (0b1 << 5);	// accept Interrupt signal from UART
}
