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

/**
 * \file 	sensor-low_pc.c
 * \brief 	Low-Level Routinen fuer die Sensor Steuerung des c't-Bots
 * \author 	Benjamin Benz (bbe@heise.de)
 * \date 	01.12.2005
 */

#ifdef PC
#include "ct-Bot.h"
#include "sensor-low.h"
#include "bot-2-sim.h"
#include "command.h"
#include "sensor.h"
#include <string.h>

/**
 * Initialisiere alle Sensoren.
 */
void bot_sens_init(void) {
#ifdef ARM_LINUX_BOARD
	sensor_update_distance = sensor_dist_straight; // Distanzsensordaten 1:1 weiterreichen
#endif
}

/**
 * Alle Sensoren aktualisieren.
 */
void bot_sens(void) {
	sensor_update(); // Weiterverarbeitung der rohen Sensordaten
	led_update(); // LEDs updaten
}
#endif // PC
