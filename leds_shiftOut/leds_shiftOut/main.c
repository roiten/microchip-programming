#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

void InitPorts(void);
void InitTimer(void);
void InitSPI(void);
void ShiftOut19(uint32_t data);


#define LED_COUNT 19

volatile uint8_t mode = 0;
volatile uint8_t tick = 0;
volatile uint8_t button_pressed = 0;

uint8_t step = 0;
uint32_t frame = 0;

void InitPorts(void)
{
	DDRB |= (1 << PINB3) | (1 << PINB5) | (1 << PINB2);
	PORTB &= ~(1 << PINB2);

	DDRD &= ~(1 << PIND2); // PD2 - кнопка
	PORTD |= (1 << PIND2); // подтяжка
}

void InitTimer(void)
{
	TCCR1B |= (1 << WGM12); // CTC режим
	TCCR1B |= (1 << CS12) | (1 << CS10); // делитель 1024

	OCR1A = 1696; // 16 МГц / 1024 = 15625 Гц, 15625 * 0.25 = 3906
	TIMSK1 |= (1 << OCIE1A); // Разрешить прерывание по совпадению
}

void InitSPI(void)
{
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); // F_SPI = F_CPU/16 = 1 МГц	
}

void ShiftOut19(uint32_t data)
{
	PORTB &= ~(1 << PINB2);  // ST_CP = 0
	
	//(биты 16-23)
	SPDR = (data >> 16) & 0xFF;
	while(!(SPSR & (1 << SPIF)));
		
	//(биты 8-15)
	SPDR = (data >> 8) & 0xFF;
	while(!(SPSR & (1 << SPIF)));
	
	//(биты 0-7)
	SPDR = data & 0xFF;
	while(!(SPSR & (1 << SPIF)));
		
	// Защелка данных - высокий уровень
	PORTB |= (1 << PINB2);  // ST_CP = 1
}

// Прерывания
ISR(INT0_vect) {
	button_pressed = 1;
}

ISR(TIMER1_COMPA_vect) {
	tick = 1;
}

// ------------------------
// Режимы
// ------------------------

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
	static uint8_t line = 0;

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
		step = 0;
	}
	else if (line == 2)
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
	InitSPI();

	EICRA |= (1 << ISC01); // Прерывание по спаду кнопки
	EIMSK |= (1 << INT0);  // Разрешить прерывание INT0
	sei();                 // Разрешить глобальные прерывания
	
	while (1)
	{
		if (button_pressed)
		{
			mode = (mode + 1) % 3;
			button_pressed = 0;
			step = 0;    // зачистить движение
			frame = 0;   // очистить кадр
		}

		if (tick)
		{
			tick = 0;

			switch (mode)
			{
				case 0: Mode1_collect(); break;
				case 1: Mode2_tripleCollect(); break;
				case 2: Mode3_creative(); break;
			}
		}
	}
}
