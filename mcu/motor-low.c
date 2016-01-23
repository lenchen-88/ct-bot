/*
 * c't-Bot
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

/**
 * \file 	motor-low.c
 * \brief 	Low-Level Routinen fuer die Motor- und Servosteuerung des c't-Bots
 * \author 	Benjamin Benz (bbe@heise.de)
 * \date 	01.12.2005
 * Benutzt Bit 0 und 1 von GPIOR0 fuer die Servos (auf ATmega1284P)
 */

#ifdef MCU
#include "ct-Bot.h"
#include "motor-low.h"
#include "motor.h"
#include "timer.h"
#include "sensor.h"
#include "display.h"

#include <avr/io.h>
#include <stdlib.h>

/* Drehrichtung der Motoren */
#define BOT_DIR_L_PIN  _BV(PC6)
#define BOT_DIR_L_PORT PORTC
#define BOT_DIR_L_DDR  DDRC

#define BOT_DIR_R_PIN  _BV(PC7)
#define BOT_DIR_R_PORT PORTC
#define BOT_DIR_R_DDR  DDRC

/* PWM der Motoren */
#define PWM_L OCR1A
#define PWM_R OCR1B

#ifdef SPEED_CONTROL_AVAILABLE
#define PWM_CLK_1 (_BV(CS10)) /**< Prescaler fuer PWM1 = 1 */
#else
#define PWM_CLK_1 (_BV(CS10) | _BV(CS11)) /**< Prescaler fuer PWM1 = 64 */
#endif

#define PWM_FREQUENCY 16129 /**< PWM Frequenz / Hz, ohne SPEED_CONTROL: / 64 Hz */
#define PWM_TOP (uint16_t)((float) F_CPU / 2.f / PWM_FREQUENCY)


/* PWM fuer Servos (ATmega32) */
#define PWM_CLK_0 (_BV(CS02) | _BV(CS00)) /**< Prescaler fuer PWM0 = 1024 */


int16_t motor_left;  /**< zuletzt gestellter Wert linker Motor */
int16_t motor_right; /**< zuletzt gestellter Wert rechter Motor */

/**
 * Timer 1: Kontrolliert die Motoren per PWM
 * Initialisiert Timer 1 und startet ihn
 */
static void pwm_1_init(void) {
	DDRD |= _BV(PD4) | _BV(PD5); // PWM-Pins output
	TCNT1 = 0; // TIMER1 init

	TCCR1A = _BV(COM1A1) |_BV(COM1A0) |	// Clear on top, set on compare
			 _BV(COM1B1) |_BV(COM1B0);	// Clear on top, set on compare

	TCCR1B = _BV(WGM13) | PWM_CLK_1; // Phase and Frequency Correct PWM, TOP at ICR1

	ICR1 = PWM_TOP;

	OCR1A = PWM_TOP;
	OCR1B = PWM_TOP;
}

/**
 * Stellt einen PWM-Wert fuer einen Motor ein
 * low-level
 * \param dev Motor (0: links; 1: rechts)
 */
void motor_update(uint8_t dev) {
	if (dev == 0) {
		/* linker Motor */
		if (direction.left == DIRECTION_FORWARD) {
			BOT_DIR_L_PORT |= BOT_DIR_L_PIN; // vorwaerts
		} else {
			BOT_DIR_L_PORT = (uint8_t) (BOT_DIR_L_PORT & ~BOT_DIR_L_PIN); // rueckwaerts
		}

		uint8_t sreg = SREG;
		__builtin_avr_cli();
		const int16_t motor_l = motor_left;
		SREG = sreg;
		PWM_L = (uint16_t) (PWM_TOP - ((float) motor_l / 511.f) * PWM_TOP);
	} else {
		/* rechter Motor */
		/* Einer der Motoren ist invertiert, da er ja in die andere Richtung schaut */
		if (direction.right == DIRECTION_BACKWARD) {
			BOT_DIR_R_PORT |= BOT_DIR_R_PIN; // rueckwaerts
		} else {
			BOT_DIR_R_PORT = (uint8_t) (BOT_DIR_R_PORT & ~BOT_DIR_R_PIN); // vorwaerts
		}

		uint8_t sreg = SREG;
		__builtin_avr_cli();
		const int16_t motor_r = motor_right;
		SREG = sreg;
		PWM_R = (uint16_t) (PWM_TOP - ((float) motor_r / 511.f) * PWM_TOP);
	}
}

/**
 * Initialisiert Timer3 fuer Servoansteuerung (nur ATmega1284P)
 */
static void timer_3_init(void) {
#ifdef __AVR_ATmega1284P__
	TCNT3 = 0; // TIMER3 init
	TIMSK3 = 0; // Timer Interrupts Disable
	TIFR1 = _BV(TOV3) | _BV(OCF3A) | _BV(OCF3B); // Clear Timer Flags
	TCCR3A = 0;
	TCCR3B = _BV(CS31); // normal timer mode, prescaler = 8 -> 20 MHz: 26,2144 ms period, 16 MHz: 32.768 ms period
	OCR3A = 0;
	OCR3B = 0;
#endif // __AVR_ATmega1284P__
}

/**
 * Initialisiert Ansteuerung fuer Servo 1
 * Auf ATmega1284P mit Timer3, ansonsten mit Timer0
 */
static void servo_1_init(void) {
	DDRB |= _BV(DDB3); // PWM-Pin Output
#ifndef __AVR_ATmega1284P__
	TCNT0 = 0; // TIMER0 vorladen

#if defined MCU_ATMEGA644X
	TCCR0A = _BV(WGM00) | // Phase Correct PWM Mode
			 _BV(COM0A1); // Clear on Compare Match when up-counting. Set on Compare Match when down-counting

#else
	TCCR0 = _BV(WGM00) | // Phase Correct PWM Mode
			_BV(COM01);  // Clear on Compare Match when up-counting. Set on Compare Match when down-counting

#endif // MCU_ATMEGA644X
#endif // ! __AVR_ATmega1284P__
}

/**
 * Initialisiert Ansteuerung fuer Servo 2 (nur ATmega1284P)
 */
static void servo_2_init(void) {
#ifdef __AVR_ATmega1284P__
	GPIOR0 = 0; // beide Servos aus
	DDRD |= _BV(DDD7); // PWM-Pin Output
#endif // __AVR_ATmega1284P__
}

#ifdef __AVR_ATmega1284P__
/**
 * Timer3 Overflow Interrupt
 */
ISR(TIMER3_OVF_vect, ISR_NAKED) {
	if (GPIOR0 & 1) {
		PORTB |= _BV(PB3); // PWM0 high
	}
	if (GPIOR0 & 2) {
		PORTD |= _BV(PD7); // PWM2 high
	}
	reti();
}

/**
 * Timer3 Output Compare A Match Interrupt
 */
ISR(TIMER3_COMPA_vect, ISR_NAKED) {
	PORTB = (uint8_t) (PORTB & ~_BV(PB3)); // PWM0 low
	reti();
}

/**
 * Timer3 Output Compare B Match Interrupt
 */
ISR(TIMER3_COMPB_vect, ISR_NAKED) {
	PORTD = (uint8_t) (PORTD & ~_BV(PD7)); // PWM2 low
	reti();
}
#endif // __AVR_ATmega1284P__

/**
 * Stellt die Servos
 * \param servo Nummer des Servos (1 oder 2)
 * \param pos Zielwert oder 0 fuer Servo aus
 */
void servo_low(uint8_t servo, uint8_t pos) {
	if (servo == SERVO1) {
#ifdef __AVR_ATmega1284P__
		if (pos == SERVO_OFF) {
			GPIOR0 = (uint8_t) (GPIOR0 & ~_BV(0));
			TIMSK3 = (uint8_t) (TIMSK3 & ~_BV(OCIE3A)); // Timer Interrupt Disable
			TIFR1 |= _BV(OCF3A); // Clear Timer Flags
			PORTB = (uint8_t) (PORTB & ~_BV(PB3)); // PWM0 low
		} else {
#if F_CPU == 20000000
			OCR3A = (uint16_t) (pos * 18 + 1400); // PWM value [1418; 5990] -> [0.6 ms; 2.4 ms] pulse
#elif F_CPU == 16000000
			OCR3A = (uint16_t) (pos * 13 + 1400); // PWM value [1413; 4715] -> [0.7 ms; 2.4 ms] pulse
#else
#warning "current F_CPU not supported for servo_low()"
#endif // F_CPU
			TIMSK3 |= (uint8_t) (_BV(TOIE3) | _BV(OCIE3A)); // Overflow Interrupt Enable, Output Compare A Match Interrupt Enable
			GPIOR0 |= _BV(0);
		}
#else // ! __AVR_ATmega1284P__
		if (pos == SERVO_OFF) {
#ifdef MCU_ATMEGA644X
			TCCR0B = (uint8_t) (TCCR0B & ~PWM_CLK_0); // PWM0 aus
#else
			TCCR0 = (uint8_t) (TCCR0 & ~PWM_CLK_0); // PWM0 aus
#endif // MCU_ATMEGA644X
		} else {
#ifdef MCU_ATMEGA644X
			TCCR0B |= PWM_CLK_0; // PWM0 an
			OCR0A = pos;
#else
			TCCR0 |= PWM_CLK_0; // PWM0 an
			OCR0 = pos;
#endif // MCU_ATMEGA644X
		}
#endif // __AVR_ATmega1284P__
	} else if (servo == SERVO2) {
#ifdef __AVR_ATmega1284P__
		if (pos == SERVO_OFF) {
			GPIOR0 = (uint8_t) (GPIOR0 & ~_BV(1));
			TIMSK3 = (uint8_t) (TIMSK3 & ~_BV(OCIE3B)); // Timer Interrupt Disable
			TIFR1 |= _BV(OCF3B); // Clear Timer Flags
			PORTD = (uint8_t) (PORTD & ~_BV(PD7)); // PWM2 low
		} else {
#if F_CPU == 20000000
			OCR3B = (uint16_t) (pos * 18 + 1400); // PWM value [1418; 5990] -> [0.6 ms; 2.4 ms] pulse
#elif F_CPU == 16000000
			OCR3B = (uint16_t) (pos * 13 + 1400); // PWM value [1413; 4715] -> [0.7 ms; 2.4 ms] pulse
#else
#warning "current F_CPU not supported for servo_low()"
#endif // F_CPU
			TIMSK3 |= (uint8_t) (_BV(TOIE3) | _BV(OCIE3B)); // Overflow Interrupt Enable, Output Compare B Match Interrupt Enable
			GPIOR0 |= _BV(1);
		}
#endif //__AVR_ATmega1284P__
	}
}

/**
 * Initialisiert alles fuer die Motor- und Servorsteuerung
 */
void motor_low_init() {
	BOT_DIR_L_DDR |= BOT_DIR_L_PIN;
	BOT_DIR_R_DDR |= BOT_DIR_R_PIN;

	pwm_1_init();
	timer_3_init();
	servo_1_init();
	servo_2_init();
	motor_left  = 0;
	motor_right = 0;
	PWM_L = PWM_TOP;
	PWM_R = PWM_TOP;
	direction.left  = DIRECTION_FORWARD;
	direction.right = DIRECTION_FORWARD;
}

#endif // MCU
