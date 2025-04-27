#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED_COUNT 19

volatile uint8_t mode = 0;
volatile uint8_t tick = 0;
volatile uint8_t button_pressed = 0;

uint8_t step = 0;
uint32_t frame = 0;
uint8_t line = 0;


void InitPorts(void)
{
	DDRB |= 0xFF;
	DDRC |= 0x3F;
	DDRD |= 0x73;
	
	DDRD &= ~(1 << PIND2); // PIND2 - кнопка
	PORTD |= (1 << PIND2); // подтяжка к питанию
}

void InitTimer(void)
{
	TCCR1A = 0;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 400;
}

void ShiftOut19(uint32_t data)
{
	PORTB = data & 0xFF;
	PORTC = (data >> 8) & 0x3F;
	
	if (data & (1UL << 14))
	{
		PORTD |= (1 << PIND0);
	}
	else
	{
		PORTD &= ~(1 << PIND0);
	}
	
	if (data & (1UL << 15))
	{
		PORTD |= (1 << PIND1);
	}
	else
	{
		PORTD &= ~(1 << PIND1);
	}
	
	if (data & (1UL << 16))
	{
		PORTD |= (1 << PIND4);
	}
	else
	{
		PORTD &= ~(1 << PIND4);
	}
	
	if (data & (1UL << 17))
	{
		PORTD |= (1 << PIND5);
	}
	else
	{
		PORTD &= ~(1 << PIND5);
	}
	
	if (data & (1UL << 18))
	{
		PORTD |= (1 << PIND6);
	}
	else
	{
		PORTD &= ~(1 << PIND6);
	}
}

ISR(INT0_vect)
{
	_delay_ms(30);
	if (!(PIND & (1 << PIND2)))
	{
		button_pressed = 1;
	}
}

ISR(TIMER1_COMPA_vect)
{
	tick = 1;
}

void Mode1_collect(void)
{
	if (step < LED_COUNT)
	{
		frame = (1UL << step);
	}
	else if (step < 2 * LED_COUNT)
	{
		frame = (1UL << (2 * LED_COUNT - step - 1));
	}
	else
	{
		step = 0;
		frame = (1UL << step);
	}
	
	ShiftOut19(frame);
	step++;
}

void Mode2_tripleCollect(void)
{
	static uint8_t pos_led = 0; // текущая позиция
	static uint8_t count = 1;
	static uint8_t line = 0; 

	frame = 0;
	for(uint8_t i = 0; i < count; i++)
	{
		frame |= (1UL << ((pos_led + i) % LED_COUNT));
	}

	pos_led = (pos_led + 1) % LED_COUNT;

	if(pos_led == 0)
	{
		if(line == 0)
		{
			if(count < 3)
			{
				count++;
			}
			else
			{
				line = 1;
			}
		}
		else
		{
			if(count > 1)
			{
				count--;
			}
			else
			{
				line = 0;
			}
		}
	}

	ShiftOut19(frame);
}

void Mode3_creative(void)
{
	if (line == 0)
	{
		if (step < 10)
		{
			frame |= (1UL << step);
			frame |= (1UL << (18 - step));
			step++;
		}
		else
		{
			line = 1;
			step = 0;
		}
	}
	else if (line == 1)
	{
		frame = 0x7FFFF;
		line = 2;
	}
	else
	{
		if (step < 10)
		{
			frame &= ~(1UL << step);
			frame &= ~(1UL << (18 - step));
			step++;
		}
		else
		{
			line = 0;
			step = 0;
			frame = 0;
		}
	}
	
	ShiftOut19(frame);
}

int main(void)
{
	InitPorts();
	InitTimer();
	
	EICRA |= (1 << ISC01);
	EIMSK |= (1 << INT0);
	sei();

	while (1)
	{
		if (button_pressed)
		{
			mode = (mode + 1) % 3;
			button_pressed = 0;
			step = 0;
			frame = 0;
			line = 0;
		}

		if (tick)
		{
			tick = 0;
			
			switch (mode)
			{
				case 0:
					Mode1_collect();
					break;
				case 1:
					Mode2_tripleCollect();
					break;
				case 2:
					Mode3_creative();
					break;
			}
		}
	}
}