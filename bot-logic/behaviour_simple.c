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


/*! @file 	behaviour_simple.c
 * @brief 	ganz einfache Beispielverhalten
 * Diese Datei sollte der Einstiegspunkt fuer eigene Experimente sein, 
 * den Roboter zu steuern.
 * 
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	03.11.06
*/


#include "bot-logic/bot-logik.h"
#ifdef BEHAVIOUR_SIMPLE_AVAILABLE


/*! 
 * Ein ganz einfaches Verhalten, es hat maximale Prioritaet
 * Hier kann man auf ganz einfache Weise die ersten Schritte wagen. 
 * Wer die Moeglichkeiten des ganzen Verhaltensframeworks ausschoepfen will, kann diese Funktion getrost auskommentieren
 * und findet dann in bot_behave_init() und bot_behave() weitere Hinweise fuer elegante Bot-Programmierung....
 * 
 * Das Verhalten ist per default abgeschaltet. 
 * Damit es läuft, muss man in include/bot-logik/available_behaviours die Kommentarzeichen vor BEHAVIOUR_SIMPLE_AVAILABLE entfernen. 
 * Achtung, da bot_simple_behaviour() maximale Prioritaet hat, kommt es vor den anderen Regeln, wie dem Schutz vor Abgruenden, etc. zum Zuge
 * Das sollte am Anfang nicht stoeren, spaeter sollte man jedoch die Prioritaet herabsetzen.
 * 
 * @param *data der Verhaltensdatensatz
 */
void bot_simple_behaviour(Behaviour_t *data){
	static uint8 state=0;
	
	switch (state){
		case 0:
			bot_drive_distance(data ,0 , BOT_SPEED_MAX, 14);
			state++;
			break;
		case 1:
			bot_turn(data , 90);
			state=0;
			break;
		default:
			state=0;
			return_from_behaviour(data);
			break;
	}
}


/*!
 * Rufe das Simple-Verhalten auf 
 * @param caller Der obligatorische Verhaltensdatensatz des Aufrufers
 */
void bot_simple(Behaviour_t * caller, int16 light){
	switch_to_behaviour(caller,bot_simple_behaviour,OVERRIDE);	
}


/*! Uebergabevariable fuer SIMPLE2 */
static int16 simple2_light=0; 

/*! 
 * Ein ganz einfaches Beispiel fuer ein Hilfsverhalten, 
 * das selbst SpeedWishes aussert und 
 * nach getaner Arbeit die aufrufende Funktion wieder aktiviert
 * Zudem prueft es, ob eine Uebergabebedingung erfuellt ist.
 * 
 * Zu diesem Verhalten gehoert die Botenfunktion bot_simple2()
 * 
 * Hier kann man auf ganz einfache Weise die ersten Schritte wagen. 
 * Wer die Moeglichkeiten des ganzen Verhaltensframeworks ausschoepfen will, kann diese Funktion getrost auskommentieren
 * und findet dann in bot_behave_init() und bot_behave() weitere Hinweise fuer elegante Bot-Programmierung....
 * 
 * Das Verhalten ist per default abgeschaltet. 
 * Damit es läuft, muss man in include/bot-logik/available_behaviours die Kommentarzeichen vor BEHAVIOUR_SIMPLE_AVAILABLE entfernen. 
 * Achtung, da bot_simple2_behaviour() maximale Prioritaet hat, kommt es vor den anderen Regeln, wie dem Schutz vor Abgruenden, etc. zum Zuge
 * Das sollte am Anfang nicht stoeren, spaeter sollte man jedoch die Prioritaet herabsetzen.
 * 
 * bot_simple2_behaviour faehrt den Bot solange geradeaus, bis es dunkler als im Uebergabeparameter spezifiziert ist wird
 * 
 * @param *data der Verhaltensdatensatz
 */
void bot_simple2_behaviour(Behaviour_t *data){
	#define STATE_SIMPLE2_INIT 0
	#define STATE_SIMPLE2_WORK 1
	#define STATE_SIMPLE2_DONE 2
	static uint8 state = 0;

	switch	(state) {
		case STATE_SIMPLE2_INIT: 
			// Nichts zu tun
			state=STATE_SIMPLE2_WORK;
			break;
		case STATE_SIMPLE2_WORK: 
			// Fahre ganz schnell
			speedWishLeft = BOT_SPEED_FAST;
			speedWishRight = BOT_SPEED_FAST; 
			if (sensLDRL< simple2_light)	// Beispielbedingung
				// Wenn es dunkler als angegeben wird, dann haben wir unserd Ziel erreicht
				state=STATE_SIMPLE2_DONE;		
			break;
			
		case STATE_SIMPLE2_DONE:		/* Sind wir fertig, dann Kontrolle zurueck an Aufrufer */
			state=STATE_SIMPLE2_INIT;
			return_from_behaviour(data);
			break;
	}
}

/*!
 * Rufe das Simple2-Verhalten auf und uebergebe light
 * @param caller Der obligatorische Verhaltensdatensatz des Aufrufers
 * @param light Uebergabeparameter
 */
void bot_simple2(Behaviour_t * caller, int16 light){
	simple2_light=light;

	// Zielwerte speichern
	switch_to_behaviour(caller,bot_simple2_behaviour,OVERRIDE);	
}
#endif
