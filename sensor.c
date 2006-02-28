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

/*! @file 	sensor.c  
 * @brief 	Architekturunabhaengiger Teil der Sensorsteuerung
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	15.01.05
*/

#include "ct-Bot.h"

volatile int16 sensLDRL=0;		/*!< Lichtsensor links */
volatile int16 sensLDRR=0;		/*!< Lichtsensor rechts */

volatile int16 sensDistL=1023;		/*!< Distanz linker IR-Sensor in [mm], wenn korrekt umgerechnet wird */
volatile int16 sensDistR=1023;		/*!< Distanz rechter IR-Sensor in [mm], wenn korrekt umgerechnet wird */

volatile int16 sensBorderL=0;	/*!< Abgrundsensor links */
volatile int16 sensBorderR=0;	/*!< Abgrundsensor rechts */

volatile int16 sensLineL=0;	/*!< Lininensensor links */
volatile int16 sensLineR=0;	/*!< Lininensensor rechts */

volatile char sensTrans=0;		/*!< Sensor Ueberwachung Transportfach */

volatile char sensDoor=0;		/*!< Sensor Ueberwachung Klappe */

volatile char sensError=0;		/*!< Ueberwachung Motor oder Batteriefehler */

volatile char sensMouseDX;		/*!< Maussensor Delta X, positive Werte zeigen querab der Fahrtrichtung nach rechts */
volatile char sensMouseDY;		/*!< Maussensor Delta Y, positive Werte zeigen in Fahrtrichtung */

volatile int sensMouseX;		/*!< Mausposition X, positive Werte zeigen querab der Fahrtrichtung nach rechts */
volatile int sensMouseY;		/*!< Mausposition Y, positive Werte zeigen in Fahrtrichtung  */

volatile int sensEncL=0;	/*!< Encoder linker Motor */
volatile int sensEncR=0;	/*!< Encoder rechter Motor */

volatile char v_left;			/*!< Abrollgeschwindigkeit des linken Rades in [mm/s] [-128 bis 127] relaisitisch [-50 bis 50] */
volatile char v_right;			/*!< Abrollgeschwindigkeit des linken Rades in [mm/s] [-128 bis 127] relaisitisch [-50 bis 50] */
