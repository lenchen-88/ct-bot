/*
 * c't-Bot - Robotersimulator fuer den c't-Bot
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

/*! @file 	map.c  
 * @brief 	Karte 
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	19.09.06
*/
#include <stdio.h>
#include "ct-Bot.h"
#include "bot-local.h"
#include "sensor_correction.h"
#include "map.h"

#ifdef MAP_AVAILABLE
#include <math.h>
#include <stdlib.h>

/*
 * Eine Karte ist wie folgt organisiert:
 * Es gibt Sektionen zu je MAP_SECTION_WIDTH cm * MAP_SECTION_HEIGHT cm. Diese Sektionen enthalten direkt die Pixel-Daten
 * Ein uebergeordnetes Array haelt die einzelnen Sections 
 * So laesst sich leicht eine Karte aufbauen, 
 * ohne dass viel Platz fuer unbenutzte Felder draufgeht
 */

#ifdef MCU
	#define MAP_SIZE			4	/*! Kantenlaenge der Karte in Metern. Ursprung ist der Startplatz des Bots */
	#define MAP_SECTION_POINTS 32	/*!< Kantenlaenge einer Section in Punkten ==> eine Section braucht MAP_SECTION_POINTS*MAP_SECTION_POINTS Bytes  */
	#define MAP_RESOLUTION 	(MAP_SECTION_POINTS/MAP_SIZE)	/*!< Aufloesung der Karte in Punkte pro Meter */
#else
	#define MAP_SIZE			20	/*! Kantenlaenge der Karte in Metern. Ursprung ist der Startplatz des Bots */
	#define MAP_RESOLUTION 	512	/*!< Aufloesung der Karte in Punkte pro Meter */
	#define MAP_SECTION_POINTS 128	/*!< Kantenlaenge einer Section in Punkten ==> eine Section braucht MAP_SECTION_POINTS*MAP_SECTION_POINTS Bytes  */
#endif

#define MAP_SECTIONS ((( MAP_SIZE*MAP_RESOLUTION)/MAP_SECTION_POINTS))

/* Felder werden wie folgt aktualisiert:
 * Wird ein Punkt al frei betrachtet, erhoehen wir den Wert des Feldes um MAP_STEP_FREE
 * Wird ein Punkt als belegt erkannt, ziehen wir um ihn einen Streukreis mit dem Radius MAP_RADIUS. 
 * Das Zentrum des Kreises wird um MAP_STEP_OCCUPIED dekrementiert, nach aussen hin immer weniger
 */ 

#define MAP_STEP_FREE		2	/*!< Stufenbreite, wenn ein Feld als frei erkannt wird */
#define MAP_STEP_OCCUPIED	10	/*!< Stufenbreite, wenn ein Feld als belegt erkannt wird */

#define MAP_RADIUS			10	/*!< Umkreis einen Messpunkt, der als Besetzt aktualisiert wird (Streukreis) [mm]*/
#define MAP_RADIUS_FIELDS	(MAP_RESOLUTION*MAP_RADIUS/1000)	/*!< Umkreis einen Messpunkt, der als Besetzt aktualisiert wird (Streukreis) [Felder]*/

/* Es lohnt nicht gigantische Karten auszugeben, wenn sie nichts enthalten, daher hier zwei Varianten, um die Karte auf die realen groesse zu reduzieren */
//#define SHRINK_MAP_ONLINE		/*!< Wenn gesetzt, wird bei jedem update der belegte Bereich der Karte protokolliert. Pro: schnelle ausgabe Contra permanenter aufwand  */
#define SHRINK_MAP_OFFLINE		/*!< Wenn gesetzt, wird erst beid er Ausgabe der belegte Bereich der Karte berechnet. Pro: kein permanenter aufwand Contra: ausgabe dauert lange */

#ifdef SHRINK_MAP_ONLINE
	int16 map_min_x=MAP_SIZE*MAP_RESOLUTION/2; /*!< belegten Bereich der Karte sichern */
	int16 map_max_x=MAP_SIZE*MAP_RESOLUTION/2;/*!< belegten Bereich der Karte sichern */
	int16 map_min_y=MAP_SIZE*MAP_RESOLUTION/2;/*!< belegten Bereich der Karte sichern */
	int16 map_max_y=MAP_SIZE*MAP_RESOLUTION/2;/*!< belegten Bereich der Karte sichern */
#endif

typedef struct {
	int8 section[MAP_SECTION_POINTS][MAP_SECTION_POINTS]; /*!< Einzelne Punkte */
} map_section_t;   /*!< Datentyp fuer die elementarfelder einer Gruppe */

map_section_t * map[MAP_SECTIONS][MAP_SECTIONS];	/*! Array mit den Zeigern auf die Elemente */

/*!
 * liefert den Wert eines Feldes 
 * @param x x-Ordinate der Karte (nicht der Welt!!!)
 * @param y y-Ordinate der Karte (nicht der Welt!!!)
 * @return Wert des Feldes (>0 heisst frei, <0 heisst belegt
 */
int8 map_get_field (int16 x, int16 y) {
	int16 section_x, section_y, index_x, index_y;
		
	// Berechne in welcher Sektion sich der Punkt befindet
	section_x=x/ MAP_SECTION_POINTS;
	section_y=y/ MAP_SECTION_POINTS;
	
		
	if ((section_x>= MAP_SECTIONS) || (section_y >= MAP_SECTIONS) ||
		(section_x < 0) || (section_y <0)){
		#ifdef PC
			printf("Versuch ein Feld ausserhalb der Karte zu lesen!! x=%d y=%d\n",x,y);
		#endif
		return 0;
	}

	// Eventuell existiert die Section noch nicht
	if (map[section_x][section_y] == NULL)
		return 0;

	// Berechne den Index innerhalb der Section
	index_x = x % MAP_SECTION_POINTS;
	index_y = y % MAP_SECTION_POINTS;

	return map[section_x][section_y]->section[index_x][index_y];	
}

/*!
 * Setzt den Wert eines Feldes auf den angegebenen Wert
 * @param x x-Ordinate der Karte (nicht der Welt!!!)
 * @param y y-Ordinate der Karte (nicht der Welt!!!)
 * @param value neuer wert des Feldes (> 0 heisst frei, <0 heisst belegt
 */
void map_set_field(int16 x, int16 y, int8 value) {
	int16 section_x, section_y, index_x, index_y;
		
	// Berechne in welcher Sektion sich der Punkt befindet
	section_x=x/ MAP_SECTION_POINTS;
	section_y=y/ MAP_SECTION_POINTS;
	
		
	if ((section_x>= MAP_SECTIONS) || (section_y >= MAP_SECTIONS) ||
		(x < 0) || (y <0)){
		#ifdef PC
			printf("Versuch ein Feld ausserhalb der Karte zu schreiben!! x=%d y=%d\n",x,y);	
		#endif
		return;
	}

	// Eventuell existiert die Section noch nicht
	if (map[section_x][section_y] == NULL){
		// Dann anlegen
		map[section_x][section_y]= malloc(sizeof(map_section_t));
		for (index_x=0; index_x<MAP_SECTION_POINTS; index_x++)
			for (index_y=0; index_y<MAP_SECTION_POINTS; index_y++)
				map[section_x][section_y]->section[index_x][index_y]=0;
	}

	// Berechne den Index innerhalb der Section
	index_x = x % MAP_SECTION_POINTS;
	index_y =  y % MAP_SECTION_POINTS;
	
	map[section_x][section_y]->section[index_x][index_y]=value;
	
	#ifdef SHRINK_MAP_ONLINE
		// Belegte Kartengroesse anpassen
		if (x < map_min_x)
			map_min_x=x;
		if (x > map_max_x)
			map_max_x= x;
		if (y < map_min_y)
			map_min_y=y;
		if (y > map_max_y)
			map_max_y= y;
	#endif
	
}

/*!
 * aendert den Wert eines Feldes um den angegebenen Betrag 
 * @param x x-Ordinate der Karte (nicht der Welt!!!)
 * @param y y-Ordinate der Karte (nicht der Welt!!!)
 * @param value Betrag um den das Feld veraendert wird (>= heisst freier, <0 heisst belegter
 */
void map_update_field (int16 x, int16 y, int16 value) {

	int16 tmp= map_get_field(x,y);
	if (tmp == -128)	// Nicht aktualiseren, wenn es sich um ein Loch handelt
		return;
			
	tmp+=value;
	
	// pruefen, ob kein ueberlauf stattgefunden hat und das Feld nicht schon als Loch (-128) markiert ist
	if ((tmp < 128) && (tmp > -128))
		map_set_field(x,y,(int8)tmp);
}

/*!
 * markiert ein Feld als frei -- drueckt den Feldwert etwas mehr in Richtung "frei"
 * @param x x-Ordinate der Karte (nicht der Welt!!!)
 * @param y y-Ordinate der Karte (nicht der Welt!!!)
 */
void map_update_free (int16 x, int16 y) {
  map_update_field(x,y,MAP_STEP_FREE);
}

/*!
 * aendert den Wert eines Feldes um den angegebenen Betrag 
 * @param x x-Ordinate der Karte (nicht der Welt!!!)
 * @param y y-Ordinate der Karte (nicht der Welt!!!)
 * @param value Betrag um den das Feld veraendert wird (>= heisst freier, <0 heisst belegter
 */
#if MAP_RADIUS_FIELDS > 0
	void map_update_field_circle(int16 x, int16 y, int16 radius, int16 value) {
		int16 dX,dY;
	    int16 h=radius*radius;
		for(dX = -radius; dX <= radius; dX++){
	      for(dY = -radius; dY <= radius; dY++) {
	           if(dX*dX + dY*dY <= h)	 
	           	 map_update_field (x + dX, y + dY,value);
	      }
		}
	}
#endif

/*!
 * markiert ein Feld als belegt -- drueckt den Feldwert etwas mehr in Richtung "belegt"
 * @param x x-Ordinate der Karte (nicht der Welt!!!)
 * @param y y-Ordinate der Karte (nicht der Welt!!!)
 */
void map_update_occupied (int16 x, int16 y) {
  // Nur wenn ein Umkreis gewuenscht ist, auch einen malen
  #if MAP_RADIUS_FIELDS > 0
	  uint8 r;
	  for (r=1; r<=MAP_RADIUS_FIELDS; r++){
	  	map_update_field_circle(x,y,r,-MAP_STEP_OCCUPIED/MAP_RADIUS_FIELDS);
	  }
  #else 
  	map_update_field(x,y,-MAP_STEP_OCCUPIED);
  #endif
}

/*!
 * Konvertiert eine Weltkoordinate in eine Kartenkoordinate
 * @param koord Weltkordiante
 * @return kartenkoordinate
 */
int16 world_to_map(float koord){
	int16 tmp = koord * MAP_RESOLUTION / 1000  + (MAP_SIZE*MAP_RESOLUTION/2);
	#ifdef PC
		if ((tmp < 0) || (tmp >= MAP_SIZE*MAP_RESOLUTION)){
			printf("Weltkoordinate %f passt nicht in die Karte!\n",koord);
		}
	#endif
		
	return tmp;
}


/*! 
 * Aktualisiert die Karte mit den Daten eines Sensors
 * @param x X-Achse der Position des Sensors
 * @param y Y-Achse der Position des Sensors
 * @param head Blickrichtung im Bogenmaß
 * @param dist Sensorwert 
 */ 
void update_map_sensor(float x, float y, float h, int16 dist){
	//Ort des Sensors in Kartenkoordinaten
	int16 X = world_to_map(x);
	int16 Y = world_to_map(y);
	
	int16 d;
	if (dist==SENS_IR_INFINITE)
		d=SENS_IR_MAX_DIST;
	else 
		d=dist;
		
		
	
	// Hinderniss, dass der Sensor sieht in Weltkoordinaten
	float PH_x = x + (DISTSENSOR_POS_FW + d) * cos(h);
	float PH_y = y + (DISTSENSOR_POS_FW + d) * sin(h);
	// Hinderniss, dass der linke Sensor sieht in Kartenkoordinaten
	int16 PH_X = world_to_map(PH_x);
	int16 PH_Y = world_to_map(PH_y);
	
	if ((dist > 80 ) && (dist <SENS_IR_INFINITE))
			map_update_occupied(PH_X,PH_Y);
	


	// Nun markiere alle Felder vor dem Hinderniss als frei
	int16 i;

	int16 lX = X; 	//	Beginne mit dem Feld, in dem der Sensor steht
	int16 lY = Y; 
	
	int8 sX = (PH_X < X ? -1 : 1);  
	int16 dX=abs(PH_X - X);		// Laenge der Linie in X-Richtung
	
	int8 sY = (PH_Y < Y ? -1 : 1); 
	int16 dY =abs(PH_Y - Y);	// Laenge der Linie in Y-Richtung

	if (dX >= dY) {			// Hangle Dich an der laengeren Achse entlang
	  dY--;	// stoppe ein Feld vor dem Hinderniss
	  int16 lh = dX / 2;
	  for (i=0; i<dX; ++i) {
	    map_update_free (lX+i*sX, lY);
	    lh += dY;
	    if (lh >= dX) {
	      lh -= dX;
	      lY += sY;
	    }
	  }
	} else {
	  dX--; // stoppe ein Feld vor dem Hinderniss
	  int16 lh = dY / 2;
	  for (i=0; i<dY; ++i) {
	    map_update_free (lX, lY+i*sY);
	    lh += dX;
	    if (lh >= dY) {
	      lh -= dY;
	      lX += sX;
	    }
	  }
	}	
}

/*!
 * Aktualisiert die interne Karte
 * @param x X-Achse der Position
 * @param y Y-Achse der Position
 * @param head Blickrichtung in Grad
 * @param distL Sensorwert links
 * @param distR Sensorwert rechts
 */
void update_map(float x, float y, float head, int16 distL, int16 distR){
//	printf("update_map: x=%f, y=%f, head=%f, distL=%d, distR=%d\n",x,y,head,distL,distR);
	
	int16 x_map = world_to_map(x);
	int16 y_map = world_to_map(y);
    float h= head * M_PI /180;
	
	// Aktualisiere zuerst die vom Bot selbst belegte Flaeche
	#define dim BOT_DIAMETER/2*MAP_RESOLUTION/100
	int16 dX,dY;
	for(dX = -dim; dX <= dim; dX++)
      for(dY = -dim; dY <= dim; dY++) {
           if(dX*dX + dY*dY <= dim*dim)	 
           	 map_update_free (x_map + dX, y_map + dY);
     }
        
    //Ort des rechten Sensors in Weltkoordinaten
    float Pr_x = x + (DISTSENSOR_POS_SW * sin(h));
	float Pr_y = y - (DISTSENSOR_POS_SW * cos(h));

    //Ort des linken Sensors in Weltkoordinaten
    float Pl_x = x - (DISTSENSOR_POS_SW * sin(h));
	float Pl_y = y + (DISTSENSOR_POS_SW * cos(h));

	update_map_sensor(Pl_x,Pl_y,h,distL);
	update_map_sensor(Pr_x,Pr_y,h,distR);
}


#ifdef PC
	/*!
	 *
	 */
	void map_to_pbm(char * filename){
		printf("Speichere Karte \n");
		FILE *fp = fopen(filename, "w");
		
		int16 x,y;

		#ifndef SHRINK_MAP_ONLINE
			int16 map_min_x=0;
			int16 map_max_x=MAP_SIZE*MAP_RESOLUTION;
			int16 map_min_y=0;
			int16 map_max_y=MAP_SIZE*MAP_RESOLUTION;
		#endif		
		
		#ifdef 	SHRINK_MAP_OFFLINE
		
			int16 min_x=0;
			int16 max_x=MAP_SIZE*MAP_RESOLUTION;
			int16 min_y=0;
			int16 max_y=MAP_SIZE*MAP_RESOLUTION;
			// Kartengroesse reduzieren
			int8 free=1;
			while ((min_y < max_y) && (free ==1)){
				for (x=min_x; x<max_x; x++){
					if (map_get_field(x,min_y) != 0){
						free=0;
						break;
					}					
				}
				min_y++;
			}
			free=1;
			while ((min_y < max_y) && (free ==1)){
				for (x=min_x; x<max_x; x++){
					if (map_get_field(x,max_y-1) != 0){
						free=0;
						break;
					}					
				}
				max_y--;
			}
			
			free=1;
			while ((min_x < max_x) && (free ==1)){
				for (y=min_y; y<max_y; y++){
					if (map_get_field(min_x,y) != 0){
						free=0;
						break;
					}					
				}
				min_x++;
			}
			free=1;
			while ((min_x < max_x) && (free ==1)){
				for (y=min_y; y<max_y; y++){
					if (map_get_field(max_x-1,y) != 0){
						free=0;
						break;
					}					
				}
				max_x--;
			}
	
			map_min_x=min_x;
			map_max_x=max_x;
			map_min_y=min_y;
			map_max_y=max_y;
	
		#endif
			
		int16 map_size_x=map_max_x-map_min_x;
		int16 map_size_y=map_max_y-map_min_y;
		fprintf(fp,"P5\n%d %d\n255\n",map_size_x,map_size_y);

		printf("Karte beginnt bei X=%d,Y=%d und geht bis X=%d,Y=%d (%d * %d Punkte)\n",map_min_x,map_min_y,map_max_x,map_max_y,map_size_x,map_size_y);
		
		uint8 tmp;
		for (y=map_max_y-1; y>= map_min_y; y--)
			for (x=map_min_x; x<map_max_x; x++){
				tmp=map_get_field(x,y)+128;
				fwrite(&tmp,1,1,fp);
			}
		fclose(fp);
		
	}
#endif

/*!
 * Zeigt die Karte an
 */
void print_map(void){
	#ifdef PC
		map_to_pbm("map.pgm");
	#else
		// Todo: Wie soll der Bot eine Karte ausgeben ....
	#endif
}



#endif
