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
 * \file 	sensor_correction.h
 * \brief 	Kalibrierungsdaten fuer die IR-Sensoren
 * \author 	Timo Sandmann (mail@timosandmann.de)
 * \date 	21.04.2007
 */
#ifndef SENSOR_CORRECTION_H_
#define SENSOR_CORRECTION_H_

/* Mit diesen Daten wird das EEPROM des realen Bots initialisiert.
 * Im Falle eines simulierten Bots fuer den Sim liegen die Daten im RAM.
 * Das Kalibrierungsverhalten gibt die hier eizutragenden Daten am Ende
 * schon richtig vorformatiert per LOG aus, so dass man sie nur per
 * copy & paste aus dem LOG-Fenster hierher uebernehmen muss.
 */
#if defined MCU && ! defined DISTSENS_TYPE_GP2Y0A60
/**
 * Wertepaare (BOT) fuer IR-Sensoren LINKS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsteigende Sortierung!
 */
#define SENSDIST_DATA_LEFT { \
	{498,100},{346,150},{266,200},{216,250},{180,300},{152,350},{138,400},{116,450},{104,500},{96,550},{84,600},{74,650},{76,700},{64,750} \
}
/**
 * Wertepaare (BOT) fuer IR-Sensoren RECHTS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsetigende Sortierung!
 */
#define SENSDIST_DATA_RIGHT { \
	{496,100},{344,150},{266,200},{214,250},{176,300},{144,350},{128,400},{112,450},{96,500},{78,550},{70,600},{72,650},{70,700},{54,750} \
}
#endif

#if defined MCU && defined DISTSENS_TYPE_GP2Y0A60
/**
 * Wertepaare (BOT) fuer IR-Sensoren LINKS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsteigende Sortierung!
 */
#define SENSDIST_DATA_LEFT { \
	{ 889,  60},{ 720, 100},{ 517, 150},{ 403, 200},{ 335, 250},{ 292, 300},{ 261, 350},{ 237, 400},{ 224, 450},{ 207, 500}, \
	{ 189, 550},{ 179, 600},{ 161, 650},{ 155, 700},{ 144, 750},{ 144, 800},{ 134, 850},{ 130, 900},{ 118, 950},{ 134,1000}  \
}
/**
 * Wertepaare (BOT) fuer IR-Sensoren RECHTS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsetigende Sortierung!
 */
#define SENSDIST_DATA_RIGHT { \
	{ 865,  60},{ 635, 100},{ 370, 150},{ 247, 200},{ 233, 250},{ 221, 300},{ 192, 350},{ 175, 400},{ 159, 450},{ 156, 500}, \
	{ 119, 550},{  87, 600},{  73, 650},{  61, 700},{  54, 750},{  50, 800},{  45, 850},{  40, 900},{  35, 950},{  30,1000}  \
}
#endif

#if defined PC && ! defined DISTSENS_TYPE_GP2Y0A60
/**
 * Wertepaare (SIM) fuer IR-Sensoren LINKS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsteigende Sortierung!
 */
#define SENSDIST_DATA_LEFT { \
	{510/2,100/5},{376/2,150/5},{292/2,200/5},{244/2,250/5},{204/2,300/5},{184/2,350/5},{168/2,400/5}, \
	{156/2,450/5},{144/2,500/5},{136/2,550/5},{130/2,600/5},{126/2,650/5},{120/2,700/5},{114/2,750/5}  \
}
/**
 * Wertepaare (SIM) fuer IR-Sensoren RECHTS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsetigende Sortierung!
 */
#define SENSDIST_DATA_RIGHT { \
	{494/2,100/5},{356/2,150/5},{276/2,200/5},{230/2,250/5},{188/2,300/5},{164/2,350/5},{144/2,400/5}, \
	{128/2,450/5},{116/2,500/5},{106/2,550/5},{98/2,600/5},{90/2,650/5},{84/2,700/5},{80/2,750/5}      \
}
#endif

#if defined PC && defined DISTSENS_TYPE_GP2Y0A60
/**
 * Wertepaare (SIM) fuer IR-Sensoren LINKS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsteigende Sortierung!
 */
#define SENSDIST_DATA_LEFT { \
	{ 889,  60},{ 720, 100},{ 517, 150},{ 403, 200},{ 335, 250},{ 292, 300},{ 261, 350},{ 237, 400},{ 224, 450},{ 207, 500}, \
	{ 189, 550},{ 179, 600},{ 161, 650},{ 155, 700},{ 144, 750},{ 144, 800},{ 134, 850},{ 130, 900},{ 118, 950},{ 134,1000}  \
}
/**
 * Wertepaare (SIM) fuer IR-Sensoren RECHTS. Es ist jeweils (Spannung | Distanz) gespeichert.
 * Aufsetigende Sortierung!
 */
#define SENSDIST_DATA_RIGHT { \
	{ 865,  60},{ 635, 100},{ 370, 150},{ 247, 200},{ 233, 250},{ 221, 300},{ 192, 350},{ 175, 400},{ 159, 450},{ 156, 500}, \
	{ 119, 550},{  87, 600},{  73, 650},{  61, 700},{  54, 750},{  50, 800},{  45, 850},{  40, 900},{  35, 950},{  30,1000}  \
}
#endif

#endif // SENSOR_CORRECTION_H_
