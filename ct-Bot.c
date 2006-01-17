/*! @file 	ct-Bot.c
 * @brief 	Demo-Hauptprogramm
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
*/

#include "ct-Bot.h"

#ifdef MCU
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <avr/signal.h>
#endif
	
#ifdef PC
	#include "bot-2-sim.h"
	#include "tcp.h"
	#include "tcp-server.h"
	#include <pthread.h>
#endif


#include <string.h>
#include <stdio.h>

#include "global.h"
#include "display.h"
#include "led.h"
#include "ena.h"
#include "shift.h"
#include "delay.h"
#include "uart.h"
#include "adc.h"
#include "timer.h"
#include "sensor.h"


#include "motor.h"
#include "sensor-low.h"
#include "bot-logik.h"
#include "mouse.h"

#include "command.h"
#include "ir-rc5.h"
#include "rc5.h"

/*!
 * Der Mikrocontroller und der PC-Simulator brauchen ein paar Einstellungen, 
 * bevor wir loslegen könen.
 */
void init(void){
	
	#ifdef MCU
		PORTA=0; DDRA=0;		//Alles Eingang alles Null
		PORTB=0; DDRB=0;
		PORTC=0; DDRC=0;
		PORTD=0; DDRD=0;
	#endif

	#ifdef DISPLAY_AVAILABLE
		display_init();
		display_update=1;
	#endif

	#ifdef LED_AVAILABLE
		LED_init();
	#endif

	#ifdef PC
		bot_2_sim_init();
	#endif

	motor_init();
	bot_sens_init();
	
	#ifdef MCU
		#ifdef RC5_AVAILABLE
			ir_init();
		#endif
	#endif
	
	

/*	#ifdef UART_AVAILABLE	
		uart_init();
	#endif
*/			
	#ifdef MAUS_AVAILABLE
		maus_sens_init();
	#endif

}

#ifdef DISPLAY_AVAILABLE

/*!
 * Zeige ein paar Infos an
 */
	void display(void){
		if (display_update >0){
			display_cursor(1,1);
			sprintf(display_buf,"P=%03X %03X D=%03X %03X ",sensLDRL,sensLDRR,sensDistL,sensDistR);
			display_buffer();

			display_cursor(2,1);
			sprintf(display_buf,"B=%03X %03X L=%3X %03X ",sensBorderL,sensBorderR,sensLineL,sensLineR);
			display_buffer();

			display_cursor(3,1);
			sprintf(display_buf,"R=%d %d F=%d K=%d T=%d ",sensEncL,sensEncR,sensError,sensDoor,sensTrans);
			display_buffer();
			
			display_cursor(4,1);
			sprintf(display_buf,"I=%04X M=%02X %02X",RC5_Code,sensMouseDX,sensMouseDY);
			display_buffer();				
		}
	}
#endif

#ifdef MCU
/*! 
 * Hauptprogramm des Bots. Diese Schleife kümmert sich um seine Intelligenz
 */
	int main (void){
#endif

#ifdef PC
/*! 
 * Hauptprogramm des Bots. Diese Schleife kümmert sich um seine Intelligenz
 */
 	int main (int argc, char *argv[]){

    if (argc ==2 )    // Test for correct number of arguments
    {
    	printf("ARGV[0]= %s\n",argv[1]);
       tcp_server_init();
       tcp_server_run();
    } else {
    	printf("c't-Bot\n");
    }
    
    
#endif
	init();		
	
	#ifdef WELCOME_AVAILABLE
		display_cursor(1,1);
		display_string("c't-Roboter");
		LED_set(0x00);
	#endif
		
	/// Hauptschleife des Bots
	for(;;){
		#ifdef MCU
			bot_sens_isr();
		#endif
						
		bot_behave();
		
		#ifdef DISPLAY_AVAILABLE
			display();
		#endif
		
		#ifdef PC
			wait_for_time(100000);
		#endif
		
	}
	
	/// Falls wir das je erreichen sollten ;-)
	return 1;	
}
