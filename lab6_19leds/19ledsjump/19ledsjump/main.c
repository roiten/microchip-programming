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
		// �������� ������ (����� �������)
		for (uint8_t i = 0; i <= LED_COUNT - 3; i++)
		{
			ShiftOut19(1UL << i);                     // 1 ���������
			ShiftOut19((1UL << i) | (1UL << (i+1))); // 2 ����������
			ShiftOut19((1UL << i) | (1UL << (i+1)) | (1UL << (i+2))); // 3 ����������
			ShiftOut19((1UL << i) | (1UL << (i+1))); // 2 ����������
			ShiftOut19(1UL << i);                     // 1 ���������
		}

		// �������� ����� (������ ������)
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
	PORTB &= ~(1 << PB0);  // ���������� ������
}

void ShiftOut19(uint32_t data)
{
	PORTB &= ~(1 << PB1);
	
	// ���������� 19 ���
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