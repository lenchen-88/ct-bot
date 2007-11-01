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

/*! 
 * @file 	sensor-low.c  
 * @brief 	Low-Level Routinen fuer die Sensor Steuerung des c't-Bots
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	01.12.05
 */

#ifdef MCU

#include <avr/io.h>
#include "adc.h" 
#include "global.h"

#include "ena.h"
#include "sensor.h"
#include "mouse.h"
#include "motor.h"
#include "timer.h"
#include "sensor_correction.h"
#include "bot-local.h"
#include "bot-logic/available_behaviours.h"
#include "bot-logic/behaviour_servo.h"
#include "display.h"
#include "mmc.h"
#include "mini-fat.h"
#include "led.h"
#include "sensor-low.h"
#include "i2c.h"
#include <string.h>

// ADC-PINS
#define SENS_ABST_L	0			/*!< ADC-PIN Abstandssensor Links */
#define SENS_ABST_R	1			/*!< ADC-PIN Abstandssensor Rechts */
#define SENS_M_L		2		/*!< ADC-PIN Liniensensor Links */
#define SENS_M_R		3		/*!< ADC-PIN Liniensensor Rechts */
#define SENS_LDR_L		4		/*!< ADC-PIN Lichtsensor Links */
#define SENS_LDR_R		5		/*!< ADC-PIN Lichtsensor Rechts */
#define SENS_KANTE_L	6		/*!< ADC-PIN Kantensensor Links */
#define SENS_KANTE_R	7		/*!< ADC-PIN Kantensensor Rechts */

// Sonstige Sensoren
#define SENS_DOOR_PINR 		PIND	/*!< Port an dem der Klappensensor hängt */
#define SENS_DOOR_DDR 		DDRD	/*!< DDR für den Klappensensor */
#define SENS_DOOR			6		/*!< Pin  an dem der Klappensensor hängt */

#ifdef SPI_AVAILABLE
#define SENS_ENCL_PINR		PINC	/*!< Port an dem der linke Encoder hängt */
#define SENS_ENCL_DDR		DDRC	/*!< DDR für den linken Encoder  */
#define SENS_ENCL			5		/*!< Pin an dem der linke Encoder hängt */
#else
#define SENS_ENCL_PINR		PINB	/*!< Port an dem der linke Encoder hängt */
#define SENS_ENCL_DDR		DDRB	/*!< DDR für den linken Encoder  */
#define SENS_ENCL			4		/*!< Pin an dem der linke Encoder hängt */
#endif

#define SENS_ENCR_PINR		PIND	/*!< Port an dem der rechte Encoder hängt */
#define SENS_ENCR_DDR		DDRD	/*!< DDR für den rechten Encoder  */
#define SENS_ENCR			3		/*!< Pin an dem der rechte Encoder hängt */

#define SENS_ERROR_PINR		PINB	/*!< Port an dem die Fehlerüberwachung hängt */
#define SENS_ERROR_DDR		DDRB	/*!< DDR für die Fehlerüberwachung */
#define SENS_ERROR			2		/*!< Pin an dem die Fehlerüberwachung hängt */

#define SENS_TRANS_PINR		PINB	/*!< Port an dem die Transportfachueberwachung haengt */
#define SENS_TRANS_PORT		PORTB	/*!< Port an dem die Transportfachueberwachung haengt */
#define SENS_TRANS_DDR		DDRB	/*!< DDR für die Transportfachueberwachung */
#define SENS_TRANS			0		/*!< Pin an dem die Transportfachueberwachung haengt */

#define ENC_L ((SENS_ENCL_PINR >> SENS_ENCL) & 0x01)	/*!< Abkuerzung zum Zugriff auf Encoder */
#define ENC_R ((SENS_ENCR_PINR >> SENS_ENCR) & 0x01)	/*!< Abkuerzung zum Zugriff auf Encoder */

#define ENC_ENTPRELL	12		/*!< Nur wenn der Encoder ein paar mal den gleichen wert gibt uebernehmen */

#ifdef SPEED_CONTROL_AVAILABLE
	uint16 encTimeL[8] = {0};	/*!< Timestamps linker Encoder */
	uint16 encTimeR[8] = {0};	/*!< Timestamps rechter Encoder */
	uint8 i_encTimeL = 0;		/*!< Array-Index auf letzten Timestampeintrag links */
	uint8 i_encTimeR = 0;		/*!< Array-Index auf letzten Timestampeintrag rechts */
	uint8 timeCorrectL = 0;
	uint8 timeCorrectR = 0; 
#endif // SPEED_CONTROL_AVAILABLE

/* Some Debug-Loggings */
#ifdef SPEED_LOG_AVAILABLE	
	volatile slog_t slog_data[2][25] = {{{0}}, {{0}}};	/*!< Speed-Log Daten */
	volatile uint8 slog_i[2] = {0,0};					/*!< Array-Index */
	uint32 slog_sector = 0;								/*!< Sektor auf der MMC fuer die Daten */
	volatile uint8 slog_count[2] = {0,0};				/*!< Anzahl Loggings seit letztem Rueckschreiben */
#endif // SPEED_LOG_AVAILABLE

/*!
 * Initialisiere alle Sensoren
 */
void bot_sens_init(void){	
	ENA_init();
	adc_init(0xFF);		// Alle ADC-Ports aktivieren
	
	ENA_set(ENA_RADLED | ENA_ABSTAND);		// Alle Sensoren bis auf Radencoder & Abstandssensoren deaktivieren
	
	SENS_DOOR_DDR	&= ~(1<<SENS_DOOR);		// Input
	
	SENS_ERROR_DDR	&= ~(1<<SENS_ERROR);	// Input

	SENS_TRANS_DDR	&= ~(1<<SENS_TRANS);	// Input	
	SENS_TRANS_PORT	|=  (1<<SENS_TRANS);	// Pullup an
	
	SENS_ENCL_DDR	&= ~(1<<SENS_ENCL);		// Input	
	SENS_ENCR_DDR	&= ~(1<<SENS_ENCR);		// Input	

	timer_2_init();
}


/*!
 * Alle Sensoren aktualisieren
 * Derzeit pollt diese Routine alle Sensoren. Insbesondere bei den 
 * analogen dauert das eine Weile. Daher kann man hier einiges
 * an Performance gewinnen, wenn man die Routine aufspaltet und
 * zumindest die analogen Sensoren per Interrupt bearbeitet,
 * denn im Moment blockiert adc_read so lange, bis ein Sensorwert ausgelesen ist.
 * Die digitalen Sensoren liefern ihre Werte dagegen unmittelbar
 * Aber Achtung es lohnt auch nicht, immer alles so schnell als moeglich
 * zu aktualiseren, der Bot braucht auch Zeit zum nachdenken ueber Verhalten
 */
void bot_sens_isr(void) {
	ENA_on(ENA_KANTLED|ENA_MAUS|ENA_SCHRANKE|ENA_KLAPPLED);	// Die Distanzsensoren sind im Normalfall an, da sie 50 ms zum booten brauchen	

#ifdef CMPS03_AVAILABLE
	cmps03_get_bearing(&sensCmps03);
#endif
	
	/* aktualisiere Distanz-Sensoren, interrupt-driven I/O */
	static uint8 measure_count = 0;
	static int16 distLeft[4];
	static int16 distRight[4];
	static uint16 old_dist;		// Zeit der letzten Messung der Distanzsensoren

	/* Auswertung der Distanzsensoren alle 50 ms */
	uint16 dist_ticks = TIMER_GET_TICKCOUNT_16;
	if ((uint16)(dist_ticks-old_dist) > MS_TO_TICKS(50)){	
		adc_read_int(SENS_ABST_L, &distLeft[measure_count]);
		#ifdef BEHAVIOUR_SERVO_AVAILABLE
			if ((servo_active & SERVO1) == 0)	// Wenn die Transportfachklappe bewegt wird, stimmt der Messwert des rechten Sensor nicht
		#endif
				adc_read_int(SENS_ABST_R, &distRight[measure_count]);
				
		measure_count++;
		measure_count &= 0x3;	// Z/4Z
	}

	/* die anderen analogen Sensoren, auch int-driven I/O */
	adc_read_int(SENS_M_L, &sensLineL);
	adc_read_int(SENS_M_R, &sensLineR);
	
	adc_read_int(SENS_LDR_L, &sensLDRL);
	adc_read_int(SENS_LDR_R, &sensLDRR);

	adc_read_int(SENS_KANTE_L, &sensBorderL);
	adc_read_int(SENS_KANTE_R, &sensBorderR);
			
	#ifdef MAUS_AVAILABLE
	 	// Aktualisiere die Position des Maussensors 
		sensMouseDX = maus_sens_read(MOUSE_DELTA_X_REG);	
		sensMouseDY = maus_sens_read(MOUSE_DELTA_Y_REG);
	#endif
		
	/* alle digitalen Sensoren */
	sensDoor = (SENS_DOOR_PINR >> SENS_DOOR) & 0x01;
	sensTrans = (SENS_TRANS_PINR >> SENS_TRANS) & 0x01;
	sensError = (SENS_ERROR_PINR >> SENS_ERROR) & 0x01;		
	
	/* Aufruf der Motorregler, falls Stillstand */
	#ifdef SPEED_CONTROL_AVAILABLE 
		/* us-Ticks sichern */
		register uint16 pid_ticks = TIMER_GET_TICKCOUNT_16;	// [178 us]
		register uint8 i_time;
		register uint8* p_time;
		/* Index auf Encodertimestamps zwischenspeichern */
		i_time = i_encTimeL;
		p_time = (uint8*)encTimeL;	
		/* Bei Stillstand Regleraufruf links nach PID_TIME ms */
		if (pid_ticks-*(uint16*)(p_time+i_time) > PID_TIME*50/TIMER_STEPS*20){
			/* Timestamp links verschieben / speichern */
			i_time = (i_time + sizeof(encTimeL[0])) & 0xf;	// encTime ist Z/8Z und jeder Eintrag hat 2 Byte => 0xf
			*(uint16*)(p_time + i_time) = pid_ticks;
			i_encTimeL = i_time;
			/* Regleraufruf */
			speed_control(0,  (int16*)&motor_left, (uint16*)encTimeL, i_encTimeL, 0);
			timeCorrectL = 1;
		}
		/* Bei Stillstand Regleraufruf rechts nach PID_TIME ms */
		i_time = i_encTimeR;
		p_time = (uint8*)encTimeR;
		if (pid_ticks-*(uint16*)(p_time+i_time) > PID_TIME*50/TIMER_STEPS*20) {
			/* Timestamp rechts verschieben / speichern */ 
			i_time = (i_time + sizeof(encTimeR[0])) & 0xf;	// encTime ist Z/8Z und jeder Eintrag hat 2 Byte => 0xf
			*(uint16*)(p_time + i_time) = pid_ticks;
			i_encTimeR = i_time;
			/* Regleraufruf rechts */
			speed_control(1, (int16*)&motor_right, (uint16*)encTimeR, i_encTimeR, 0);
			timeCorrectR = 1;
		}
		
		#ifdef SPEED_LOG_AVAILABLE
			/* Speed-Log-Daten auf MMC schreiben, falls Puffer voll */
			if (slog_sector == 0){
				/* init-stuff here */
				slog_sector = mini_fat_find_block("slog", __builtin_alloca(512));
				//slog_start_sector = slog_sector;
			}
			if (slog_count[0] > 20 || slog_count[1] > 20){	// etwas Luft lassen, denn die Daten kommen per ISR
				uint8 i;
				for (i=slog_count[0]+1; i<25; i++){
					memset((uint8*)&slog_data[0][i],0,sizeof(slog_t));	// q&d
				}
				for (i=slog_count[1]+1; i<25; i++){
					memset((uint8*)&slog_data[1][i],0,sizeof(slog_t));	// q&d
				}
				mmc_write_sector(slog_sector++, (uint8*)slog_data, 0);	// swap-out
				/* Index-Reset */
				slog_i[0] = 0;
				slog_count[0] = 0;
				slog_i[1] = 0;
				slog_count[1] = 0;				
			}
		#endif // SPEED_LOG_AVAILABLE
	#endif // SPEED_CONTROL_AVAILABLE

	sensor_update();	// Weiterverarbeitung der rohen Sensordaten			
	
	if ((uint16)(dist_ticks-old_dist) > MS_TO_TICKS(50)) {
		old_dist = dist_ticks;	// Zeit fuer naechste Messung merken
		// dieser Block braucht insgesamt ca. 80 us (MCU)
		/* Dist-Sensor links */
		while (adc_get_active_channel() < 1) {}
		uint16 voltL = distLeft[0]+distLeft[1]+distLeft[2]+distLeft[3];
		(*sensor_update_distance)(&sensDistL, &sensDistLToggle, sensDistDataL, voltL);
		#ifdef TEST_AVAILABLE_ANALOG
			sensDistL = voltL >> 2;
		#endif	
		/* Dist-Sensor rechts */
		while (adc_get_active_channel() < 2) {}
		uint16 voltR = distRight[0]+distRight[1]+distRight[2]+distRight[3];
		(*sensor_update_distance)(&sensDistR, &sensDistRToggle, sensDistDataR, voltR);
		#ifdef TEST_AVAILABLE_ANALOG
			sensDistR = voltR >> 2;
		#endif			
	}
	
#ifdef CMPS03_AVAILABLE
	cmps03_finish(&sensCmps03);
	heading = (float)sensCmps03.bearing / 10.0;
#endif
	
	/* alle anderen analogen Sensoren */	
	while (adc_get_active_channel() != 255) {}	// restliche Zeit mit busy-waiting verbrauchen
	// in den Testmodi bleibt imemr alles an.
	#ifndef TEST_AVAILABLE_ANALOG 
	  #ifndef TEST_AVAILABLE_DIGITAL 
	     #ifndef TEST_AVAILABLE_MOTOR
	  		ENA_off(ENA_KANTLED|ENA_MAUS|ENA_SCHRANKE|ENA_KLAPPLED);	// Kanten (ENA_KANTLED), Liniensensoren (ENA_MAUS), Transportfach-LED und Klappensensor aus
	  	  #endif
	  #endif
	#endif
		
	/* LEDs updaten */
	#ifdef LED_AVAILABLE
	#ifndef TEST_AVAILABLE
		if (sensTrans != 0) LED_on(LED_GELB);
		else LED_off(LED_GELB);
		if (sensError != 0) LED_on(LED_ORANGE);
		else LED_off(LED_ORANGE);

		if (sensDistL < 500) LED_on(LED_LINKS);
		else LED_off(LED_LINKS);
		if (sensDistR < 500) LED_on(LED_RECHTS);
		else LED_off(LED_RECHTS);

		/* Sollen die LEDs mit den Rohdaten der Sensoren arbeiten, 
		 * kommentiert man die folgenden Zeilen ein (und die Obigen aus) */
		 
		//if (voltL > 80) LED_on(LED_LINKS);
		//else LED_off(LED_LINKS);
		//if (voltR > 80) LED_on(LED_RECHTS);
		//else LED_off(LED_RECHTS);
	#endif	// TEST_AVAILABLE
	#endif	// LED_AVAILABLE	
}

/*!
 * Kuemmert sich um die Radencoder
 * Das muss schneller gehen als die anderen Sensoren,
 * daher Update per Timer-Interrupt und nicht per Polling
 */
void bot_encoder_isr(void){
	static uint8 enc_l=0;		/*!< Puffer fuer die letzten Encoder-Staende */
	static uint8 enc_r=0;		/*!< Puffer fuer die letzten Encoder-Staende */
	static uint8 enc_l_cnt=0;	/*!< Entprell-Counter fuer L-Encoder */
	static uint8 enc_r_cnt=0;	/*!< Entprell-Counter fuer R-Encoder */
	register uint8 enc_tmp;					// Pegel der Encoderpins im Register zwischenspeichern
	
	#ifdef SPEED_CONTROL_AVAILABLE 
		register uint16 ticks = TIMER_GET_TICKCOUNT_16;	// aktuelle Systemzeit zwischenspeichern
		register uint8 i_time;							// Index des Timestamparrays zwischenspeichern
	#endif	// SPEED_CONTROL_AVAILABLE 
	/* Rad-Encoder links */
	enc_tmp = ENC_L;
	if (enc_tmp != enc_l){	// uns interesieren nur Veraenderungen
		enc_l=enc_tmp;		// neuen Wert sichern
		enc_l_cnt=0;		// Counter zuruecksetzen
	} else {				// zaehlen, wie lange Pegel bleibt
		if (enc_l_cnt < ENC_ENTPRELL) // Nur bis zur Entprell-Marke
			enc_l_cnt++;				
		else if (enc_l_cnt == ENC_ENTPRELL){ 	// wenn lange genug konstant
			enc_l_cnt++;	// diese Flanke nur einmal auswerten
			if (direction.left == DIRECTION_FORWARD)	// Drehrichtung beachten
				sensEncL++;	//vorwaerts
			else 
				sensEncL--;	//rueckwaerts
			#ifdef SPEED_CONTROL_AVAILABLE
				/* Timestamps fuer Regler links verschieben und speichern */ 
				i_time = (i_encTimeL + sizeof(encTimeL[0])) & 0xf;	// encTime ist Z/8Z und jeder Eintrag hat 2 Byte => 0xf
				*(uint16*)((uint8*)encTimeL + i_time) = ticks;
				i_encTimeL = i_time;
				/* Regleraufruf links */
				if (timeCorrectL == 0) speed_control(0, (int16*)&motor_left, (uint16*)encTimeL, i_encTimeL, enc_tmp);
				else timeCorrectL = 0;		
				/* pro TIMER_STEP wird maximal ein Encoder ausgewertet, da max alle 6 ms (Fullspeed) eine Flanke kommen kann */
				return;	// hackhack	
			#endif // SPEED_CONTROL_AVAILABLE		
		}
	}
	
	/* Rad-Encoder rechts */
	enc_tmp = ENC_R;
	if (enc_tmp != enc_r){	// uns interesieren nur Veraenderungen
		enc_r=enc_tmp;		// neuen Wert sichern
		enc_r_cnt=0;		// Counter zuruecksetzen
	} else { 				// zaehlen, wie lange Pegel bleibt
		if (enc_r_cnt < ENC_ENTPRELL)	// nur bis zur Entprell-Marke
			enc_r_cnt++;	
		else if (enc_r_cnt == ENC_ENTPRELL){ 	// wenn lange genug konstant
			enc_r_cnt++;	// diese Flanke nur einmal auswerten
			if (direction.right == DIRECTION_FORWARD)	// Drehrichtung beachten
				sensEncR++;	//vorwaerts
			else 
				sensEncR--;	//rueckwaerts
			#ifdef SPEED_CONTROL_AVAILABLE
				/* Timestamps fuer Regler rechts verschieben und speichern */ 
				i_time = (i_encTimeR + sizeof(encTimeR[0])) & 0xf;	// encTime ist Z/8Z und jeder Eintrag hat 2 Byte => 0xf
				*(uint16*)((uint8*)encTimeR + i_time) = ticks;
				i_encTimeR = i_time;
				/* Regleraufruf rechts */
				if (timeCorrectR == 0) speed_control(1, (int16*)&motor_right, (uint16*)encTimeR, i_encTimeR, enc_tmp);
				else timeCorrectR = 0;
			#endif // SPEED_CONTROL_AVAILABLE
		}
	}
}
#endif	// MCU
