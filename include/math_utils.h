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
 * @file 	math_utils.h
 * @brief 	Hilfsfunktionen fuer mathematische Dinge, architekturunabhaenigig
 * @author 	Timo Sandmann (mail@timosandmann.de)
 * @date 	17.10.2007
 */

#ifndef MATH_UTILS_H_
#define MATH_UTILS_H_

/*!
 * Rundet float und gibt das Ergebnis als int zurueck.
 * Selbst implementiert, weil es kein roundf() in der avr-libc gibt.
 * @param x	Eingabewert
 * @return	roundf(x)
 */
static inline int16_t iroundf(float x) {
	if (x >= 0) {
		return (int16_t) (x + 0.5);
	}
	return (int16_t) (x - 0.5);
}

/*!
 * Berechnung deiner Winkeldifferenz zwischen dem aktuellen Standpunkt und einem anderen Ort
 * @param xDiff	x-Differenz
 * @param yDiff	y-Differenz
 * @return 		berechnete Winkeldifferenz
 */
float calc_angle_diff(float xDiff, float yDiff);

/*!
 * Ermittelt das Vorzeichnen eines 8 Bit Integers. Interpretiert 0 immer als positiv.
 * @param z	Zahl, deren VZ gewuenscht ist
 * @return	1, falls z >= 0, -1 sonst
 */
static inline int16_t sign8(int8_t z) {
	return (z & 0x80) ? -1 : 1;
}

/*!
 * Ermittelt das Vorzeichnen eines 16 Bit Integers. Interpretiert 0 immer als positiv.
 * @param z	Zahl, deren VZ gewuenscht ist
 * @return	1, falls z >= 0, -1 sonst
 */
static inline int16_t sign16(int16_t z) {
	return (z & 0x8000) ? -1 : 1;
}

/*!
 * Ermittelt das Vorzeichnen eines 32 Bit Integers. Interpretiert 0 immer als positiv.
 * @param z	Zahl, deren VZ gewuenscht ist
 * @return	1, falls z >= 0, -1 sonst
 */
static inline int16_t sign32(int32_t z) {
	return (z & 0x80000000) ? -1 : 1;
}

/*!
 * Berechnet die Differenz eines Winkels zur aktuellen
 * Botausrichtung.
 * @param angle		Winkel [Grad] zum Vergleich mit heading 
 * @return			Winkeldifferenz [Grad] in Richtung der derzeitigen Botdrehung.
 * 					-1, falls Bot geradeaus faehrt oder steht
 */
int16_t turned_angle(int16_t angle);

/*!
 * Ermittlung des Quadrat-Abstandes zwischen 2 Koordinaten
 * @param x1 x-Koordinate des ersten Punktes
 * @param y1 y-Koordinate des ersten Punktes
 * @param x2 Map-Koordinate des Zielpunktes
 * @param y2 Map-Koordinate des Zielpunktes
 * @return liefert Quadrat-Abstand zwischen den Map-Punkten 
 */
int16 get_dist(int16 x1, int16 y1, int16 x2, int16 y2);

#endif	/*MATH_UTILS_H_*/
