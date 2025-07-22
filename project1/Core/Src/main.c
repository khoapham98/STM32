#include "main.h"

#define ADC1_BASE_ADDR 0x40012000
void ADC_Init();
float readVin();
float convert2Temp(float vin);
float temp;
int main()
{
	HAL_Init();
	ADC_Init();
	while (1)
	{
		float tmp = readVin();
		temp = convert2Temp(tmp);
		HAL_Delay(1000);
	}
	return 0;
}

float convert2Temp(float vin)
{
	float temp = ((vin - 0.76) / 2.5e-3) + 25;
	return temp;
}

float readVin()
{
	uint32_t* ADC_SR   = (uint32_t*) (ADC1_BASE_ADDR + 0x00);
	uint32_t* ADC_CR2  = (uint32_t*) (ADC1_BASE_ADDR + 0x08);
	uint32_t* ADC_JDR1 = (uint32_t*) (ADC1_BASE_ADDR + 0x3C);
	/* start conversion of injected channel */
	*ADC_CR2 |= 1 << 22;
	/* wait until conversion complete */
	while (((*ADC_SR >> 2) & 1) == 0);
	/* return conversion value */
	uint16_t val = *ADC_JDR1;
	float vin = (val * 3.0f) / 4095;
	return vin;
}

void ADC_Init()
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	uint32_t* ADC_CR2   = (uint32_t*) (ADC1_BASE_ADDR + 0x08);
	uint32_t* ADC_JSQR  = (uint32_t*) (ADC1_BASE_ADDR + 0x38);
	uint32_t* ADC_CCR   = (uint32_t*) (ADC1_BASE_ADDR + 0x04 + 0x300);
	uint32_t* ADC_SMPR1 = (uint32_t*) (ADC1_BASE_ADDR + 0x0C);
	/* select 1 conversion */
	*ADC_JSQR &= ~(0b11 << 20);
	/* select src ADC_16 */
	*ADC_JSQR |= 16 << 15;
	/* enable temp. sensor */
	*ADC_CCR |= 1 << 23;
	/* set 15 ADC CLK cycles for 12 bit resolution */
	*ADC_SMPR1 |= 0b001 << 18;
	/* enable ADC */
	*ADC_CR2 |= 1 << 0;
}
