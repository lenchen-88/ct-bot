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
 * @file 	ct-Bot.h
 * @brief 	globale Schalter fuer die einzelnen Bot-Funktionalitaeten
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.2005
 */
#ifndef CT_BOT_H_DEF
#define CT_BOT_H_DEF

/************************************************************
* Module switches, to make code smaller if features are not needed
************************************************************/
//#define LOG_CTSIM_AVAILABLE		/*!< Logging zum ct-Sim (PC und MCU) */
//#define LOG_DISPLAY_AVAILABLE		/*!< Logging ueber das LCD-Display (PC und MCU) */
//#define LOG_UART_AVAILABLE		/*!< Logging ueber UART (nur fuer MCU) */
#define LOG_STDOUT_AVAILABLE 		/*!< Logging auf die Konsole (nur fuer PC) */
//#define LOG_MMC_AVAILABLE			/*!< Logging in eine txt-Datei auf MMC */
#define USE_MINILOG					/*!< schaltet fuer MCU auf schlankes Logging um (nur in Verbindung mit Log2Sim) */

//#define CREATE_TRACEFILE_AVAILABLE	/*!< Aktiviert das Schreiben einer Trace-Datei (nur PC) */


#define LED_AVAILABLE		/*!< LEDs aktiv */

#define IR_AVAILABLE		/*!< Infrarot Fernbedienung aktiv */
#define RC5_AVAILABLE		/*!< Key-Mapping fuer IR-RC aktiv */
//#define KEYPAD_AVAILABLE	/*!< Keypad-Eingabe vorhanden? */

#define BOT_2_SIM_AVAILABLE	/*!< Soll der Bot mit dem Sim kommunizieren? */
//#define BOT_2_BOT_AVAILABLE	/*!< Sollen Bots untereinander kommunizieren? */
//#define BOT_2_BOT_PAYLOAD_AVAILABLE		/*!< Aktiviert Payload-Versand per Bot-2-Bot Kommunikation */

//#define TIME_AVAILABLE		/*!< Gibt es eine Systemzeit in s und ms? */

#define DISPLAY_AVAILABLE	/*!< Display aktiv */
#define DISPLAY_REMOTE_AVAILABLE /*!< Sende LCD Anzeigedaten an den Simulator */

#define MEASURE_MOUSE_AVAILABLE			/*!< Geschwindigkeiten werden aus den Maussensordaten berechnet */
//#define MEASURE_COUPLED_AVAILABLE		/*!< Geschwindigkeiten werden aus Maus- und Encoderwerten ermittelt und gekoppelt */
//#define MEASURE_POSITION_ERRORS_AVAILABLE	/*!< Fehlerberechnungen bei der Positionsbestimmung */

//#define WELCOME_AVAILABLE	/*!< kleiner Willkommensgruss */

#define ADC_AVAILABLE		/*!< A/D-Konverter */

//#define MOUSE_AVAILABLE	/*!< Maus Sensor */

#define ENA_AVAILABLE		/*!< Enable-Leitungen */
#define SHIFT_AVAILABLE		/*!< Shift Register */

//#define BPS_AVAILABLE		/*!< Bot Positioning System */

//#define TEST_AVAILABLE_ANALOG		/*!< Sollen die LEDs die analoge Sensorwerte anzeigen */
//#define TEST_AVAILABLE_DIGITAL	/*!< Sollen die LEDs die digitale Sensorwerte anzeigen */
//#define TEST_AVAILABLE_MOTOR		/*!< Sollen die Motoren ein wenig drehen */

#define BEHAVIOUR_AVAILABLE		/*!< Nur wenn dieser Parameter gesetzt ist, exisitiert das Verhaltenssystem */

#define POS_STORE_AVAILABLE		/*!< Positionsspeicher vorhanden */

//#define MAP_AVAILABLE			/*!< Aktiviert die Kartographie */
//#define MAP_2_SIM_AVAILABLE	/*!< Sendet die Map zur Anzeige an den Sim */

//#define SPEED_CONTROL_AVAILABLE /*!< Aktiviert die Motorregelung */
//#define ADJUST_PID_PARAMS		/*!< macht PID-Paramter zur Laufzeit per FB einstellbar */
//#define SPEED_LOG_AVAILABLE 	/*!< Zeichnet Debug-Infos der Motorregelung auf MMC auf */

//#define SRF10_AVAILABLE		/*!< Ultraschallsensor SRF10 vorhanden */
//#define CMPS03_AVAILABLE		/*!< Kompass CMPS03 vorhanden */
//#define SP03_AVAILABLE		/*!< Sprachmodul SP03 vorhanden */

//#define MMC_AVAILABLE			/*!< haben wir eine MMC/SD-Karte zur Verfuegung? Nach jeder Aenderung dieses Schalters bitte Project -> Clean ausfuehren! */
//#define SPI_AVAILABLE			/*!< verwendet den Hardware-SPI-Modus des Controllers, um mit der MMC zu kommunizieren. Nach jeder Aenderung dieses Schalters bitte Project -> Clean ausfuehren! Muss ausserdem _immer_ an sein, wenn der Hardware-SPI-Umbau durchgefuehrt wurde! Hinweise in mcu/mmc.c beachten! */
//#define MMC_VM_AVAILABLE		/*!< Virtual Memory Management mit MMC / SD-Card oder PC-Emulation */
#define OS_AVAILABLE			/*!< Aktiviert BotOS fuer Threads und Scheduling */

//#define EEPROM_EMU_AVAILABLE	/*!< Aktiviert die EEPROM-Emulation fuer PC */

//#define BOOTLOADER_AVAILABLE	/*!< Aktiviert den Bootloadercode - das ist nur noetig fuer die einmalige "Installation" des Bootloaders. Siehe auch Documentation/Bootloader.html */

/************************************************************
* Some Dependencies!!!
************************************************************/

#ifdef DOXYGEN
	/* Beim Generieren der Doku alles anschalten */
	#define PC
	#define MCU
	#define TEST_AVAILABLE_MOTOR
#endif

#ifndef DISPLAY_AVAILABLE
	#undef WELCOME_AVAILABLE
	#undef DISPLAY_REMOTE_AVAILABLE
#endif

#ifndef IR_AVAILABLE
	#undef RC5_AVAILABLE
#endif

#ifndef MOUSE_AVAILABLE
	#undef MEASURE_MOUSE_AVAILABLE
	#undef MEASURE_COUPLED_AVAILABLE
#endif

#ifdef BOT_2_BOT_AVAILABLE
	#define BOT_2_SIM_AVAILABLE
#endif

#ifdef PC
	#ifndef DOXYGEN
		/* Folgende Optionen deaktivieren, es gibt sie nicht fuer PC */
		#undef UART_AVAILABLE
		#undef SRF10_AVAILABLE
		#undef TWI_AVAILABLE
		#undef SPEED_CONTROL_AVAILABLE
		#undef SPEED_LOG_AVAILABLE
		#undef MMC_AVAILABLE
		#undef I2C_AVAILABLE
		#undef CMPS03_AVAILABLE
		#undef SP03_AVAILABLE
	#endif

	#ifndef BOT_2_SIM_AVAILABLE
		#define BOT_2_SIM_AVAILABLE // simulierte Bots brauchen immer Kommunikation zum Sim
	#endif

	#define COMMAND_AVAILABLE		/*!< High-Level Kommunikation */
	#undef USE_MINILOG
#endif	// PC

#ifdef MCU
	#ifdef LOG_CTSIM_AVAILABLE
		#define BOT_2_SIM_AVAILABLE
	#endif
	#ifdef BOT_2_SIM_AVAILABLE
		#define UART_AVAILABLE		/*!< Serielle Kommunikation */
		#define COMMAND_AVAILABLE	/*!< High-Level Communication */
	#else
		#undef DISPLAY_REMOTE_AVAILABLE
		#undef MAP_2_SIM_AVAILABLE
	#endif
	#undef EEPROM_EMU_AVAILABLE
	#undef CREATE_TRACEFILE_AVAILABLE
#endif	// MCU


#ifdef TEST_AVAILABLE_MOTOR
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams? */
	#define TEST_AVAILABLE_DIGITAL	/*!< Sollen die LEDs die digitale Sensorwerte anzeigen? */
#endif

#ifdef TEST_AVAILABLE_DIGITAL
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams? */
	#undef TEST_AVAILABLE_ANALOG
#endif

#ifdef TEST_AVAILABLE_ANALOG
	#define TEST_AVAILABLE			/*!< brauchen wir den Testkrams? */
#endif

#ifndef SPEED_CONTROL_AVAILABLE
	#undef ADJUST_PID_PARAMS
#endif

#ifdef LOG_UART_AVAILABLE
	#define LOG_AVAILABLE	/*!< LOG aktiv? */
#endif
#ifdef LOG_CTSIM_AVAILABLE
	#define LOG_AVAILABLE	/*!< LOG aktiv? */
#endif
#ifdef LOG_DISPLAY_AVAILABLE
	#define LOG_AVAILABLE	/*!< LOG aktiv? */
#endif
#ifdef LOG_STDOUT_AVAILABLE
	#define LOG_AVAILABLE	/*!< LOG aktiv? */
#endif
#ifdef LOG_MMC_AVAILABLE
	#define LOG_AVAILABLE	/*!< LOG aktiv? */
#endif

#ifndef MMC_AVAILABLE
	#undef SPEED_LOG_AVAILABLE
	#ifdef MCU
		#undef MAP_AVAILABLE // Map geht auf dem MCU nur mit MMC
		#undef MMC_VM_AVAILABLE
	#endif
#endif

#ifndef BEHAVIOUR_AVAILABLE
	#undef MAP_AVAILABLE
#endif

#if defined SPEED_LOG_AVAILABLE && defined MAP_AVAILABLE
	#undef MAP_AVAILABLE
	#warning "Speed-Log und Map schliessen sich aus, deaktiviere Map"
#endif

#ifdef MAP_AVAILABLE
	#define OS_AVAILABLE // Map braucht BotOS
#else
	#undef MAP_2_SIM_AVAILABLE
#endif

#ifndef BOT_2_BOT_AVAILABLE
	#undef BOT_2_BOT_PAYLOAD_AVAILABLE
#endif

#ifdef LOG_AVAILABLE
	#ifndef LOG_CTSIM_AVAILABLE
		#undef USE_MINILOG
	#endif
	#ifdef PC
		/* Auf dem PC gibts kein Logging ueber UART. */
		#undef LOG_UART_AVAILABLE
	#endif

	#ifdef MCU
		/* Mit Bot zu Sim Kommunikation auf dem MCU gibts kein Logging ueber UART.
		 * Ohne gibts keine Kommunikation ueber ct-Sim. */
		#undef LOG_STDOUT_AVAILABLE		/*!< MCU hat kein LOG_STDOUT */
		#ifdef BOT_2_SIM_AVAILABLE
			#undef LOG_UART_AVAILABLE
		#else
			#undef LOG_CTSIM_AVAILABLE
		#endif
	#endif	// MCU

	/* Ohne Display gibts auch keine Ausgaben auf diesem. */
	#ifndef DISPLAY_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
	#endif

	/* Es kann immer nur ueber eine Schnittstelle geloggt werden. */
	#ifdef LOG_UART_AVAILABLE
		#define UART_AVAILABLE			/*!< Serielle Kommunikation */
		#undef LOG_CTSIM_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
		#undef LOG_STDOUT_AVAILABLE
		#undef LOG_MMC_AVAILABLE
	#endif

	#ifdef LOG_CTSIM_AVAILABLE
		#undef LOG_DISPLAY_AVAILABLE
		#undef LOG_STDOUT_AVAILABLE
		#undef LOG_MMC_AVAILABLE
	#endif

	#ifdef LOG_DISPLAY_AVAILABLE
		#undef LOG_STDOUT_AVAILABLE
		#undef LOG_MMC_AVAILABLE
	#endif

	#ifdef LOG_STDOUT_AVAILABLE
		#undef LOG_MMC_AVAILABLE
	#endif

	#ifndef MMC_VM_AVAILABLE
		#undef LOG_MMC_AVAILABLE
	#endif

	// Wenn keine sinnvolle Log-Option mehr uebrig, loggen wir auch nicht
	#ifndef LOG_CTSIM_AVAILABLE
		#ifndef LOG_DISPLAY_AVAILABLE
			#ifndef LOG_UART_AVAILABLE
				#ifndef LOG_STDOUT_AVAILABLE
					#ifndef LOG_MMC_AVAILABLE
						#undef LOG_AVAILABLE
					#endif
				#endif
			#endif
		#endif
	#endif

#endif	// LOG_AVAILABLE


#ifdef SRF10_AVAILABLE
	#define TWI_AVAILABLE	/*!< TWI-Schnittstelle (I2C) nutzen */
#endif

#ifdef CMPS03_AVAILABLE
	#define I2C_AVAILABLE	/*!< I2C-Treiber verfuegbar fuer Kompassmodul */
#endif

#ifdef SP03_AVAILABLE
	#define I2C_AVAILABLE	/*!< I2C-Treiber verfuegbar fuer Sprachmodul */
#endif

#ifdef TWI_AVAILABLE
	#define I2C_AVAILABLE	/*!< I2C-Treiber statt TWI-Implementierung benutzen */
#endif

#include "global.h"
#include "bot-local.h"

#endif	// CT_BOT_H_DEF
