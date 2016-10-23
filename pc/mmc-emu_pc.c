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
 * \file 	mmc-emu_pc.c
 * \brief 	MMC / SD-Card Emulation fuer PC
 * \author 	Timo Sandmann (mail@timosandmann.de)
 * \date 	10.12.2006
 */

/* Die PC-Emulation einer MMC / SD-Card ermoeglicht es, am PC dieselben Funktionen zu benutzen, wie bei einer echten
 * MMC / SD-Card. Als Speichermedium dient hier eine Datei (MMC_EMU_FILE), deren Groesse sich mit MMC_EMU_SIZE in Byte 
 * einstellen laesst. Ist die Datei nicht vorhanden oder derzeit kleiner als MMC_EMU_SIZE, wird sie angelegt bzw. 
 * vergroessert. Achtung, startet man den C-Bot vom Sim aus, liegt die Datei, falls kein absoluter Pfad angegeben wurde, im
 * Verzeichnis dem ct-Sims, von der Konsole aus gestartet erwartet / erzeugt der Code die Datei im Unterverzeichnis 
 * "Debug-Linux" bzw. "Debug-W32".
 * Eine sinnvolle (und die derzeit einzig moegliche) Verwendung der Emulation ergibt sich im Zusammenspiel mit dem
 * Virtual Memory Management fuer MMC. Ein Verhalten kann in diesem Fall immer auf dieselbe Art und Weise Speicher anfordern,
 * je nach System liegt dieser physisch dann entweder auf einer MMC / SD-Card (MCU) oder in einer Datei (PC). Fuer das
 * Verhalten ergibt sich kein Unterschied und es kann einfach derselbe Code verwendet werden.
 * Moechte man die Funktion mmc_fopen() benutzen, also auf FAT16-Dateien zugreifen, so ist zu beachten, dass deren "Dateiname"
 * in der Datei fuer die Emulation am Anfang eines 512 Byte grossen Blocks steht (denn auf einer echten MMC / SD-Card ezeugt
 * ein Betriebssystem eine neue Datei immer am Anfang eines Clusters und mmc_fopen() sucht nur dort nach dem "Dateinamen").
 * Im Moment gibt es noch keine Funktion zum Anlegen einer neuen Datei auf einer echten oder emulierten MMC / SD-Card. 
 * Die Code der Emulation ist voellig symmetrisch zum Code fuer eine echte MMC / SD-Card aufgebaut.  
 */
 
#ifdef PC
#include "ct-Bot.h"

#ifdef MMC_VM_AVAILABLE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mmc-emu.h"
#include "mmc-vm.h"
#include "display.h"
#include "ui/available_screens.h"
#include "delay.h"

volatile uint8_t mmc_emu_init_state = 1;	/**< Initialierungsstatus der Karte, 0: ok, 1: Fehler  */
static FILE * mmc_emu_file;					/**< Der Inhalt der emulierten Karte wird einfach in eine Datei geschrieben */

#ifdef DISPLAY_MINIFAT_INFO
/**
 * Hilfsfunktion, die eine 23-Bit Blockadresse auf dem Display als hex anzeigt.
 * Da display_printf() @MCU maximal mit 16 Bit Zahlen umgehen kann, zerlegt diese Funktion die Adresse ein zwei Teile.
 */
static void display_block(uint32_t addr) {
	uint16_t low  = (uint16_t)addr;
	uint16_t high = (uint16_t)(addr>>16);
	display_printf("0x%02x%04x", high, low);
}

/**
 * Display-Screen fuer Ausgaben des MiniFAT-Treibers, falls dieser welche erzeugt.
 * Da die MiniFat-Funktionen im Wesentlichen den aktuellen Suchstatus der MMC
 * ausgeben, erfolgt die eigentliche Ausgabe in der jeweiligen Schleife der 
 * MiniFAT-Funktion, dieser Screen ist dafuer nur ein Platzhalter
 */
void mini_fat_display(void) {
	display_cursor(1,1);
	display_puts("MiniFAT:");
}
#endif	// DISPLAY_MINIFAT_INFO

/**
 * Checkt Initialisierung der emulierten Karte
 * \return	0, wenn initialisiert
 * @see		mcu/mmc.c
 */
uint8_t mmc_emu_get_init_state(void){
	return mmc_emu_init_state;	
}

/**
 * Initialisiere die emulierte SD/MMC-Karte
 * \return	0 wenn allles ok, sonst 1
 * @see		mcu/mmc.c 
 */
uint8_t mmc_emu_init(void) {
	mmc_emu_init_state = 0;
	mmc_emu_file = fopen(MMC_EMU_FILE, "r+b");		// Datei versuchen zu oeffnen
	if (mmc_emu_file == NULL){
		/* Datei existiert noch nicht oder kann nicht erzeugt werden */
		mmc_emu_file = fopen(MMC_EMU_FILE, "w+b");	// Datei neu anlegen
		if (mmc_emu_file == NULL) {
			/* Datei kann nicht erzeugt werden */
			mmc_emu_init_state = 1;
			return 1;
		}	
	}
	if (mmc_emu_get_size() < MMC_EMU_SIZE) {
		/* vorhandene Datei ist zu klein, also auf MMC_EMU_SIZE vergroessern */
		mmc_emu_init_state = 1;
		if (fseek(mmc_emu_file, MMC_EMU_SIZE-1, SEEK_SET) != 0) return 2;
		if (putc(0, mmc_emu_file) != 0) return 3;
		if (fflush(mmc_emu_file) != 0) return 4;
		mmc_emu_init_state = 0;
	}
	return 0;	
}

/**
 * Liest einen Block von der emulierten Karte
 * \param addr 		Nummer des 512-Byte Blocks
 * \param buffer 	Puffer von mindestens 512 Byte
 * \return 			0 wenn alles ok ist
 * @see				mcu/mmc.c
 */	
uint8_t mmc_emu_read_sector(uint32_t addr, uint8_t * buffer) {
	if (mmc_emu_get_init_state() != 0 && mmc_emu_init() !=0) return 1;
	if (fseek(mmc_emu_file, addr<<9, SEEK_SET) != 0) return 2;	// Adresse in Byte umrechnen und an Dateiposition springen
	if (fread(buffer, 512, 1, mmc_emu_file) != 1) return 3;		// Block lesen
	return 0;	
}

/**
 * Schreibt einen 512-Byte Sektor auf die emulierte Karte
 * \param addr 		Nummer des 512-Byte Blocks
 * \param buffer 	Zeiger auf den Puffer
 * \return 			0 wenn alles ok ist
 * @see				mcu/mmc.c
 */
uint8_t mmc_emu_write_sector(uint32_t addr, uint8_t * buffer) {
	if (mmc_emu_get_init_state() != 0 && mmc_emu_init() !=0) return 1;
	if (fseek(mmc_emu_file, addr<<9, SEEK_SET) != 0) return 2;	// Adresse in Byte umrechnen und an Dateiposition springen
	if (fwrite(buffer, 512, 1, mmc_emu_file) != 1) return 3;	// Block schreiben
	if (fflush(mmc_emu_file) != 0) return 4;					// Puffer leeren
	return 0;	
}

/**
 * Liefert die Groesse der Karte zurueck
 * \return	Groesse der emulierten Karte in Byte.
 */
uint32_t mmc_emu_get_size(void) {
	if (mmc_emu_get_init_state() != 0 && mmc_emu_init() !=0) return 0;
	if (fseek(mmc_emu_file, 0L, SEEK_END) != 0) return 0;	// Groesse der emulierten Karte = Groesse der Datei
	return ftell(mmc_emu_file)+1;
}

/**
 * Liest die Groesse einer Datei im MiniFAT-Dateisystem aus 
 * \param file_start	Anfangsblock der Datei (Nutzdaten, nicht Header)
 * \return				Groesse der Datei in Byte, 0 falls Fehler
 */
uint32_t mmc_emu_get_filesize(uint32_t file_start) {
	file_len_t length;
	uint8_t * buffer = malloc(512);
	if (mmc_emu_read_sector(file_start-1, buffer) != 0) return 0;
	/* Dateilaenge aus Block 0, Byte 256 bis 259 der Datei lesen */
	uint8_t i;
	for (i=0; i<4; i++) {
		length.u8[i] = buffer[259-i];
	}
	return length.u32;	
}

/**
 * Leert eine Datei im MiniFAT-Dateisystem
 * \param file_start	Anfangsblock der Datei
 */
void mmc_emu_clear_file(uint32_t file_start) {
	uint32_t length = mmc_emu_get_filesize(file_start) >> 9;
	uint8_t * buffer = malloc(512);
	memset(buffer, 0, 512);	// Puffer leeren
	/* Alle Bloecke der Datei mit dem 0-Puffer ueberschreiben */
	uint32_t addr;
	for (addr=file_start; addr<file_start+length; addr++) {
		if (mmc_emu_write_sector(addr, buffer) != 0) return;
	}
}

/**
 * \brief			Sucht die Adresse einer Mini-FAT-Datei im EERROM
 * \param filename	Datei-ID
 * \param buffer	Zeiger auf 512 Byte grossen Speicherbereich (wird ueberschrieben)
 * \return			(Byte-)Adresse des ersten Nutzdatenblock der gesuchten Datei oder 0, falls nicht im EEPROM
 * Nur DUMMY fuer MMC-Emulation am PC. Wenn es mal eine EEPROM-Emulation fuer PC gibt, kann man diese Funktion implementieren.
 */
uint32_t mmc_emu_fat_lookup_adr(const char * filename, uint8_t * buffer) {
	/* keine warnings */
	filename = filename;
	buffer = buffer;
	// absichtlich leer
#ifdef DISPLAY_MINIFAT_INFO
	display_cursor(2, 1);
	display_puts("no EEPROM:");
	display_block(0);
#endif // DISPLAY_MINIFAT_INFO
	return 0;
}

/**
 * \brief			Speichert die Adresse einer Mini-FAT-Datei in einem EERROM-Slab
 * \param block		(Block-)Adresse der Datei, die gespeichert werden soll
 * Nur DUMMY fuer MMC-Emulation am PC. Wenn es mal eine EEPROM-Emulation fuer PC gibt, kann man diese Funktion implementieren.
 */
void mmc_emu_fat_store_adr(uint32_t block) {
	block = block; // kein warning
	// absichtlich leer
#ifdef DISPLAY_MINIFAT_INFO
	display_cursor(3, 1);
	display_puts("no EEPROM:");
	display_cursor(3, 13);
	display_block(0);
#endif // DISPLAY_MINIFAT_INFO
}

/**
 * \brief			Sucht einen Block auf der MMC-Karte, dessen erste Bytes dem Dateinamen entsprechen
 * \param filename 	String im Flash zur Identifikation
 * \param buffer 	Zeiger auf 512 Byte Puffer im SRAM
 * \param end_addr	Byte-Adresse, bis zu der gesucht werden soll
 * \return			Anfangsblock der Nutzdaten der Datei
 * Achtung das Prinzip geht nur, wenn die Dateien nicht fragmentiert sind 
 */
uint32_t mmc_emu_find_block(const char * filename, uint8_t * buffer, uint32_t end_addr) {
	end_addr >>= 9;	// letzte Blockadresse ermitteln
	
	/* zunaechst im EEPROM-FAT-Cache nachschauen */
	uint32_t block = mmc_emu_fat_lookup_adr(filename, buffer);
	if (block != 0)	return block;
	
#ifdef DISPLAY_MINIFAT_INFO
	display_cursor(2,1);
	display_printf("Find %s: ", filename);
#endif	// DISPLAY_MINIFAT_INFO
	
	/* sequenziell die Karte durchsuchen */
	for (block=0; block<end_addr; block++) {
#ifdef DISPLAY_MINIFAT_INFO
		display_cursor(2,13);
		display_block(block);
#endif	// DISPLAY_MINIFAT_INFO
		if (mmc_emu_read_sector(block, buffer) != 0) return 0xffffffff;
		if (strcmp((char *)buffer, filename) == 0) {
			/* gefunden, Nutzdaten laden und Adresse ins EEPROM schreiben */
			if (mmc_emu_read_sector(++block, buffer) != 0) return 0xffffffff;
			mmc_emu_fat_store_adr(block);
#ifdef DISPLAY_MINIFAT_INFO
			display_cursor(4,1);
			display_puts("Found:");
			display_cursor(4,7);
			display_block(block);
#endif // DISPLAY_MINIFAT_INFO
			return block;
		}
	}
	display_cursor(4,1);
	display_puts("Not found :(");
	return 0xffffffff;	
}

/**
 * Testet VM und MMC / SD-Card Emulation am PC
 */
uint8_t mmc_emu_test(void) {
	/* Initialisierung checken */
	if (mmc_emu_init_state != 0 && mmc_emu_init() != 0) return 1;
	uint16_t i;
	static uint16_t pagefaults = 0;
	/* virtuelle Adressen holen */
	static uint32_t v_addr1 = 0;
	static uint32_t v_addr2 = 0;
	static uint32_t v_addr3 = 0;
	static uint32_t v_addr4 = 0;
	if (v_addr1 == 0) v_addr1 = mmcalloc(512, 1);	// Testdaten 1
	if (v_addr2 == 0) v_addr2 = mmcalloc(512, 1);	// Testdaten 2
	if (v_addr3 == 0) v_addr3 = mmcalloc(512, 1);	// Dummy 1
	if (v_addr4 == 0) v_addr4 = mmcalloc(512, 1);	// Dummy 2
	/* Pointer auf Puffer holen */
	uint8_t * p_addr = mmc_get_data(v_addr1);
	if (p_addr == NULL) return 2;
	/* Testdaten schreiben */
	for (i=0; i<512; i++)
		p_addr[i] = (i & 0xff);
	/* Pointer auf zweiten Speicherbereich holen */
	p_addr = mmc_get_data(v_addr3);
	if (p_addr == NULL)	return 3;
	/* Testdaten Teil 2 schreiben */
	for (i=0; i<512; i++)
		p_addr[i] = 255 - (i & 0xff);			
	/* kleiner LRU-Test */
		p_addr = mmc_get_data(v_addr1);
		p_addr = mmc_get_data(v_addr4);
		p_addr = mmc_get_data(v_addr1);						
		p_addr = mmc_get_data(v_addr3);
		p_addr = mmc_get_data(v_addr1);
		p_addr = mmc_get_data(v_addr4);						
	/* Pointer auf Testdaten Teil 1 holen */	
	p_addr = mmc_get_data(v_addr1);
	if (p_addr == NULL) return 4;		
	/* Testdaten 1 vergleichen */
	for (i=0; i<512; i++)
		if (p_addr[i] != (i & 0xff)) return 5;
	/* Pointer auf Testdaten Teil 2 holen */
	p_addr = mmc_get_data(v_addr3);
	if (p_addr == NULL) return 6;		
	/* Testdaten 2 vergleichen */
	for (i=0; i<512; i++)
		if (p_addr[i] != (255 - (i & 0xff))) return 7;
	#ifdef VM_STATS_AVAILABLE 
		/* Pagefaults merken */		
		pagefaults = mmc_get_pagefaults();
	#endif
	/* kleine Statistik ausgeben */
	display_cursor(3, 1);
	display_printf("Pagefaults: %5u  ", pagefaults);
	// hierher kommen wir nur, wenn alles ok ist		
	return 0;
}

#endif // MMC_VM_AVAILABLE
#endif // PC
