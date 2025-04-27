#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

void InitPorts(void);
void ShiftOut19(uint32_t data);

int main(void)
{
	InitPorts();
	while (1)
	{
		uint32_t frame = (0b0000000000000000000); //создаём кадр для отрисовки 
		// Бег влево
		for (uint8_t i = 0; i < 19; i++)
		{
			frame |= (1UL << i);
			ShiftOut19(frame);
			_delay_ms(100);
		}
		
		// Бег вправо
		for (uint8_t i = 0; i < 19; i++)
		{
			frame &= ~(1UL << (18 - i)); 
			ShiftOut19(frame);
			_delay_ms(100);
		}
	}
}

void InitPorts(void)
{
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);  // PB3 = DS (данные)
	PORTB &= ~(1 << PB0);  // OE = 0 (включить выходы)
}

void ShiftOut19(uint32_t data)
{
	PORTB &= ~(1 << PB1);  // ST_CP = 0
	
	// Отправляем 19 бит
	for (int8_t i = 18; i >= 0; i--)
	{
		PORTB &= ~(1 << PB2);  // SH_CP = 0
		
		if (data & (1UL << i))
		PORTB |= (1 << PB3);   // DS = 1
		else
		PORTB &= ~(1 << PB3);  // DS = 0
		
		PORTB |= (1 << PB2);  // SH_CP = 1
	}
	
	PORTB |= (1 << PB1);  // ST_CP = 1 (защёлка)
}