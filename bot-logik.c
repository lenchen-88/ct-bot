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

/*! @file 	bot-logik.c
 * @brief 	High-Level Routinen fuer die Steuerung des c't-Bots.
 * Diese Datei sollte der Einstiegspunkt fuer eigene Experimente sein, 
 * den Roboter zu steuern.
 * 
 * bot_behave() arbeitet eine Liste von Verhalten ab. 
 * Jedes Verhalten kann entweder absolute Werte setzen, dann kommen niedrigerpriorisierte nicht mehr dran
 * Alternativ dazu kann es Modifikatoren aufstellen, die bei niedriger priosierten angewendet werden.
 * bot_behave_init() baut diese Liste auf.
 * Jede Verhaltensfunktion bekommt einen Verhaltensdatensatz übergeben, in den Sie ihre Daten eintraegt
 * 
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	01.12.05
*/
#include <stdio.h>

#include "ct-Bot.h"
#include "motor.h"
#include "sensor.h"
#include "bot-logik.h"
#include "display.h"

#include "rc5.h"
#include <stdlib.h>

#define BOT_SPEED_IGNORE	1000	/*!< Wert ausserhalb von -BOT_SPEED_MAX und BOT_SPEED_MAX wird verwendet um einen Eintrag zu ignorieren */

#define BORDER_DANGEROUS	500		/*!< Wert, ab dem wir sicher sind, dass es eine Kante ist */

#define COL_CLOSEST		100		/*!< Abstand in mm, den wir als zu nah betrachten */
#define COL_NEAR			200		/*!< Nahbereich */
#define COL_FAR			400		/*!< Fernbereich */

#define ZONE_CLOSEST	0			/*!< Zone fuer extremen Nahbereich */
#define ZONE_NEAR		1			/*!< Zone fuer Nahbereich */
#define ZONE_FAR		2			/*!< Zone fuer Fernbereich */
#define ZONE_CLEAR		3			/*!< Zone fuer Freien Bereich */

#define BRAKE_CLOSEST 	-1.0		/*!< Bremsfaktor fuer extremen Nahbereich ( <1 ==> bremsen <0 ==> rueckwaerts) */
#define BRAKE_NEAR		0.6			/*!< Bremsfaktor fuer Nahbereich ( <1 ==> bremsen <0 ==> rueckwaerts) */
#define BRAKE_FAR		0.8			/*!< Bremsfaktor fuer Fernbereich ( <1 ==> bremsen <0 ==> rueckwaerts) */

#define MOT_GOTO_MAX  3 		/*!< Richtungsaenderungen, bis goto erreicht sein muss */


char col_zone_l=ZONE_CLEAR;			/*!< Kollisionszone, in der sich der linke Sensor befindet */
char col_zone_r=ZONE_CLEAR;			/*!< Kollisionszone, in der sich der rechte Sensor befindet */

volatile int16 mot_l_goto=0;	/*!< Speichert, wie weit der linke Motor drehen soll */
volatile int16 mot_r_goto=0;	/*!< Speichert, wie weit der rechte Motor drehen soll */

volatile int16 mot_goto_l=0;	/*!< Muss der linke Motor noch drehen?  */
volatile int16 mot_goto_r=0;	/*!< Muss der rechte Motor noch drehen?  */

volatile int16 target_speed_l=0;	/*!< Sollgeschwindigkeit linker Motor */
volatile int16 target_speed_r=0;	/*!< Sollgeschwindigkeit rechter Motor */


/*! Verwaltungsstruktur für die Verhaltensroutinen */
typedef struct _Behaviour_t {
   void (*work) (struct _Behaviour_t *data); 	/*!< Zeiger auf die Funktion, die das Verhalten bearbeitet */
   int16 speed_l;				/*!< Zielvariablen der Funktion links (wenn != 0, dann werden untere Prioritäten ignoriert)*/
   int16 speed_r;				/*!< Zielvariablen der Funktion rechts (wenn != 0, dann werden untere Prioritäten ignoriert) */

   float faktor_l;				/*!< Modifikationsfaktor für die Geschwindigkeit unterer Prioritäten links*/
   float faktor_r;				/*!< Modifikationsfaktor für die Geschwindigkeit unterer Prioritäten rechts */
   
   uint8 priority;				/*!< Priorität */
   char active:1;				/*!< Ist das Verhalten aktiv */
   char ignore:1;				/*!< Sollen alle Zielwerte ignoriert werden */
   struct _Behaviour_t *next;					/*!< Naechster Eintrag in der Liste */
#ifndef DOXYGEN
	}__attribute__ ((packed)) Behaviour_t;
#else
	} Behaviour_t;
#endif

/*! Liste mit allen Verhalten */
Behaviour_t *behaviour = NULL;


/*! 
 * Ein ganz einfaches Verhalten 
 * Es hat maximale Prioritaet
 * Hier kann man ganz einfach spielen. Eleganter ist es jedoch, dieses Nicht zu verwenden!!!
 * Wer Lust hat sich mit dem ganzen Verhaltensframework zu beschäfigen, kann diese Funktion getrost auskommentieren
 * und findet dann in bot_behave_init() und bot_behave() weiter Hinweise ....
 * @param *data der Verhaltensdatensatz
 */
void bot_simple(Behaviour_t *data){
/*  int16 speed_l_col, speed_r_col;
  data->speed_l=BOT_SPEED_MAX;
  data->speed_r=BOT_SPEED_MAX;
  if (sensDistL < COL_NEAR)
    speed_r_col=-speed_r-BOT_SPEED_NORMAL;
  else speed_r_col=0;
  
  if (sensDistR < COL_NEAR)
    speed_l_col=-speed_l-BOT_SPEED_FAST;
  else speed_l_col=0;

  data->speed_l+=speed_l_col;
  data->speed_r+=speed_r_col;  
 */
}

/*! 
 * Das basisverhalten Grundverhalten 
 * @param *data der Verhaltensdatensatz
 */
void bot_base(Behaviour_t *data){
	data->speed_l=target_speed_l;
	data->speed_r=target_speed_r;
}


/*!
 * Kuemmert sich intern um die Ausfuehrung der goto-Kommandos,
 * @param *data der Verhaltensdatensatz
 * @see bot_goto()
 */
void bot_goto_system(Behaviour_t *data){

  	int diff_l = sensEncL - mot_l_goto;	/* Restdistanz links */
	int diff_r = sensEncR - mot_r_goto;	/* Restdistanz rechts */
	
	/* Motor L hat noch keine MOT_GOTO_MAX Nulldurchgaenge gehabt */
	if (mot_goto_l >0){
		if (abs(diff_l) <= 2){	/* 2 Encoderstaende Genauigkeit reicht */
			data->speed_l = BOT_SPEED_STOP;	/* Stop */
			mot_goto_l--;			/* wie Nulldurchgang behandeln */
		}else if (abs(diff_l) < 4)
			data->speed_l= BOT_SPEED_SLOW;
		else if (abs(diff_l) < 10)
			data->speed_l= BOT_SPEED_NORMAL;
		else if (abs(diff_l) < 40)
			data->speed_l= BOT_SPEED_FAST;
		else data->speed_l= BOT_SPEED_MAX;

		// Richtung	
		if (diff_l>0) {		// Wenn uebersteuert,
			data->speed_l= -data->speed_l;	//Richtung umkehren
		}
		
		// Wenn neue Richtung ungleich alter Richtung
		if (((data->speed_l<0)&& (speed_l>0))|| ( (data->speed_l>0) && (speed_l<0) ) ) 
			mot_goto_l--;		// Nulldurchgang merken
	} else
		data->speed_l = BOT_SPEED_IGNORE;

	// Motor R hat noch keine MOT_GOTO_MAX Nulldurchgaenge gehabt
	if (mot_goto_r >0){
		if (abs(diff_r) <= 2){			// 2 Encoderstaende Genauigkeit reicht
			data->speed_r = BOT_SPEED_STOP;	//Stop
			mot_goto_r--;			// wie Nulldurchgang behandeln
		}else if (abs(diff_r) < 4)
			data->speed_r= BOT_SPEED_SLOW;
		else if (abs(diff_r) < 10)
			data->speed_r= BOT_SPEED_NORMAL;
		else if (abs(diff_r) < 40)
			data->speed_r= BOT_SPEED_FAST;
		else data->speed_r= BOT_SPEED_MAX;

		// Richtung	
		if (diff_r>0) {		// Wenn uebersteurt,
			data->speed_r= -target_speed_r;	//Richtung umkehren
		}

		// Wenn neue Richtung ungleich alter Richtung
		if (((data->speed_r<0)&& (speed_r>0))|| ( (data->speed_r>0) && (speed_r<0) ) ) 
			mot_goto_r--;		// Nulldurchgang merken
	} else
			data->speed_r = BOT_SPEED_IGNORE;
}

/*!
 * TODO: Diese Funktion ist nur ein Dummy-Beispiel, wie eine Kollisionsvermeidung aussehen
 * koennte. Hier ist ein guter Einstiegspunkt fuer eigene Experimente und Algorithmen!
 * Passt auf, dass keine Kollision mit Hindernissen an der Front des Roboters  
 * geschieht.
 * @param *data der Verhaltensdatensatz
 */ 
void bot_avoid_col(Behaviour_t *data){	
	if (sensDistR < COL_CLOSEST)	/* sehr nah */
		col_zone_r=ZONE_CLOSEST;	/* dann auf jeden Fall CLOSEST Zone */
	else 
	/* sind wir naeher als NEAR und nicht in der inneren Zone gewesen */
	if ((sensDistR < COL_NEAR) && (col_zone_r > ZONE_CLOSEST))
		col_zone_r=ZONE_NEAR;	/* dann auf in die NEAR-Zone */
	else
	/* sind wir naeher als FAR und nicht in der NEAR-Zone gewesen */
	if ((sensDistR < COL_FAR) && (col_zone_r > ZONE_NEAR))
		col_zone_r=ZONE_FAR;	/* dann auf in die FAR-Zone */
	else
	/* wir waren in einer engeren Zone und verlassen sie in Richtung NEAR */
	if (sensDistR < (COL_NEAR * 0.50))
		col_zone_r=ZONE_NEAR;	/* dann auf in die NEAR-Zone */
	else
	if (sensDistR < (COL_FAR * 0.50))
		col_zone_r=ZONE_FAR;	/* dann auf in die NEAR-Zone */
	else
		col_zone_r=ZONE_CLEAR;	/* dann auf in die CLEAR-Zone */
	
	if (sensDistL < COL_CLOSEST)	/* sehr nah */
		col_zone_l=ZONE_CLOSEST;	/* dann auf jeden Fall CLOSEST-Zone */
	else 
	/* sind wir naeher als NEAR und nicht in der inneren Zone gewesen */
	if ((sensDistL < COL_NEAR) && (col_zone_l > ZONE_CLOSEST))
		col_zone_l=ZONE_NEAR;	/* dann auf in die NEAR-Zone */
	else
	/* sind wir naeher als FAR und nicht in der NEAR-Zone gewesen */
	if ((sensDistL < COL_FAR) && (col_zone_l > ZONE_NEAR))
		col_zone_l=ZONE_FAR;	/* dann auf in die FAR-Zone */
	else
	/* wir waren in einer engeren Zone und verlassen sie in Richtung NEAR */
	if (sensDistL < (COL_NEAR * 0.50))
		col_zone_l=ZONE_NEAR;	/* dann auf in die NEAR-Zone */
	else
	if (sensDistL < (COL_FAR * 0.50))
		col_zone_l=ZONE_FAR;	/* dann auf in die NEAR-Zone */
	else
		col_zone_l=ZONE_CLEAR;	/* dann auf in die CLEAR-Zone */
		
	switch (col_zone_l){
		case ZONE_CLOSEST:
			data->faktor_r = BRAKE_CLOSEST;
			break;
		case ZONE_NEAR:
			data->faktor_r =  BRAKE_NEAR;
			break;
		case ZONE_FAR:
			data->faktor_r =  BRAKE_FAR;
			break;
		case ZONE_CLEAR:
			data->faktor_r = 1;
			break;
		default:
			col_zone_l = ZONE_CLEAR;
			break;
	}
		
	switch (col_zone_r){
		case ZONE_CLOSEST:
			data->faktor_l = BRAKE_CLOSEST;
			break;
		case ZONE_NEAR:
			data->faktor_l = BRAKE_NEAR;
			break;
		case ZONE_FAR:
			data->faktor_l = BRAKE_FAR;
			break;
		case ZONE_CLEAR:
			data->faktor_l = 1;
			break;
		default:
			col_zone_r = ZONE_CLEAR;
			break;
	}	
	
/*	if (sensDistR < COL_CLOSEST)	// sehr nah
		speed_l_col=-target_speed_l-BOT_SPEED_NORMAL;	// rueckwaerts fahren
	else if (sensDistR < COL_NEAR)	//  nah
		speed_l_col=-target_speed_l * 0.9;		// langsamer werden
	else if (sensDistR < COL_FAR)	//  fern
		speed_l_col=-target_speed_r * 0.65;		// langsamer werden
    else speed_l_col=0;			// nichts tun

	if (sensDistL < COL_CLOSEST)	// sehr nah
		speed_r_col=-target_speed_r-BOT_SPEED_NORMAL;	// rueckwaerts fahren
	else if (sensDistL < COL_NEAR)	//  nah
		speed_r_col=-target_speed_r  * 0.9;
	else if (sensDistL < COL_FAR)	//  fern
		speed_r_col=-target_speed_r  * 0.65;
	     else speed_r_col=0;
*/
	if ((col_zone_r == ZONE_CLOSEST)&&(col_zone_l == ZONE_CLOSEST)){
		data->speed_l = -target_speed_l + BOT_SPEED_MAX;
		data->speed_r = -target_speed_r - BOT_SPEED_MAX;
	}
	else {
		data->speed_l = BOT_SPEED_IGNORE;
		data->speed_r = BOT_SPEED_IGNORE;
	}
}

/*!
 * Verhindert, dass der Bot in Graeben faellt
 * @param *data der Verhaltensdatensatz
 */
void bot_avoid_border(Behaviour_t *data){
	if (sensBorderL > BORDER_DANGEROUS)
		data->speed_l=-BOT_SPEED_NORMAL;
	else
		data->speed_l=BOT_SPEED_IGNORE;
	
	if (sensBorderR > BORDER_DANGEROUS)
		data->speed_r=-BOT_SPEED_NORMAL;
	else 
		data->speed_r=BOT_SPEED_IGNORE;
}

/*! 
 * Zentrale Verhaltens-Routine, 
 * wird regelmaessig aufgerufen. 
 * Dies ist der richtige Platz fuer eigene Routinen, 
 * um den Bot zu steuern
 */
void bot_behave(void){	
	Behaviour_t *job = behaviour;		// Zeiger auf ein Verhalten
	char skip = 0;						// Abbruchvariable für die Schleife
	char addNext = 0;					// Sollen Verhalten kummuliert werden?

	float faktor_l = 1.0;				// Puffer für modifkatoren
	float faktor_r = 1.0;				// Puffer für modifkatoren
	int16 speedLeft = BOT_SPEED_IGNORE;				// Puffer für Geschwindigkeiten
	int16 speedRight = BOT_SPEED_IGNORE;				// Puffer für Geschwindigkeiten
	
	#ifdef RC5_AVAILABLE
		rc5_control();					// Abfrage der IR-Fernbedienung
	#endif

	/* Solange noch Verhalten in der Liste sind
	   Achtung wir werten die Jobs sortiert nach Prioritaet. Wichtige zuerst!!! */
	while ((job != NULL	) && (skip == 0)){
		if (job->active != 0) {
			job->work(job);	/* Verhalten ausfuehren */
			// printf("\nVerhalten mit Prioritaet %d speed_l= %d speed_r= %d faktor_l=%5.4f faktor_r= %5.4f\n",job->priority,job->speed_l,job->speed_r,job->faktor_l,job->faktor_r);

			/* Geschwindigkeit aendern? */
			if ((job->speed_l != BOT_SPEED_IGNORE) || (job->speed_r != BOT_SPEED_IGNORE)){
				if (addNext == 0) {
					speedLeft = job->speed_l * faktor_l;
					speedRight = job->speed_r * faktor_r;
				}
				else {
					speedLeft += job->speed_l * faktor_l;
					speedRight += job->speed_r * faktor_r;
				}
			}
			/* Wenn ein Verhalten Werte modifizieren will, dann sammeln */
			if ((job->faktor_l != 1.0) || (job->faktor_r != 1.0)){
				faktor_l *= job->faktor_l;
				faktor_r *= job->faktor_r;
			}
		}
		
		/* Hat der naechste Job die gleiche Prioritaet wie der aktuelle, dann diesen additiv behandeln */
		if (NULL != job->next) {
			if (job->priority == job->next->priority)
				addNext = 1;
			else
				addNext = 0;
		} else 
			addNext = 0;
		
		/* Wenn ein Verhalten Werte direkt setzen will, nicht weitermachen, ausser der
		   naechste Job hat die gleiche Prioritaet */
		if ((addNext == 0) && ((speedLeft != BOT_SPEED_IGNORE) || (speedRight!= BOT_SPEED_IGNORE))) {
			motor_set(speedLeft * faktor_l, speedRight * faktor_r);
			skip = 1;
		}
		
		/* printf("Add next = %d Skip = %d speedLeft=%d speedRight=%d\n", addNext, skip, speedLeft, speedRight); */
		
		job = job->next;	/* Und weiter in der Liste */
	}
}

/*! 
 * Erzeugt ein neues Verhalten 
 * @param priority Die Prioritaet
 * @param *work Den Namen der Funktion, die sich drum kuemmert
 */
Behaviour_t *new_behaviour(char priority, void (*work) (struct _Behaviour_t *data)){
	Behaviour_t *newbehaviour = (Behaviour_t *) malloc(sizeof(Behaviour_t)); 
	
	if (newbehaviour == NULL) 
		return NULL;
	
	newbehaviour->priority = priority;
	newbehaviour->speed_l=BOT_SPEED_IGNORE;
	newbehaviour->speed_r=BOT_SPEED_IGNORE;
	newbehaviour->faktor_l=1.0;
	newbehaviour->faktor_r=1.0;
	newbehaviour->active=1;
	newbehaviour->ignore=0;
	newbehaviour->next= NULL;
	newbehaviour->work=work;
	
	return newbehaviour;
}

/*!
 * Fuegt ein Verhalten der Verhaltenliste anhand der Prioritaet ein.
 * @param list Die Speicherstelle an der die globale Verhaltensliste anfaengt
 * @param behave Einzufuegendes Verhalten
 */
static void insert_behaviour_to_list(Behaviour_t **list, Behaviour_t *behave){
	Behaviour_t	*ptr	= *list;
	Behaviour_t *temp	= NULL;
	
	/* Kein Eintrag dabei? */
	if (behave == NULL)
		return;
	
	/* Erster Eintrag in der Liste? */
	if (ptr == NULL){
		ptr = behave;
		*list = ptr;
	} else {
		/* Gleich mit erstem Eintrag tauschen? */
		if (ptr->priority < behave->priority) {
			behave->next = ptr;
			ptr = behave;
			*list = ptr;
		} else {
			/* Mit dem naechsten Eintrag vergleichen */
			while(NULL != ptr->next) {
				if (ptr->next->priority < behave->priority)	
					break;
				
				/* Naechster Eintrag */
				ptr = ptr->next;
			}
			
			temp = ptr->next;
			ptr->next = behave;
			behave->next = temp;
		}
	}
}

/*!
 * Initialisert das ganze Verhalten
 */
void bot_behave_init(void){

	/* Einfache Verhaltensroutine, die alles andere uebersteuert */
	insert_behaviour_to_list(&behaviour, new_behaviour(210, bot_simple));
	insert_behaviour_to_list(&behaviour, new_behaviour(200, bot_avoid_border));

	insert_behaviour_to_list(&behaviour, new_behaviour(100, bot_avoid_col));
	insert_behaviour_to_list(&behaviour, new_behaviour(50, bot_goto_system));
	insert_behaviour_to_list(&behaviour, new_behaviour(0, bot_base));

	#ifdef PC
		#ifdef DISPLAY_AVAILABLE
			/* Annzeigen der geladenen Verhalten  */
				Behaviour_t	*ptr	= behaviour;
	
				display_cursor(1,1);
				sprintf(display_buf,"Verhaltensstack:\n");			
				display_buffer();
				while(ptr != NULL)	{
					sprintf(display_buf,"Prioritaet: %d.\n", ptr->priority);
					display_buffer();
					ptr = ptr->next;
				}
		#endif
	#endif

	return;
}


/*!
 * Drehe die Raeder um die gegebene Zahl an Encoder-Schritten weiter
 * @param left Schritte links
 * @param right Schritte rechts
 */
void bot_goto(int16 left, int16 right){
	// Zielwerte speichern
	mot_l_goto=left; 
	mot_r_goto=right;

	// Encoder zuruecksetzen
	sensEncL=0;
	sensEncR=0;
	
	//Goto-System aktivieren
	if (left !=0) mot_goto_l= MOT_GOTO_MAX; 
	else mot_goto_l=0;

	if (right!=0) mot_goto_r=MOT_GOTO_MAX;
	else mot_goto_r=0;
}
