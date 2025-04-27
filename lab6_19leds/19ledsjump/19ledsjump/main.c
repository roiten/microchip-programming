#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

#define LED_COUNT 19

void InitPorts(void);
void ShiftOut19(uint32_t data);

int main(void)
{
	InitPorts();
	
	while (1)
	{
		// Движение вперед (слева направо)
		for (uint8_t i = 0; i <= LED_COUNT - 3; i++)
		{
			ShiftOut19(1UL << i);                     // 1 светодиод
			ShiftOut19((1UL << i) | (1UL << (i+1))); // 2 светодиода
			ShiftOut19((1UL << i) | (1UL << (i+1)) | (1UL << (i+2))); // 3 светодиода
			ShiftOut19((1UL << i) | (1UL << (i+1))); // 2 светодиода
			ShiftOut19(1UL << i);                     // 1 светодиод
		}

		// Движение назад (справа налево)
		for (uint8_t i = LED_COUNT - 3; i > 0; i--)
		{
			ShiftOut19(1UL << i);
			ShiftOut19((1UL << i) | (1UL << (i+1)));
			ShiftOut19((1UL << i) | (1UL << (i+1)) | (1UL << (i+2)));
			ShiftOut19((1UL << i) | (1UL << (i+1)));
			ShiftOut19(1UL << i);
		}
	}
}

void InitPorts(void)
{
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
	PORTB &= ~(1 << PB0);  // Активируем выходы
}

void ShiftOut19(uint32_t data)
{
	PORTB &= ~(1 << PB1);
	
	// Отправляем 19 бит
	for (int8_t i = 18; i >= 0; i--)
	{
		PORTB &= ~(1 << PB2);
		if (data & (1UL << i))
		PORTB |= (1 << PB3);
		else
		PORTB &= ~(1 << PB3);
		PORTB |= (1 << PB2);
	}
	_delay_ms(120);
	PORTB |= (1 << PB1);
}