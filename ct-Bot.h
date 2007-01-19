/*
 * c't-Sim - Robotersimulator fuer den c't-Bot
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

/*! @file 	ct-Bot.h
 * @brief 	Demo-Hauptprogramm
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
*/
#ifndef CT_BOT_H_DEF
#define CT_BOT_H_DEF

#ifndef MMC_LOW_H_
	#include "global.h"	// denn in mmc-low.S sind typedefs unerwuenscht
#endif

/************************************************************
* Module switches, to make code smaller if features are not needed
************************************************************/
//#define LOG_CTSIM_AVAILABLE		/*!< Logging ueber das ct-Sim (PC und MCU) */
//#define LOG_DISPLAY_AVAILABLE		/*!< Logging ueber das LCD-Display (PC und MCU) */
//#define LOG_UART_AVAILABLE		/*!< Logging ueber UART (NUR für MCU) */
#define LOG_STDOUT_AVAILABLE 		/*!< Logging auf die Konsole (NUR für PC) */


#define LED_AVAILABLE		/*!< LEDs for local control */

#define IR_AVAILABLE		/*!< Infrared Remote Control */
#define RC5_AVAILABLE		/*!< Key-Mapping for IR-RC	 */

#define BOT_2_PC_AVAILABLE	/*!< Soll der Bot mit dem PC kommunmizieren? */

//#define TIME_AVAILABLE		/*!< Gibt es eine Systemzeit im s und ms? */

#define DISPLAY_AVAILABLE	/*!< Display for local control */
#define DISPLAY_REMOTE_AVAILABLE /*!< Sende LCD Anzeigedaten an den Simulator */
#define DISPLAY_SCREENS_AVAILABLE	/*!< Ermoeglicht vier verschiedene Screen */
//#define DISPLAY_SCREEN_RESETINFO	/*!< Zeigt auf Screen 4 Informationen ueber Resets an */
//#define DISPLAY_ODOMETRIC_INFO 	/*!< Zeigt auf Screen 4 Positions- und Geschwindigkeitsdaten */
//#define DISPLAY_REGELUNG_AVAILABLE 3 /*!< Zeigt Reglerdaten auf Screen 4, wenn SPEED_CONTROL_AVAILABLE gesetzt ist*/
#define DISPLAY_MMC_INFO	/*!< Zeigt auf Screen 4 die Daten der MMC-Karte an */
//#define DISPLAY_BEHAVIOUR_AVAILABLE  /*!< Anzeige der Verhalten im Display Screen 3, ersetzt Counteranzeige */
#define DISPLAY_DYNAMIC_BEHAVIOUR_AVAILABLE /*!< Zustandsaenderungen der Verhalten sind direkt unter Umgehung der Puffervar sichtbar */
#define MEASURE_MOUSE_AVAILABLE			/*!< Geschwindigkeiten werden aus den Maussensordaten berechnet */
//#define MEASURE_COUPLED_AVAILABLE		/*!< Geschwindigkeiten werden aus Maus- und Encoderwerten ermittelt und gekoppelt */


//#define WELCOME_AVAILABLE	/*!< kleiner Willkommensgruss */

#define ADC_AVAILABLE		/*!< A/D-Converter */

#define MAUS_AVAILABLE		/*!< Maus Sensor */

#define ENA_AVAILABLE		/*!< Enable-Leitungen */
#define SHIFT_AVAILABLE		/*!< Shift Register */

//#define TEST_AVAILABLE_ANALOG	/*!< Sollen die LEDs die analoge Sensorwerte anzeigen */
#define TEST_AVAILABLE_DIGITAL	/*!< Sollen die LEDs die digitale Sensorwerte anzeigen */
//#define TEST_AVAILABLE_MOTOR	/*!< Sollen die Motoren ein wenig drehen */
//#define TEST_AVAILABLE_COUNTER /*!< Gibt einen Endlos-Counter auf Screen 3 aus und aktiviert Screen 3 */
//#define DOXYGEN		/*!< Nur zum Erzeugen der Doku, wenn dieser schalter an ist, jammert der gcc!!! */

#define BEHAVIOUR_AVAILABLE /*!< Nur wenn dieser Parameter gesetzt ist, exisitiert das Verhaltenssystem */

#define MAP_AVAILABLE /*!< Aktiviere die Kartographie */

//#define SPEED_CONTROL_AVAILABLE /*!< Aktiviert die Motorregelung */

//#define SRF10_AVAILABLE		/*!< Ultraschallsensor SRF10 vorhanden */

#define MMC_AVAILABLE			/*!< haben wir eine MMC/SD-Karte zur Verfuegung */
#define MINI_FAT_AVAILABLE		/*!< koennen wir sektoren in FAT-systemen finden */
//#define MMC_VM_AVAILABLE		/*!< Virtual Memory Management mit MMC / SD-Card oder PC-Emulation */

#define BOOTLOADER_AVAILABLE	/*!< Aktiviert den Bootloadercode - das ist nur noetig fuer die einmalige "Installation" des Bootloaders. Achtung, Linkereinstellungen anpassen (siehe mcu/bootloader.c)! */
/************************************************************
* Some Dependencies!!!
************************************************************/

#ifdef DOXYGEN
	#define PC			/*!< Beim generieren der Doku alles anschalten */
	#define MCU		/*!< Beim generieren der Doku alles anschalten */
	#define TEST_AVAILABLE_MOTOR	/*!< Beim generieren der Doku alles anschalten */
#endif


#ifndef DISPLAY_AVAILABLE
	#undef WELCOME_AVAILABLE
   #undef DISPLAY_REMOTE_AVAILABLE
   #undef DISPLAY_SCREENS_AVAILABLE
   #undef DISPLAY_SCREEN_RESETINFO
#endif

#ifndef IR_AVAILABLE
	#undef RC5_AVAILABLE
#endif

#ifndef MAUS_AVAILABLE
	#undef MEASURE_MOUSE_AVAILABLE
	#undef MEASURE_COUPLED_AVAILABLE
#endif

#ifdef PC
	#ifndef DOXYGEN
		#undef UART_AVAILABLE
		#undef BOT_2_PC_AVAILABLE
		#undef SRF10_AVAILABLE
		#undef TWI_AVAILABLE
		#undef SPEED_CONTROL_AVAILABLE // Deaktiviere die Motorregelung 
		#undef MMC_AVAILABLE
	#endif

	#define COMMAND_AVAILABLE		/*!< High-Level Communication */
   #undef DISPLAY_SCREEN_RESETINFO
   #undef TEST_AVAILABLE_COUNTER
#endif

#ifdef MCU
	#ifdef LOG_CTSIM_AVAILABLE
		#define BOT_2_PC_AVAILABLE
	#endif
	#ifdef BOT_2_PC_AVAILABLE
		#define UART_AVAILABLE	/*!< Serial Communication */
		#define COMMAND_AVAILABLE	/*!< High-Level Communication */
	#endif
//	#undef MAP_AVAILABLE
#endif


#ifdef TEST_AVAILABLE_MOTOR
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams */
	#define TEST_AVAILABLE_DIGITAL /*!< Sollen die LEDs die digitale Sensorwerte anzeigen */
#endif

#ifdef TEST_AVAILABLE_DIGITAL
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams */
	#undef TEST_AVAILABLE_ANALOG
#endif

#ifdef TEST_AVAILABLE_ANALOG
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams */
#endif

#ifdef TEST_AVAILABLE_COUNTER
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams */
	#define DISPLAY_SCREENS_AVAILABLE
	#define DISPLAY_SCREEN_RESETINFO
#endif

#ifdef DISPLAY_ODOMETRIC_INFO
	#undef DISPLAY_SCREEN_RESETINFO		/*!< Wenn Odometrieinfos, dann keine Resetinfos */
#endif

#ifndef SPEED_CONTROL_AVAILABLE
	#undef DISPLAY_REGELUNG_AVAILABLE
#endif

#ifdef LOG_UART_AVAILABLE
	#define LOG_AVAILABLE
#endif 
#ifdef LOG_CTSIM_AVAILABLE
	#define LOG_AVAILABLE
#endif 
#ifdef LOG_DISPLAY_AVAILABLE
	#define LOG_AVAILABLE
#endif 
#ifdef LOG_STDOUT_AVAILABLE
	#define LOG_AVAILABLE
#endif 

#ifndef MMC_AVAILABLE
	#undef MINI_FAT_AVAILABLE
	#ifdef MCU
		#undef MMC_VM_AVAILABLE
	#endif
#endif

#ifdef LOG_AVAILABLE
	#ifdef PC
		/* Auf dem PC gibts kein Logging ueber UART. */
		#undef LOG_UART_AVAILABLE
	#endif
	
	#ifdef MCU
		/* Mit Bot zu PC Kommunikation auf dem MCU gibts kein Logging ueber UART.
		 * Ohne gibts keine Kommunikation ueber ct-Sim.
		 */
		#undef LOG_STDOUT_AVAILABLE		/*!< MCU hat kein STDOUT */
		#ifdef BOT_2_PC_AVAILABLE
			#undef LOG_UART_AVAILABLE
		#else
			#undef LOG_CTSIM_AVAILABLE
		#endif
	#endif
	
	/* Ohne Display gibts auch keine Ausgaben auf diesem. */
	#ifndef DISPLAY_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
	#endif
	
	/* Logging aufs Display ist nur moeglich, wenn mehrere Screens
	 * unterstuetzt werden.
	 */
	#ifndef DISPLAY_SCREENS_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
	#endif
	
	/* Es kann immer nur ueber eine Schnittstelle geloggt werden. */
	
	#ifdef LOG_UART_AVAILABLE
		#define UART_AVAILABLE
		#undef LOG_CTSIM_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
		#undef LOG_STDOUT_AVAILABLE
	#endif
	
	#ifdef LOG_CTSIM_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
		#undef LOG_STDOUT_AVAILABLE
	#endif
	
	#ifdef LOG_DISPLAY_AVAILABLE
		#undef LOG_STDOUT_AVAILABLE
	#endif

	// Wenn keine sinnvolle Log-Option mehr uebrig, loggen wir auch nicht
	#ifndef LOG_CTSIM_AVAILABLE
		#ifndef LOG_DISPLAY_AVAILABLE
			#ifndef LOG_UART_AVAILABLE
				#ifndef LOG_STDOUT_AVAILABLE
					#undef LOG_AVAILABLE
				#endif
			#endif
		#endif
	#endif

#endif


#ifdef SRF10_AVAILABLE
	#define TWI_AVAILABLE				/*!< TWI-Schnittstelle (I2C) nutzen */
#endif


#define F_CPU	16000000L    /*!< Crystal frequency in Hz */
#define XTAL F_CPU			 /*!< Crystal frequency in Hz */

#define LINE_FEED "\n\r"	/*!< Windows und Linux unterscheiden beim Linefeed. Windows erwarten \n\r, Linux nur \n */

#ifdef MCU
	#ifndef MMC_LOW_H_
		#include <avr/interrupt.h>
	#endif
	#ifdef SIGNAL
		#define NEW_AVR_LIB
	#endif
#endif

#endif
