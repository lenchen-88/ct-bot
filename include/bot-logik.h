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

/*! @file 	bot-logik.h
 * @brief 	High-Level-Routinen fuer die Steuerung des c't-Bots
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	01.12.05
*/

#ifndef bot_logik_H_
#define bot_logik_H_

#include "global.h"
#include "ct-Bot.h"

/*! Verwaltungsstruktur fuer die Verhaltensroutinen */

typedef struct _Behaviour_t {
   void (*work) (struct _Behaviour_t *data); 	/*!< Zeiger auf die Funktion, die das Verhalten bearbeitet */
   
   uint8 priority;				/*!< Prioritaet */
   struct _Behaviour_t *caller ; /* aufrufendes verhalten */
   
   char active:1;				/*!< Ist das Verhalten aktiv */
   char subResult:2;			/*!< War das aufgerufene unterverhalten erfolgreich (==1)?*/
   struct _Behaviour_t *next;					/*!< Naechster Eintrag in der Liste */
#ifndef DOXYGEN
	}__attribute__ ((packed)) Behaviour_t;
#else
	} Behaviour_t;
#endif


extern volatile int16 target_speed_l;	/*!< Sollgeschwindigkeit linker Motor */
extern volatile int16 target_speed_r;	/*!< Sollgeschwindigkeit rechter Motor */

/*!
 * Kuemmert sich intern um die Ausfuehrung der goto-Kommandos
 * @see bot_goto()
 */
extern void bot_behave(void);

/*!
 * Initilaisert das ganze Verhalten
 */
extern void bot_behave_init(void);

/*!
 * Aktiviert eine Regel mit gegebener Funktion
 * @param function Die Funktion, die das Verhalten realisiert.
 */
void activateBehaviour(void *function);

/*!
 * Aktiviert eine Regel mit gegebener Funktion
 * @param function Die Funktion, die das Verhalten realisiert.
 */
void deactivateBehaviour(void *function);

/*!
 * Beispiel fuer ein Verhalten, das einen Zustand besitzt
 * es greift auf andere Verhalten zurueck und setzt daher 
 * selbst keine speedWishes
 * Laesst den Roboter ein Quadrat abfahren
 * @param *data der Verhaltensdatensatz
 */
void bot_drive_square(Behaviour_t *data);

/*!
 * Kuemmert sich intern um die Ausfuehrung der goto-Kommandos,
 * @param *data der Verhaltensdatensatz
 * @see bot_goto()
 */
void bot_goto_behaviour(Behaviour_t *data);

#endif
