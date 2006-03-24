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

/*! @file 	mouse.c 
 * @brief 	Routinen fuer die Ansteuerung eines opt. Maussensors
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
*/
#include "global.h"


#ifdef MCU 

#include <avr/io.h>
#include "ct-Bot.h"
#include "mouse.h"

#ifdef MAUS_AVAILABLE

#define MAUS_DDR 	DDRB			/*!< DDR für Maus-SCLK */
#define MAUS_PORT 	PORTB			/*!< PORT für Maus-SCLK */
#define MAUS_SCK_PIN	(1<<7)		/*!< PIN für Maus-SCLK */

#define MAUS_SDA_NR		6		/*!< Pin an dem die SDA-Leitung haengt */
#define MAUS_SDA_PINR 	PINB		/*!< Leseregister */
#define MAUS_SDA_PIN 	(1<<MAUS_SDA_NR)	/*!< Bit-Wert der SDA-Leitung */

/*!
 * Überträgt ein Byte an den Sensor
 * @param data das Byte
 */
void maus_sens_writeByte(int8 data){
	int8 i;
	MAUS_DDR  |= MAUS_SDA_PIN; 		// SDA auf Output
	
	for (i=7; i>=0; i--){
		MAUS_PORT &= ~MAUS_SCK_PIN;		// SCK auf Low, vorbereiten
		
		//Daten rausschreiben
		MAUS_PORT = (MAUS_PORT & (~MAUS_SDA_PINR)) |  ((data >> (7 - MAUS_SDA_NR)) & MAUS_SDA_PIN);	
		data = data <<1;		// nächstes Bit vorbereiten
		
		MAUS_PORT |= MAUS_SCK_PIN;		// SCK =1 Sensor uebernimmt auf steigender Flanke
	}
}

/*!
 * Liest ein Byte vom Sensor
 * @return das Byte
 */
int8 maus_sens_readByte(void){
	int i;
	char data=0;

	MAUS_DDR  &= ~MAUS_SDA_PIN; 	// SDA auf Input

	for (i=7; i>-1; i--){
		MAUS_PORT &= ~MAUS_SCK_PIN;		// SCK =0 Sensor bereitet Daten auf fallender Flanke vor !
		data=data<<1;					// Platz schaffen
		MAUS_PORT |= MAUS_SCK_PIN;		// SCK =1 Daten lesen  auf steigender Flanke
		
		data |= (MAUS_SDA_PINR >> MAUS_SDA_NR) & 0x01;			//Daten lesen
	}
	return data;
}

/*!
 * Überträgt ein write-Kommando an den Sensor
 * @param adr Adresse
 * @param data Datum
 */
void maus_sens_write(int8 adr, uint8 data){
	int i;
	maus_sens_writeByte(adr);
	maus_sens_writeByte(data);
	for (i=0; i<75; i++){asm("nop");}	// mindestens 100 Mikrosekunden Pause!!!
}

/*!
 * Schickt ein Lesekommando an den Sensor
 * und liest ein Byte zurück
 * @param adr die Adresse
 * @return das Datum
 */
int8 maus_sens_read(uint8 adr){
	int i;
	maus_sens_writeByte(adr);
	for (i=0; i<75; i++){asm("nop");}	// mindestens 100 Mikrosekunden Pause!!!
	return maus_sens_readByte();
}

/*! 
 * Initialisiere Maussensor
 */ 
void maus_sens_init(void){
	MAUS_DDR  |= MAUS_SCK_PIN; 	// SCK auf Output
	MAUS_PORT &= ~MAUS_SCK_PIN;	// SCK auf 0
	
	maus_sens_write(MOUSE_CONFIG_REG,MOUSE_CFG_RESET);	//Reset sensor
	maus_sens_write(MOUSE_CONFIG_REG,MOUSE_CFG_FORCEAWAKE);	//Always on
}


#endif
#endif
