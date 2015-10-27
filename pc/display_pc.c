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
 * \file 	display_pc.c
 * \brief 	Routinen zur Displaysteuerung
 * \author 	Benjamin Benz (bbe@heise.de)
 * \date 	20.12.2005
 */

#ifdef PC
#include "ct-Bot.h"

#ifdef DISPLAY_AVAILABLE
#include "display.h"
#include "command.h"
#include "bot-2-sim.h"
#include "bot-2-atmega.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

uint8_t display_screen = 0; /**< zurzeit aktiver Displayscreen */

static char display_buf[DISPLAY_BUFFER_SIZE]; /**< Pufferstring fuer Displayausgaben */
static int16_t last_row = 0;
static int16_t last_column = 0;

/**
 * Loescht das ganze Display
 */
void display_clear(void) {
	command_write(CMD_AKT_LCD, SUB_LCD_CLEAR, 0, 0, 0);
}

/**
 * Positioniert den Cursor
 * \param row		Zeile
 * \param column	Spalte
 */
void display_cursor(int16_t row, int16_t column) {
	last_row = row - 1;
	last_column = column - 1;
/** \todo bot-2-linux anpassen */
	command_write(CMD_AKT_LCD, SUB_LCD_CURSOR, last_column, last_row, 0);
}

/**
 * Initialisiert das Display
 */
void display_init(void) {
	display_clear();
}

/**
 * Schreibt einen String auf das Display.
 * \param *format	Format, wie beim printf
 * \param ... 		Variable Argumentenliste, wie beim printf
 * \return			Anzahl der geschriebenen Zeichen
 */
uint8_t display_printf(const char * format, ...) {
	va_list	args;

	va_start(args, format);
	uint8_t len = vsnprintf(display_buf, DISPLAY_BUFFER_SIZE, format, args);
	va_end(args);
	if (len > DISPLAY_LENGTH) {
		len = DISPLAY_LENGTH;
	}

	command_write_rawdata(CMD_AKT_LCD, SUB_LCD_DATA, last_column, last_row, len, display_buf);
#if defined ARM_LINUX_BOARD && defined BOT_2_SIM_AVAILABLE && defined DISPLAY_REMOTE_AVAILABLE
	set_bot_2_sim();
	command_write_rawdata(CMD_AKT_LCD, SUB_LCD_DATA, last_column, last_row, len, display_buf);
	set_bot_2_atmega();
#endif // ARM_LINUX_BOARD && BOT_2_SIM_AVAILABLE && DISPLAY_REMOTE_AVAILABLE
	last_column += len;

	return len;
}

/**
 * Gibt einen String auf dem Display aus
 * \param *text	Zeiger auf den auszugebenden String
 * \return		Anzahl der geschriebenen Zeichen
 */
uint8_t display_puts(const char * text) {
	uint8_t len = strlen(text);
	if (len > DISPLAY_LENGTH) {
		len = DISPLAY_LENGTH;
	}
	command_write_rawdata(CMD_AKT_LCD, SUB_LCD_DATA, last_column, last_row, len, text);
#if defined ARM_LINUX_BOARD && defined BOT_2_SIM_AVAILABLE && defined DISPLAY_REMOTE_AVAILABLE
	set_bot_2_sim();
	command_write_rawdata(CMD_AKT_LCD, SUB_LCD_DATA, last_column, last_row, len, display_buf);
	set_bot_2_atmega();
#endif // ARM_LINUX_BOARD && BOT_2_SIM_AVAILABLE && DISPLAY_REMOTE_AVAILABLE
	last_column += len;
//	command_write(CMD_AKT_LCD, SUB_LCD_CURSOR, last_column, last_row, 0);

	return len;
}
#endif // DISPLAY_AVAILABLE
#endif // PC
