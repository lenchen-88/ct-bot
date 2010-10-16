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

/*!
 * @file 	os_scheduler.h
 * @brief 	Mini-Scheduler fuer BotOS
 * @author 	Timo Sandmann (mail@timosandmann.de)
 * @date 	02.10.2007
 */

#ifndef OS_SCHEDULER_H_
#define OS_SCHEDULER_H_
#include "ct-Bot.h"
#include "bot-logic/available_behaviours.h"

#ifdef MCU
#ifdef OS_AVAILABLE

#define OS_TIME_SLICE	10	/*!< Dauer einer Zeitscheibe in ms */

//#define MEASURE_UTILIZATION	/*!< Aktiviert Statistik ueber das Scheduling */

#if defined BEHAVIOUR_GET_UTILIZATION_AVAILABLE && !defined MEASURE_UTILIZATION
#define MEASURE_UTILIZATION
#endif

extern volatile uint8_t os_scheduling_allowed;	/*!< sperrt den Scheduler, falls != 1. Sollte nur per os_enterCS() / os_exitCS() veraendert werden! */

/*!
 * Aktualisiert den Schedule, prioritaetsbasiert
 * @param tickcount	Wert des Timer-Ticks (32 Bit)
 */
void os_schedule(uint32_t tickcount);

#ifdef MEASURE_UTILIZATION
/*!
 * Aktualisiert die Statistikdaten zur CPU-Auslastung
 */
void os_calc_utilization(void);

/*!
 * Loescht alle Statistikdaten zur CPU-Auslastung
 */

void os_clear_utilization(void);

/*!
 * Gibt die gesammelten Statistikdaten zur CPU-Auslastung per LOG aus
 */
void os_print_utilization(void);
#endif	// MEASURE_UTILIZATION

#ifdef DISPLAY_OS_AVAILABLE
extern uint8_t uart_log;	/*!< Zaehler fuer UART-Auslastung */

/*!
 * Handler fuer OS-Display
 */
void os_display(void);
#endif	// DISPLAY_OS_AVAILABLE

#endif	// OS_AVAILABLE
#endif	// MCU
#endif	// OS_SCHEDULER_H_
