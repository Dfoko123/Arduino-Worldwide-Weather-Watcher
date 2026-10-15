#ifndef GROVE_GPS_AIR530Z_H
#define GROVE_GPS_AIR530Z_H

#include <stdbool.h>
#include "cesi_types.h"

/* GPS Grove Air530Z (liaison serie logicielle). Memes fonctions que la version STM32. */
void GroveGPS_Init(void);

/* Lit une ligne NMEA (ex : "$GNGGA,...") dans buffer. false si rien recu avant timeout_ms. */
bool GroveGPS_ReadLine(char *buffer, unsigned int max_len, unsigned int timeout_ms);

/* Attend une trame GGA et remplit gps. false si aucune trame GGA avant timeout_ms.
   ATTENTION : bloque le programme pendant au maximum timeout_ms. */
bool GroveGPS_ReadGGA(GPS_Data *gps, unsigned int timeout_ms);

#endif
