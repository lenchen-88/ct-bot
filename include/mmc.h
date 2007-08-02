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
 * @file 	include/mmc.h
 * @brief 	Routinen zum Auslesen/Schreiben einer MMC-Karte
 * @author 	Benjamin Benz (bbe@heise.de)
 * @author  Ulrich Radig (mail@ulrichradig.de) www.ulrichradig.de
 * @date 	07.11.06
 */

#ifndef MMC_H_
#define MMC_H_

#include "ct-Bot.h"
#include "ui/available_screens.h"

#ifdef MMC_AVAILABLE

#define MMC_INFO_AVAILABLE			/*!< Die Karte kann uns einiges ueber sich verrraten, wenn wir sie danach fragen. Aber es kostet halt Platz im Flash */
//#define MMC_WRITE_TEST_AVAILABLE	/*!< Achtung dieser Test zerstoert die Daten auf der Karte!!! */

#ifdef SPI_AVAILABLE
#define mmc_read_sector(addr, buffer) mmc_read_sector_spi(0x51, addr, buffer)
#define mmc_read_block(cmd, buffer, length) mmc_read_sector_spi(cmd[0], 0, buffer)
#define mmc_write_sector(addr, buffer, async) mmc_write_sector_spi(addr, buffer, async)
#endif	// SPI_AVAILABLE

/*!
 * Checkt Initialisierung der Karte
 * @return	0, wenn initialisiert
 */
uint8_t mmc_get_init_state(void);

/*!
 * Schaltet die Karte aktiv und checkt dabei die Initialisierung
 * @return 			0 wenn alles ok ist, 1 wenn Init nicht moeglich
 * @author 			Timo Sandmann (mail@timosandmann.de)
 * @date 			09.12.2006
 */
uint8_t mmc_enable(void);

#ifndef SPI_AVAILABLE
/*!
 * Liest einen Block von der Karte
 * @param addr 		Nummer des 512-Byte Blocks
 * @param buffer 	Puffer von mindestens 512 Byte
 * @return 			0 wenn alles ok ist, 1 wenn Init nicht moeglich oder Timeout vor / nach Kommando 17
 * @author 			Timo Sandmann (mail@timosandmann.de)
 * @date 			17.11.2006
 * @see				mmc-low.s
 */	
uint8_t mmc_read_sector(uint32_t addr, uint8_t * buffer);

/*! 
 * Schreibt einen 512-Byte Sektor auf die Karte
 * @param addr 		Nummer des 512-Byte Blocks
 * @param buffer 	Zeiger auf den Puffer
 * @param async		0: synchroner, 1: asynchroner Aufruf, siehe MMC_ASYNC_WRITE in mmc-low.h
 * @return 			0 wenn alles ok ist, 1 wenn Init nicht moeglich oder Timeout vor / nach Kommando 24, 2 wenn Timeout bei busy
 * @author 			Timo Sandmann (mail@timosandmann.de)
 * @date 			16.11.2006
 * @see				mmc-low.s
 */
uint8_t mmc_write_sector(uint32_t addr, uint8_t * buffer, uint8_t async);

#else
/*!
 * Liest einen Block von der Karte
 * @param cmd		Kommando zum Lesen (0x51 fuer 512-Byte Block)
 * @param addr 		Adresse des Blocks
 * @param buffer 	Puffer fuer die Daten
 * @return 			0 wenn alles ok ist, 1 wenn Init nicht moeglich oder Timeout
 */	
uint8_t mmc_read_sector_spi(uint8_t cmd, uint32_t addr, uint8_t * buffer);

/*! 
 * Schreibt einen 512-Byte Sektor auf die Karte
 * @param addr 		Adresse des 512-Byte Blocks
 * @param buffer 	Zeiger auf den Puffer
 * @param async		0: synchroner, 1: asynchroner Aufruf, siehe MMC_ASYNC_WRITE in mmc-low.h
 * @return 			0 wenn alles ok ist, 1 wenn Init nicht moeglich oder Timeout vor / nach Kommando 24, 2 wenn Timeout bei busy
 */
uint8_t mmc_write_sector_spi(uint32_t addr, uint8_t * buffer, uint8_t async);
#endif	// SPI_AVAILABLE

/*! 
 * Initialisiere die MMC/SD-Karte
 * @return 0 wenn allles ok, sonst Nummer des Kommandos bei dem abgebrochen wurde
 */
uint8_t mmc_init (void);

#ifdef MMC_INFO_AVAILABLE
	/*!
	 * Liest das CSD-Register (16 Byte) von der Karte
	 * @param buffer Puffer von mindestens 16 Byte
	 */
	void mmc_read_csd (uint8_t * buffer);
	
	/*!
	 * Liest das CID-Register (16 Byte) von der Karte
	 * @param buffer Puffer von mindestens 16 Byte
	 */
	void mmc_read_cid (uint8_t * buffer);
	
	/*!
	 * Liefert die Groesse der Karte zurueck
	 * @return Groesse der Karte in Byte. Bei einer 4 GByte-Karte kommt 0xFFFFFFFF zurueck
	 */
	uint32_t mmc_get_size(void);
#endif	// MMC_INFO_AVAILABLE

#ifdef DISPLAY_MMC_INFO
	/*!
	 * Zeigt die Daten der MMC-Karte an
	 */
	void mmc_display(void);
#endif	// DISPLAY_MMC_INFO

#ifdef MMC_WRITE_TEST_AVAILABLE
	/*! Testet die MMC-Karte. Schreibt nacheinander 2 Sektoren a 512 Byte mit testdaten voll und liest sie wieder aus
	 * !!! Achtung loescht die Karte
	 * @return 0, wenn alles ok
	 */
	uint8_t mmc_test(void);
#endif	// MMC_WRITE_TEST_AVAILABLE

#endif	// MMC_AVAILABLE
#endif	// MMC_H_
