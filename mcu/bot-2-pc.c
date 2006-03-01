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

/*! @file 	bot-2-pc.c 
 * @brief 	Verbindung zwischen c't-Bot und PC
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	28.2.06
*/

#include "ct-Bot.h"
#include "command.h"
#include "uart.h"
#include "bot-2-pc.h"
#include "sensor.h"
#include "motor.h"
#include "led.h"

#ifdef MCU
#ifdef UART_AVAILABLE

/*! 
 * Diese Funktion nimmt die Daten vom PC entgegen
 * und wertet sie aus. dazu nutzt er die Funktion command_evaluate()
 */
void bot_2_pc_listen(void){
		if (uart_data_available() >= sizeof(command_t)){
			if (command_read() ==0)
				command_evaluate();
			else {		
				// TODO Fehlerbehandlung
			}
		}		
}

/*! 
 * Diese Funktion informiert den PC ueber alle Sensor und Aktuator-Werte
 */
void bot_2_pc_inform(void){
	int16 null =0;
	command_write(CMD_AKT_MOT, SUB_CMD_NORM ,(int16*)&speed_l,(int16*)&speed_r);	
	command_write(CMD_AKT_LED, SUB_CMD_NORM ,(int16*)&led,(int16*)&led);
	
	command_write(CMD_SENS_IR, SUB_CMD_NORM ,(int16*)&sensDistL,(int16*)&sensDistR);
	command_write(CMD_SENS_ENC, SUB_CMD_NORM ,(int16*)&sensEncL,(int16*)&sensEncR);
	command_write(CMD_SENS_BORDER, SUB_CMD_NORM ,(int16*)&sensBorderL,(int16*)&sensBorderR);
	command_write(CMD_SENS_LINE, SUB_CMD_NORM ,(int16*)&sensLineL,(int16*)&sensLineR);

	command_write(CMD_SENS_LDR, SUB_CMD_NORM ,(int16*)&sensLDRL,(int16*)&sensLDRR);
	
	command_write(CMD_SENS_TRANS, SUB_CMD_NORM ,(int16*)&sensTrans,&null);
	command_write(CMD_SENS_DOOR, SUB_CMD_NORM ,(int16*)&sensDoor,&null);
	command_write(CMD_SENS_MOUSE, SUB_CMD_NORM ,(int16*)&sensMouseDX,(int16*)&sensMouseDY);
	command_write(CMD_SENS_ERROR, SUB_CMD_NORM ,(int16*)&sensError,&null);
//	command_write(CMD_SENS_RC5, SUB_CMD_NORM ,(int16*)&RC5_Code,&null);
}

#endif
#endif

