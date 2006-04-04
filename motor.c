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

/*! @file 	motor.c
 * @brief 	High-Level-Routinen fuer die Motorsteuerung des c't-Bot
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	15.01.05
*/

#include <stdlib.h>
#include "global.h"
#include "ct-Bot.h"
#include "motor.h"
#include "motor-low.h"
#include "timer.h"
#include "sensor.h"
#include "display.h"

volatile int16 speed_l=0;	/*!< Geschwindigkeit linker Motor */
volatile int16 speed_r=0;	/*!< Geschwindigkeit rechter Motor */

direction_t direction;		/*!< Drehrichtung der Motoren */


#define MoMAX 255   // Maximale PWM
#define MoMIN 0    // Minimal PWM

volatile int16 err_r_old=0;
volatile int16 err_r_old2=0;
volatile int16 err_l_old=0;
volatile int16 err_l_old2=0;
volatile word reg_l_old=BOT_SPEED_NORMAL;
volatile word reg_r_old=BOT_SPEED_NORMAL;
volatile int16 last_left=BOT_SPEED_NORMAL;
volatile int16 last_right=BOT_SPEED_NORMAL;
volatile int16 tmpR=0;
volatile int16 tmpL=0;



/*!
 * Direkter Zugriff auf den Motor
 * @param left	Geschwindigkeit fuer den linken Motor
 * @param right Geschwindigkeit fuer den linken Motor
 * Geschwindigkeit liegt zwischen -255 und +255.
 * 0 bedeutet Stillstand, 255 volle Kraft voraus, -255 volle Kraft zurueck.
 * Sinnvoll ist die Verwendung der Konstanten: BOT_SPEED_XXX, 
 * also z.B. motor_set(BOT_SPEED_LOW,-BOT_SPEED_LOW);
 * fuer eine langsame Drehung
*/
void motor_set(int16 left, int16 right){
	if (left == BOT_SPEED_IGNORE)	
		left=BOT_SPEED_STOP;
		
	if (abs(left) > BOT_SPEED_MAX)	// Nicht schneller fahren als moeglich
		speed_l = BOT_SPEED_MAX;
	else if (left == 0)				// Stop wird nicht veraendert
		speed_l = BOT_SPEED_STOP;
	else if (abs(left) < BOT_SPEED_SLOW)	// Nicht langsamer als die 
		speed_l = BOT_SPEED_SLOW;	// Motoren koennen
	else				// Sonst den Wunsch uebernehmen
		speed_l = abs(left);


	if (right == BOT_SPEED_IGNORE)	
		right=BOT_SPEED_STOP;

	if (abs(right) > BOT_SPEED_MAX)// Nicht schneller fahren als moeglich
		speed_r = BOT_SPEED_MAX;
	else if (abs(right) == 0)	// Stop wird nicht veraendert
		speed_r = BOT_SPEED_STOP;
	else if (abs(right) < BOT_SPEED_SLOW)	// Nicht langsamer als die 
		speed_r = BOT_SPEED_SLOW;	// Motoren koennen
	else				// Sonst den Wunsch uebernehmen
		speed_r = abs(right);
	
	if (left < 0 ){
		speed_l=-speed_l;
		direction.left= DIRECTION_BACKWARD;
	} else  if (left > 0 )
		direction.left= DIRECTION_FORWARD;
	
	if (right < 0 )	{
		speed_r=-speed_r;
		direction.right= DIRECTION_BACKWARD;
	} else if (right > 0 )
		direction.right= DIRECTION_FORWARD;
	#ifdef SPEED_CONTROL_AVAILABLE
		speed_control(speed_l, speed_r);		
	#else
	    bot_motor(speed_l,speed_r);
	#endif    
}

/*!
 * Stellt die Servos
 * Sinnvolle Werte liegen zwischen 8 und 16
 * @param servo Nummer des Servos
 * @param servo Zielwert
 */
void servo_set(uint8 servo, uint8 pos){
	if (pos< SERVO_LEFT)
		pos=SERVO_LEFT;
	if (pos> SERVO_RIGHT)
		pos=SERVO_RIGHT;
		
	bot_servo(servo,pos);
}

/*!
 * Initialisiere den Motorkrams
 */
void motor_init(void){
	speed_l=0;
	speed_r=0;
	motor_low_init();
}

/*
 * @brief 	Drehzahlregelung für die Motoren des c't-Bots
 * @author 	Daniel Bachfeld (dab@heise.de)
 * @date 	03.04.06
 * !!!!!!!!!! parameter nachtragen
*/
void speed_control (int16 left, int16 right){
	uint8 Kp, Kd, Ki;                      // PID-Parameter
	int16 rmp, lmp, err_l, err_r;
	word reg_l,  reg_r;                    //Stellwerte 
	
	//Regler links
	 
	if (left != -BOT_SPEED_NORMAL && right !=  -BOT_SPEED_NORMAL) {  // Regelung bei Abgrund überspringen
      if ((last_left != left) || (clock_motor_control_l >1860)) {
        lmp = abs(sensEncL-tmpL);  // aktuelle Ist-Wert berechnen
		tmpL = sensEncL;          // letzte Anzahl der Encoderpulse merken
        
		if (last_left != left) {  // Bei abruptem Geschwindigkeitswechsel alte Fehler auf Null setzen
        	reg_l_old = left;
        	err_l_old = 0;    
        	err_l_old2 = 0;
        	lmp = 0;              //aktuellen Fehler auf Null setzen
        }
		
		if (left==BOT_SPEED_NORMAL) {     // Zu SPEED_NORMAL gehoeren PID-Parameter
           Kp = 7;                        //
           Ki = 5;                        // PID-Werte für SPEED_NORMAL 
           Kd = 1;                        // Hier sollte man versch. Werten für Leerlauf und Last probieren
        }   
        
        if (left==BOT_SPEED_SLOW) {
          Kp = 3;
          Ki = 2;
          Kd = 0;
        }
        
        if (clock_motor_control_l >1860) {
          err_l =left  - lmp;  // Regelabweichung links
          reg_l = reg_l_old + Kp* (err_l - err_l_old);                // P-Beitrag
          reg_l = reg_l + Ki * (err_l + err_l_old)/2;                 // I-Beitrag
          reg_l = reg_l + Kd * (err_l - 2 * err_l_old + err_l_old2);  // D-Beitrag
          if (reg_l > MoMAX) reg_l = MoMAX;         //berechneten Stellwert auf zulässige Groesse begrenzen
          if (reg_l < MoMIN) reg_l = MoMIN;
          err_l_old2 = err_l_old;       // alte Fehler merken
          err_l_old = err_l;
          reg_l_old = reg_l;            // Stellwerte merken
        }
        clock_motor_control_l =0 ;  
      }     
      //Regler rechts
    
      if ((last_right != right) || (clock_motor_control_r >1860)){
        rmp = abs(sensEncR-tmpR);        
        tmpR = sensEncR;   
       
        if (last_right != right) {
          reg_r_old = right;
          err_r_old = 0;    
          err_r_old2 = 0;
          rmp = 0;
        }
        
        if (right==BOT_SPEED_NORMAL) {
         Kp = 7;
         Ki = 5;
         Kd = 1;
        }
        
        if (right==BOT_SPEED_SLOW) {
          Kp = 3;
          Ki = 2;
          Kd = 0;
        }  
        
        if (clock_motor_control_r >1860) {
          err_r = right - rmp;
          reg_r = reg_r_old + Kp * (err_r - err_r_old);  
          reg_r = reg_r + Ki * (err_r + err_r_old)/2; 
          reg_r = reg_r + Kd * (err_r - 2 *  err_r_old + err_r_old2);
          if (reg_r > MoMAX) reg_r = MoMAX;     
          if (reg_r < MoMIN) reg_r = MoMIN;
          err_r_old2 = err_r_old;
          err_r_old = err_r;
          reg_r_old = reg_r;

        }
        clock_motor_control_r = 0;
      } 
        // Anzeige fuer Debugging
        
     #ifdef DISPLAY_AVAILABLE
        display_cursor(3,1);
	    display_printf("L:%d   R:%d       ", reg_l_old, reg_r_old);
        display_cursor(4,1);
        display_printf("SL:%d  SR:%d            ", left, right);	
     #endif
       last_left = left;              // alte Geschwindigkeit merken
       last_right = right;
       bot_motor(reg_l_old, reg_r_old);       // Motorwerte setzen
	}
	else bot_motor (left, right);  //alle Maschinen zurück, Abgrund voraus
}
