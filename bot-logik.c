/*! @file 	bot-logik.c
 * @brief 	High-Level Routinen für die Steuerung des c't-Bots
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	01.12.05
*/

#include "bot-mot.h"
#include "bot-sens.h"
#include "rc5.h"
#include <stdlib.h>

#define	BORDER_DANGEROUS	500		///< Wert, ab dem wir sicher sind, dass es eine Kante ist


#define	COL_CLOSEST			50		///< Abstand in mm den wir als zu nah betrachten
#define	COL_NEAR			100		///< Nahbereich
#define	COL_FAR				200		///< Fernbereich

#define MOT_GOTO_MAX  3 		///< Richtungsänderungen, bis goto erreicht sein muss

volatile int16 mot_l_goto=0;	///< Speichert wieweit der linke Motor drehen soll
volatile int16 mot_r_goto=0;	///< Speichert wieweit der rechte Motor drehen soll

volatile int16 mot_goto_l=0;	///< Muss der linke motor noch drehen? 
volatile int16 mot_goto_r=0;	///< Muss der rechte motor noch drehen? 

volatile int16 speed_l_col=0;	///< Kollisionsschutz links
volatile int16 speed_r_col=0;	///< Kollisionsschutz links

/*!
 * Drehe die Räder um die gegebene Zahl an Encoder-Schritten weiter
 * @param left Schritte links
 * @param right Schritte rechts
 */
void bot_goto(int left, int right){
	// Zielwerte Speichern
	mot_l_goto=left; 
	mot_r_goto=right;

	// Encoder Zur�cksetzen
	encoderL=0;
	encoderR=0;
	
	//Goto-System aktivieren
	if (left !=0) mot_goto_l= MOT_GOTO_MAX; 
	else mot_goto_l=0;

	if (right!=0) mot_goto_r=MOT_GOTO_MAX;
	else mot_goto_r=0;
}

/*!
 * Kümmert sich intern um dsie ausführung der goto-Kommandos
 * @see bot_goto()
 */
void bot_goto_system(void){
	int diff_l = encoderL - mot_l_goto;	// Restdistanz links
	int diff_r = encoderR - mot_r_goto;	// Restdistanz rechts	
	
	// Motor L hat noch keine MOT_GOTO_MAX Nulldurchg�nge gehabt
	if (mot_goto_l >0){
		if (abs(diff_l) <= 2){			// 2 Encoderst�nde genauigkeit reicht
			speed_l = BOT_SPEED_STOP;	//Stop
			mot_goto_l--;			// wie Nulldurchgang behandeln
		}else if (abs(diff_l) < 4)
			speed_l= BOT_SPEED_SLOW;
		else if (abs(diff_l) < 10)
			speed_l= BOT_SPEED_NORMAL;
		else if (abs(diff_l) < 40)
			speed_l= BOT_SPEED_FAST;
		else speed_l= BOT_SPEED_MAX;

		// Richtung	
		if (diff_l>0) {		// Wenn �bersteurt
			speed_l= -speed_l;	//Richtung umkehren
		}
		
		// Wenn neue Richtung ungleich alter Richtung
		if (((speed_l<0)&& (mot_l_dir==1))|| ( (speed_l>0) && (mot_l_dir==0) ) ) 
			mot_goto_l--;		// Nulldurchgang merken
	}

	// Motor R hat noch keine MOT_GOTO_MAX Nulldurchg�nge gehabt
	if (mot_goto_r >0){
		if (abs(diff_r) <= 2){			// 2 Encoderst�nde genauigkeit reicht
			speed_r = BOT_SPEED_STOP;	//Stop
			mot_goto_r--;			// wie Nulldurchgang behandeln
		}else if (abs(diff_r) < 4)
			speed_r= BOT_SPEED_SLOW;
		else if (abs(diff_r) < 10)
			speed_r= BOT_SPEED_NORMAL;
		else if (abs(diff_r) < 40)
			speed_r= BOT_SPEED_FAST;
		else speed_r= BOT_SPEED_MAX;

		// Richtung	
		if (diff_r>0) {		// Wenn �bersteurt
			speed_r= -speed_r;	//Richtung umkehren
		}

		// Wenn neue Richtung ungleich alter Richtung
		if (((speed_r<0)&& (mot_r_dir==1))|| ( (speed_r>0) && (mot_r_dir==0) ) ) 
			mot_goto_r--;		// Nulldurchgang merken
	}
}

/*!
 * Passt auf, dass keine Kollision mit Hindernissen an der Front geschieht.
 */ 
void bot_avoid_col(void){
	if (sensDistL < COL_CLOSEST)	// sehr nah
		speed_l_col=-speed_l-BOT_SPEED_NORMAL;	// rückwärts fahren
	else if (sensDistL < COL_NEAR)	//  nah
		speed_l_col=-speed_l/2;
	else if (sensDistL < COL_FAR)	//  fern
		speed_l_col=-speed_r/3;
    else speed_l_col=0;
	
	if (sensDistR < COL_CLOSEST)	// sehr nah
		speed_r_col=-speed_r-BOT_SPEED_NORMAL;	// rückwärts fahren
	else if (sensDistR < COL_NEAR)	//  nah
		speed_r_col=-speed_r/2;
	else if (sensDistR < COL_FAR)	//  fern
		speed_r_col=-speed_r/3;
	     else speed_r_col=0;
}


/*!
 * Verhindert, dass der Bot in Gräben fällt
 */
void bot_avoid_border(){
	int16 gotoL=0;		
	int16 gotoR=0;
	if (sensBorderL > BORDER_DANGEROUS)
		gotoL=-20;
	if (sensBorderR > BORDER_DANGEROUS)
		gotoR=-20;		
	
	if ((gotoL>0)||(gotoR>0)){
		bot_goto(gotoL,gotoR);
		speed_l_col=0;
		speed_r_col=0;
	}
}

/*! 
 * Zentrale Verhaltens Routine 
 * Wird regelmässig aufgerufen. 
 * Dies ist der richtige Platz für eigene Routinen, um den Bot zu steuern
 */
void bot_behave(void){	

	rc5_control();

	bot_avoid_col();		// changes speed_l_col, speed_r_col

//	bot_avoid_border();		// changes goto-system and speed_l_col, speed_r_col
	
	bot_goto_system();		//changes speed_r, speed_l
	
	motor_set(speed_l+speed_l_col,speed_r+speed_r_col);	
}
