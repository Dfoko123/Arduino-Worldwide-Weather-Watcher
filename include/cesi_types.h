#ifndef CESI_TYPES_H
#define CESI_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Types repris du depot du prof (version STM32), adaptes a l'Arduino Uno. */

typedef struct {
    uint8_t  seconds;
    uint8_t  minutes;
    uint8_t  hours;
    uint8_t  dayOfWeek;   /* 1 = lundi ... 7 = dimanche */
    uint8_t  day;
    uint8_t  month;
    uint16_t year;        /* ex : 2026 */
} RTC_DateTime;

/* Difference avec la version STM32 : le champ raw_sentence[96] a ete retire
   pour economiser la memoire (l'Arduino Uno n'a que 2 Ko de RAM). */
typedef struct {
    bool    valid;        /* true si le GPS a une position (fix) */
    double  latitude;     /* degres decimaux, negatif = Sud  */
    double  longitude;    /* degres decimaux, negatif = Ouest */
    uint8_t fix_quality;
    uint8_t satellites;
} GPS_Data;

typedef struct {
    float temperature_c;
    float humidity_percent;
    float pressure_hpa;
    float gas_resistance_ohm;
} Env_Data;

/* ================== AJOUTS DU GROUPE ================== */
/* (le membre 1 colle ici le bloc de sa fiche, etape 1.1) */

#endif
