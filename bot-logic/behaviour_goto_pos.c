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
 * @file 	behaviour_goto_pos.c
 * @brief 	Anfahren einer Position
 * @author 	Timo Sandmann (mail@timosandmann.de)
 * @date 	15.10.2007
 */

/*
 *TODO:	goto_pos:
 *		- Speedcontrol fuer unterschiedliche Speeds optimieren?
 * 		- (float-) Code optimieren
 * 		- Fehler / Nachlauf messen und ab ins EEPROM damit?
 * 		- Wrapper fuer drive_distance() und goto_xy() bauen
 */ 

#include "bot-logic/bot-logik.h"

#ifdef BEHAVIOUR_GOTO_POS_AVAILABLE
#include <stdlib.h>
#include <math.h>
#include "math_utils.h"
#include "bot-local.h"
#include "log.h"


//#define DEBUG_BOT_LOGIC		// Schalter um recht viel Debug-Code anzumachen

#ifndef DEBUG_BOT_LOGIC
	#undef LOG_DEBUG
	#define LOG_DEBUG(a, ...) {}
#endif


#ifdef MCU
#ifndef SPEED_CONTROL_AVAILABLE
#error "Das goto_pos-Verhalten geht nur mit Motorregelung!"
#endif
#endif

static float dest_x = 0;		/*!< x-Komponente des Zielpunktes */
static float dest_y = 0;		/*!< y-Komponente des Zielpunktes */
static int16_t dest_head = 0;	/*!< gewuenschte Blickrichtung am Zielpunkt */
static uint8_t state = 0;		/*!< Status des Verhaltens */

#define FIRST_TURN	0			/*!< Erste Drehung in ungefaehre Zielrichtung */
#define CALC_WAY	1			/*!< Berechnung des Kreisbogens */
#define RUNNING		2			/*!< Fahrt auf der berechneten Kreisbahn */
#define LAST_TURN	3			/*!< Abschliessende Drehung */

#ifdef MCU
static const int16_t target_margin	= 20;	/*!< Entfernung zum Ziel [mm], ab der das Ziel als erreicht gilt fuer MCU */
#else
static const int16_t target_margin	= 5;	/*!< Entfernung zum Ziel [mm], ab der das Ziel als erreicht gilt fuer PC */
#endif
static const int16_t straight_go	= 200;	/*!< Entfernung zum Ziel [mm], bis zu der geradeaus zum Ziel gefahren wird */
static const int16_t max_angle		= 30;	/*!< Maximaler Winkel [Grad] zwischen Start-Blickrichtung und Ziel */
static const int16_t v_min			= 100;	/*!< Minimale (mittlere) Geschwindigkeit [mm/s], mit der der Bot zum Ziel fahert */
static const int16_t v_max			= 200;	/*!< Maximale (mittlere) Geschwindigkeit [mm/s], mit der der Bot zum Ziel fahert */
static const int16_t recalc_dist	= 30;	/*!< Entfernung [mm], nach der die Kreisbahn neu berechnet wird */

/*!
 * @brief		Das Positionierungsverhalten
 * @param *data	Der Verhaltensdatensatz
 */
void bot_goto_pos_behaviour(Behaviour_t * data) {
	static float last_x;
	static float last_y;
	static int16_t done;
	static int16_t v_m;
	static int16_t v_l;
	static int16_t v_r;		
	
	/* Abstand zum Ziel berechnen (als Metrik euklidischen Abstand benutzen) */
	int16_t diff_to_target = sqrt(pow(dest_x-x_pos, 2) + pow(dest_y-y_pos, 2));

	/* Pruefen, ob wir schon am Ziel sind */
	if (diff_to_target < target_margin) state = LAST_TURN;
	
	switch (state) {
	case FIRST_TURN: {
		/* ungefaehr in die Zielrichtung drehen */
		LOG_DEBUG("first turn");
		int16_t alpha = calc_angle_diff(dest_x-x_pos, dest_y-y_pos);
		LOG_DEBUG("alpha=%d", alpha);
		if (diff_to_target < straight_go) {
			LOG_DEBUG("bot_turn(%d)", alpha);
			bot_turn(data, alpha);
			state = CALC_WAY;
			return;
		}
		if (abs(alpha) > max_angle) {
			int16_t to_turn = abs(alpha) - (max_angle-10);
			if (alpha < 0) to_turn = -to_turn;
			bot_turn(data, to_turn);
			LOG_DEBUG("bot_turn(%d)", to_turn);
		}
		state = CALC_WAY;
		break;
	}
	case CALC_WAY: {
		/* Kreisbogenfahrt zum Ziel berechnen */
		LOG_DEBUG("calc way...");
		/* Winkel- und Streckenbezeichnungen wie in -> Documentation/images/bot_pos.png */
		float diff_x = dest_x - x_pos;
		float diff_y = dest_y - y_pos;
		float alpha = heading;
		LOG_DEBUG("alpha=%f", alpha);
		float beta = calc_angle_diff(diff_x, diff_y);
		LOG_DEBUG("beta=%f", beta);
		float gamma = 90 - alpha - beta;
		alpha *= 2.0*M_PI/360.0;
		if (fmod(alpha, M_PI) == 0) alpha += 0.000001;
		beta *= 2.0*M_PI/360.0;
		if (fmod(beta, M_PI) == 0) beta = M_PI + 0.000001;
		gamma *= 2.0*M_PI/360.0;
		float h1 = diff_y / 2.0;
		float h2 = diff_x / 2.0;
		float h4 = h1 * tan(alpha);
		float h5 = h4 / sin(alpha);
		float h6 = (h2 + h4) * sin(gamma);
		float h7 = h6 / sin(beta);
		float radius = h5 + h7;
		LOG_DEBUG("radius=%f", radius);
		if (fabs(radius) < 50.0) {
			/* zu starke Kruemmung der Kreisbahn. 
			 * Wenn der Radius zu klein wird, bekommen wir fuer die Raeder Geschwindigkeiten,
			 * die kleiner bzw. groesser als moeglich sind, auesserdem faehrt der Bot dann
			 * eher Slalom, darum beginnen wir in diesem Fall neu mit dem Verhalten.
			 * Grenze: |radius| = 48.5 * v_min / (v_min - 100), mit v_min := 50 => |radius| = 48.5 */
			state = FIRST_TURN;
			speedWishLeft = BOT_SPEED_STOP;
			speedWishRight = BOT_SPEED_STOP;
			return;
		}
		/* Geschwindigkeit an Entfernung zum Zielpunkt anpassen */
		float x = diff_to_target < 360 ? diff_to_target / (360.0/M_PI*2.0) : M_PI/2;	// (0; pi/2]
		v_m = sin(x) * (float)(v_max - v_min);	// [    0; v_max-v_min]
		v_m += v_min;							// [v_min; v_max]
		/* Geschwindigkeiten auf die beiden Raeder verteilen, um den berechneten Radius der Kreisbahn zu erhalten */
		v_l = iroundf(radius / (radius + ((float)WHEEL_TO_WHEEL_DIAMETER/2.0)) * (float)v_m);
		v_r = iroundf(radius / (radius - ((float)WHEEL_TO_WHEEL_DIAMETER/2.0)) * (float)v_m);
		/* Statusupdate */
		last_x = x_pos;
		last_y = y_pos;
		done = 0;
		state = RUNNING;
		/* no break */
	}
	case RUNNING: {
		/* Berechnete Geschwindigkeiten setzen */
		LOG_DEBUG("v_l=%d; v_r=%d", v_l, v_r);
		LOG_DEBUG("x_pos=%f; y_pos=%f", x_pos, y_pos);
		speedWishLeft = v_l;
		speedWishRight = v_r;
		/* Alle recalc_dist mm rechnen wir neu, um Fehler zu korrigieren */
		done += sqrt(pow(last_x - x_pos,2) + pow(last_y - y_pos,2));
		if (done > recalc_dist) state = CALC_WAY;
		last_x = x_pos;
		last_y = y_pos;
		break;
	}
	case LAST_TURN: {
		/* Nachlauf abwarten */
		LOG_DEBUG("Nachlauf abwarten...");
		speedWishLeft = BOT_SPEED_STOP;
		speedWishRight = BOT_SPEED_STOP;
		BLOCK_BEHAVIOUR(data, 1200);
		LOG_INFO("Fehler=%d mm", diff_to_target);
		return_from_behaviour(data);
		if (dest_head == 999) {
			/* kein Drehen gewuenscht => fertig */
			return;
		} else {
			/* Noch in die gewuenschte Blickrichtung drehen */
			int16_t to_turn = (int16_t)(dest_head - (int16_t)heading) % 360;
			if (to_turn > 180) to_turn = 360 - to_turn;
			else if (to_turn < -180) to_turn += 360;
			LOG_DEBUG("to_turn=%d", to_turn);
			bot_turn(NULL, to_turn);
		}
	}
	}
}

/*!
 * @brief			Botenfunktion des Positionierungsverhaltens.
 * 					Faehrt einen absoluten angegebenen Punkt an und dreht den Bot in die gewuenschte Blickrichtung.
 * @param *caller	Der Verhaltensdatensatz des Aufrufers
 * @param x			x-Komponente des Ziels
 * @param y			y-Komponente des Ziels
 * @param head		neue Blickrichtung am Zielpunkt oder 999, falls egal
 */
void bot_goto_pos(Behaviour_t * caller, int16_t x, int16_t y, int16_t head) {
	/* Verhalten starten */
	switch_to_behaviour(caller, bot_goto_pos_behaviour, OVERRIDE);
	
	/* Inits */
	state = FIRST_TURN;
	dest_x = x;
	dest_y = y;
	dest_head = head;
	
	LOG_INFO("(%d mm|%d mm|%d Grad)", x, y, head);
}

/*!
 * @brief			Botenfunktion des Positionierungsverhaltens.
 * 					Faehrt einen als Verschiebungsvektor angegebenen Punkt an und dreht den Bot in die gewuenschte Blickrichtung.
 * @param *caller	Der Verhaltensdatensatz des Aufrufers
 * @param x			x-Komponente des Vektors vom Standort zum Ziel
 * @param y			y-Komponente des Vektors vom Standort zum Ziel
 * @param head		neue Blickrichtung am Zielpunkt oder 999, falls egal
 */
void bot_goto_pos_rel(Behaviour_t * caller, int16_t x, int16_t y, int16_t head) {
	bot_goto_pos(caller, x_pos + x, y_pos + y, head);
}

#endif	// BEHAVIOUR_GOTO_POS_AVAILABLE
