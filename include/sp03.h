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
 * @file 	sp03.h
 * @brief 	Ansteuerung des Sprachmoduls SP03
 * @author 	Harald W. Leschner (hari@h9l.net)
 * @date 	12.10.07
 */

#include "ct-Bot.h"
#ifdef SP03_AVAILABLE

#ifndef SP03_H_
#define SP03_H_

//#define SP03_V1	// schaltet alten Code an oder aus

// I2C bzw. TWI Adresse und konstante Parameter fuer das SP03 Sprachmodul
#define SP03_TWI_ADDRESS	0xC4    /*!< Die SP03 MODUL I2C Adresse */
#define SP03_COMMAND_REG	0x00    /*!< Das SP03 MODUL COMMAND Register */
#define SP03_VERSION_REG	0x01    /*!< Das SP03 MODUL VERSION Register */

#define SP03_COMMAND_NOP	0x00    /*!< Das SP03 MODUL NOP Commando */      
#define SP03_SPEAK_BUFFER	0x40    /*!< Das SP03 MODUL SPRICH! Commando */

#define SP03_MAX_VOLUME	  0x07 	    /*!< Lautstaerke 			0x00 = lauteste, 	0x07 = ganz leise = aus  */
#define SP03_MAX_PITCH	  0x07	    /*!< Stimmlage 				0x00 = hoechste, 	0x07 = tiefste 			 */
#define SP03_MAX_SPEED	  0x03	    /*!< Sprechgeschwindigkeit	0x00 = langsamste,	0x03 = schnellste 		 */

#define SP03_MAX_TEXT_SIZE	  81    /*!< maximal 81 Bytes an ASCII Text auf einmal in den Buffer laden*/
#define SP03_MAXNUMPHRASES	  30    /*!< Anzahl vordefinierter Saetze, momentan 30 */

#define SP03_DEFAULT_PHRASE	1       /*!< Standardsatz bei Fehleingabe PHRASE */   

#define SP03_DEFAULT_VOLUME 0x01	/*!< Standard Lautstaerke */
#define SP03_DEFAULT_PITCH 	0x04	/*!< Standard Stimmlage*/
#define SP03_DEFAULT_SPEED	0x01	/*!< Standard Geschwindigkeit  */

/*!
 * Laden eines Strings in den SP03, der im Flash gespeichert ist.
 * Schreibt die formatierte Ausgabe der zu sprechenden Zeichen in einen Puffer und
 * uebertraegt sie per I2C an das Sprachmodul. Die eigentliche Aussprache des
 * Textes erfolgt im Macro sp03_say() oder sp03_speakf().
 * 
 * @param format 	Format, wie beim printf
 * @param ... 		Variable Argumentenliste, wie beim printf
 */
extern void sp03_flash_speakf(const char* format, ...);
	
/*!
 * Endanwender-Makro fuer die Sprachausgabe mit default Stimmparameter.
 * @param format	Format, wie beim speakf
 * @param args 		Variable Argumentenliste, wie beim printf
 */
#define sp03_say(format, args... ) {		\
	sp03_set_voice(SP03_DEFAULT_VOLUME, SP03_DEFAULT_PITCH, SP03_DEFAULT_SPEED);	\
	static const char data[] PROGMEM = format;	\
	sp03_flash_speakf(data, ## args);		\
	sp03_cmd_speak();						\
}	 

/*!
 * Spricht einen String auf dem SP03, der String verbleibt im Flash
 * Makrofunktion zum Sprechen von Texten, die wie mit printf() erzeugt werden. Es sollte unbedingt
 * vor Aufruf immer ein sp03_set_voice() gesetzt werden, damit sich das Sprachmodul nicht durch
 * aeltere I2C Uebertragungen verschluckt.
 * 
 * @param format	Format, wie beim printf
 * @param args 		Variable Argumentenliste, wie beim printf
 */
#define sp03_speakf(format, args... ) {		\
	static const char data[] PROGMEM = format;	\
	sp03_flash_speakf(data, ## args);		\
	sp03_cmd_speak();						\
}
	 
/*!
 * SP03 Steuercodes fuer Lautstaerke, Speed und Pitch an Synth senden
 * @param sp03_volume Lautstaerke
 * @param sp03_pitch Geschwindigkeit
 * @param sp03_speed Stimmlage
 */
void sp03_set_voice(uint8_t sp03_volume, uint8_t sp03_pitch, uint8_t sp03_speed);

/*!
 * SP03 Steuercode fuer Sprechen senden
 */ 
void sp03_cmd_speak(void);

/*!
 * SP03 Vordefinierte Saetze abrufen 1-30 oder 0x01-0x1E
 * @param sp03_pre_id Vordefinierte Satz-ID-Nr
 */
void sp03_speak_phrase(uint8_t sp03_pre_id);

/*!
 * SP03 Firmwareversion auslesen
 */
void sp03_get_version(void);


// VERSION 1.0
#ifdef SP03_V1

#define SP03_DEFAULT_STRING "Ouch" 	/*!< OBSOLET: Standardsatz bei Fehleingabe STRING */
#define SP03_DEFAULT_LEN	4		/*!< OBSOLET: Laenge Standardsatz bei STRING Fehleingabe */

/*!
 * OBSOLET: SP03 Direktes Sprechen von ASCII Text max. 81 Zeichen. Hier wird in einer Funktion alles uebergeben.
 * @param sp03_volume Lautstaerke
 * @param sp03_speed Geschwindigkeit
 * @param sp03_pitch Stimmlage
 * @param *sp03_text Der zu sprechende Text
 */
void sp03_speak_string(uint8_t sp03_volume, uint8_t sp03_pitch, uint8_t sp03_speed, char * sp03_text);

/*!
 * OBSOLET: SP03 Text in den Buffer laden, max. 81 ASCII Zeichen
 * @param *sp03_textb Textbuffer
 */
void sp03_set_buffer(const char * sp03_textb);
#endif // SP03_V1

#endif // SP03_AVAILABLE
#endif // SP03_H_
