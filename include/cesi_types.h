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
} Env_Data;

typedef enum {
    MODE_STANDARD = 0,
    MODE_CONFIG,
    MODE_MAINTENANCE,
    MODE_ECO
} Mode;
typedef enum {
    ERR_AUCUNE = 0,
    ERR_RTC,            /* LED rouge/bleu   1:1 */
    ERR_GPS,            /* LED rouge/jaune  1:1 */
    ERR_CAPTEUR_ACCES,  /* LED rouge/vert   1:1 */
    ERR_CAPTEUR_INCOH,  /* LED rouge/vert   1:2 (vert 2x plus long)  */
    ERR_SD_PLEINE,      /* LED rouge/blanc  1:1 */
    ERR_SD_ACCES,       /* LED rouge/blanc  1:2 (blanc 2x plus long) */
    ERR_NB              /* nombre d'erreurs : TOUJOURS en dernier */
} CodeErreur;
typedef struct {
    bool    valide;       /* false => "NA" dans le fichier            */
    uint8_t nb_timeouts;  /* echecs de suite ; 2 => capteur en erreur */
} EtatCapteur;
typedef struct {
    RTC_DateTime date;
    Env_Data     env;     /* temperature, humidite, pression */
    uint16_t     lumin;   /* 0 a 1023 */
    GPS_Data     gps;
    EtatCapteur  etat_temp, etat_hygr, etat_pression, etat_lumin, etat_gps;
} Mesure;
typedef struct {
    uint16_t log_interval;   /* minutes entre 2 mesures   (defaut 10)   */
    uint16_t file_max_size;  /* octets max par fichier    (defaut 2048) */
    uint8_t  timeout;        /* secondes avant abandon    (defaut 30)   */
    uint8_t  lumin;          /* capteur lumiere actif ?   (defaut 1)    */
    uint16_t lumin_low;      /*                           (defaut 255)  */
    uint16_t lumin_high;     /*                           (defaut 768)  */
    uint8_t  temp_air;       /* capteur temp. actif ?     (defaut 1)    */
    int8_t   min_temp_air;   /*                           (defaut -10)  */
    int8_t   max_temp_air;   /*                           (defaut 60)   */
    uint8_t  hygr;           /* capteur humidite actif ?  (defaut 1)    */
    int8_t   hygr_mint;      /*                           (defaut 0)    */
    int8_t   hygr_maxt;      /*                           (defaut 50)   */
    uint8_t  pressure;       /* capteur pression actif ?  (defaut 1)    */
    uint16_t pressure_min;   /*                           (defaut 850)  */
    uint16_t pressure_max;   /*                           (defaut 1080) */
} Parametres;


#endif