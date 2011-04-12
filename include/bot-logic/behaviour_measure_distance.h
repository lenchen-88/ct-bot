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
 * @file 	behaviour_measure_distance.h
 * @brief 	Ermittelt die aktuelle Entfernung eines Hindernisses mit den Distanzsensoren
 * @author 	Timo Sandmann (mail@timosandmann.de)
 * @date 	27.04.2007
 */

#ifndef BEHAVIOUR_MEASURE_DISTANCE_H_
#define BEHAVIOUR_MEASURE_DISTANCE_H_

#ifdef BEHAVIOUR_MEASURE_DISTANCE_AVAILABLE
/*!
 * Das eigentliche Verhalten
 * @param *data	Zeiger auf den Verhaltensdatensatz des Aufrufers
 * @see			bot_measure_distance()
 * Das Verhalten beendet sich erst, wenn mehrere Messungen sinnvolle Werte ergaben
 */
void bot_measure_distance_behaviour(Behaviour_t * data);

/*!
 * Ermittelt die aktuelle Entfernung eines Hindernisses mit den Distanzsensoren
 * @param *caller	Zeiger auf den Verhaltensdatensatz des Aufrufers
 * @param *p_sensL	Zeiger auf eine Variable fuer den linken Sensorwert (Messwert-Ausgabe)
 * @param *p_sensR	Zeiger auf eine Variable fuer den rechten Sensorwert (Messwert-Ausgabe)
 * @param diff		Maximal zulaessige Differenz zwischen den Messungen
 */
void bot_measure_distance(Behaviour_t * caller, int16_t * p_sensL, int16_t * p_sensR, uint8_t diff);

/*!
 * Implementierung des check-distance-Verhaltens
 * @param *data	Zeiger auf den Verhaltensdatensatz
 */
void bot_check_distance_behaviour(Behaviour_t * data);

/*!
 * Prueft, ob in Entfernung max_dist [mm] ein Hindernis zu sehen ist.
 * Der Aufrufer bekommt BEHAVIOUR_SUBSUCCESS in seinen Verhaltensdatensatz, falls ja,
 * sonst BEHAVIOUR_SUBFAIL.
 * @param *caller	Zeiger auf den Verhaltensdatensatz des Aufrufers
 * @param max_dist	Entfernung in mm, bis zu der geprueft werden soll
 * @param diff		Maximal zulaessige Differenz zwischen den Messungen
 */
void bot_check_distance(Behaviour_t * caller, int16_t max_dist, uint8_t diff);

#endif // BEHAVIOUR_MEASURE_DISTANCE_AVAILABLE
#endif // BEHAVIOUR_MEASURE_DISTANCE_H_
