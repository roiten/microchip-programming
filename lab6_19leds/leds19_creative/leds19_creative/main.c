#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void InitPorts(void);
void InitTimer(void);
void WaitTick(void);
void ShiftOut19(uint32_t data);

#define LED_COUNT 19

volatile uint8_t mode = 0;
volatile uint8_t tick = 0;  // ����, ������������� ��� � 250 ��
volatile uint8_t button_pressed = 0;

void InitPorts(void) {
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
	PORTB &= ~(1 << PB0);  // ���������� ������ (OE = 0)
	
	DDRD &= ~(1 << PIND2); // PIND2 - ������
	PORTD |= (1 << PIND2); // �������� � �������
}

void InitTimer(void) {
	TCCR1B |= (1 << WGM12);               // CTC �����
	TCCR1B |= (1 << CS12) | (1 << CS10);  // �������� 1024
	OCR1A = 244;                          // ��������� ������ 250 �� (��� 1 ��� � 1024 ��������)
	TIMSK1 |= (1 << OCIE1A);              // ��������� ����������
}

// ����� 19-������� ����� �� ����������
void ShiftOut19(uint32_t data) {
	PORTB &= ~(1 << PB1);  // ST_CP = 0
	
	// ���������� 19 ���
	for (int8_t i = 18; i >= 0; i--) {
		PORTB &= ~(1 << PB2);  // SH_CP = 0
		
		if (data & (1UL << i))
		PORTB |= (1 << PB3);   // DS = 1
		else
		PORTB &= ~(1 << PB3);  // DS = 0
		
		PORTB |= (1 << PB2);  // SH_CP = 1
	}
	WaitTick();
	PORTB |= (1 << PB1);  // ST_CP = 1 (����������� ������)
}

void WaitTick(void) {
	while (!tick);
	tick = 0;
}

// ���������� ���������� INT0 (������ �� PIND2)
ISR(INT0_vect) {
	button_pressed = 1;
}

ISR(TIMER1_COMPA_vect) {
	tick = 1;
}

void Mode1_collect(void) {
	uint32_t frame;
	
	for (uint8_t i = 0; i < LED_COUNT; i++) {
		if (button_pressed) return;
		frame = (1UL << i);
		ShiftOut19(frame);
	}
	for (uint8_t i = 0; i < LED_COUNT; i++) {
		if (button_pressed) return;
		frame = (1UL << (18 - i));
		ShiftOut19(frame);
	}
}

void Mode2_tripleCollect(void) {
	uint32_t frame;

	for (uint8_t i = 0; i <= LED_COUNT - 3; i++) {
		if (button_pressed) return;
		frame = (1UL << i);
		ShiftOut19(frame);

		if (button_pressed) return;
		frame |= (1UL << (i + 1));
		ShiftOut19(frame);

		if (button_pressed) return;
		frame |= (1UL << (i + 2));
		ShiftOut19(frame);

		if (button_pressed) return;
		frame &= ~(1UL << (i + 2));
		ShiftOut19(frame);

		if (button_pressed) return;
		frame &= ~(1UL << (i + 1));
		ShiftOut19(frame);
	}

	for (int8_t i = LED_COUNT - 3; i > 0; i--) {
		if (button_pressed) return;
		frame = (1UL << i);
		ShiftOut19(frame);

		if (button_pressed) return;
		frame |= (1UL << (i + 1));
		ShiftOut19(frame);

		if (button_pressed) return;
		frame |= (1UL << (i + 2));
		ShiftOut19(frame);

		if (button_pressed) return;
		frame &= ~(1UL << (i + 2));
		ShiftOut19(frame);

		if (button_pressed) return;
		frame &= ~(1UL << (i + 1));
		ShiftOut19(frame);
	}
}

void Mode3_creative(void) {
	uint32_t frame = 0;

	for (uint8_t i = 0; i < 10; i++) {
		if (button_pressed) return;
		frame |= (1UL << i);
		ShiftOut19(frame);

		if (button_pressed) return;
		frame |= (1UL << (18 - i));
		ShiftOut19(frame);
	}

	if (button_pressed) return;
	frame = 0x7FFFF;
	ShiftOut19(frame);
	_delay_ms(200); // ����� ���� �������� �� ���� � ��������� ���������� �����

	for (uint8_t i = 0; i < 10; i++) {
		if (button_pressed) return;
		frame &= ~(1UL << i);
		ShiftOut19(frame);

		if (button_pressed) return;
		frame &= ~(1UL << (18 - i));
		ShiftOut19(frame);
	}

	if (button_pressed) return;
	frame = 0;
	ShiftOut19(frame);
}

int main(void) {
	InitPorts();
	InitTimer();

	EICRA |= (1 << ISC01);  // ���������� �� ����� (������� ������)
	EIMSK |= (1 << INT0);   // ��������� ���������� INT0
	sei();                  // ��������� ��������� ����������

	while (1) {
		if (button_pressed) {
			mode = (mode + 1) % 3;
			button_pressed = 0;
		}

		switch (mode) {
			case 0: Mode1_collect(); break;
			case 1: Mode2_tripleCollect(); break;
			case 2: Mode3_creative(); break;
		}
	}
}
