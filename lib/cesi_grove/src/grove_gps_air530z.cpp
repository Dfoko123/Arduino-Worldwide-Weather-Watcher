/* GPS Grove Air530Z - version Arduino.
   L'Arduino Uno n'a qu'une seule vraie liaison serie (utilisee par l'USB/console),
   donc le GPS utilise une liaison serie LOGICIELLE (SoftwareSerial). */
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <string.h>
#include "grove_gps_air530z.h"
#include "board_pins.h"

static SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

void GroveGPS_Init(void)
{
    gpsSerial.begin(GPS_BAUDRATE);
}

bool GroveGPS_ReadLine(char *buffer, unsigned int max_len, unsigned int timeout_ms)
{
    if (buffer == NULL || max_len < 2) return false;

    unsigned int  n = 0;
    unsigned long debut = millis();

    while (millis() - debut < timeout_ms) {
        if (!gpsSerial.available()) continue;
        char c = (char)gpsSerial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            if (n == 0) continue;          /* ligne vide : on continue d'attendre */
            buffer[n] = '\0';
            return true;
        }
        if (n < max_len - 1) buffer[n++] = c;
    }
    buffer[0] = '\0';
    return false;
}

/* Recupere le champ numero 'index' (0 = "$GNGGA") d'une trame NMEA.
   Contrairement a strtok, les champs vides ",," sont bien comptes. */
static bool nmea_champ(const char *trame, uint8_t index, char *out, uint8_t out_len)
{
    uint8_t champ = 0, j = 0;
    for (const char *p = trame; *p != '\0' && *p != '*'; p++) {
        if (*p == ',') {
            if (champ == index) { out[j] = '\0'; return true; }
            champ++;
            continue;
        }
        if (champ == index && j < out_len - 1) out[j++] = *p;
    }
    out[j] = '\0';
    return champ == index;
}

/* "4851.2345" + "N" -> 48.853908 (degres decimaux) */
static double nmea_vers_degres(const char *valeur, const char *hemisphere)
{
    double v = atof(valeur);
    int degres = (int)(v / 100.0);
    double minutes = v - degres * 100.0;
    double resultat = degres + minutes / 60.0;
    if (hemisphere[0] == 'S' || hemisphere[0] == 'W') resultat = -resultat;
    return resultat;
}

bool GroveGPS_ReadGGA(GPS_Data *gps, unsigned int timeout_ms)
{
    if (gps == NULL) return false;
    gps->valid = false;

    char ligne[83];                     /* une trame NMEA fait au plus 82 caracteres */
    char champ[16];
    char hemi[2];
    unsigned long debut = millis();

    while (millis() - debut < timeout_ms) {
        unsigned int reste = timeout_ms - (unsigned int)(millis() - debut);
        if (!GroveGPS_ReadLine(ligne, sizeof(ligne), reste)) return false;

        /* On ne garde que les trames GGA : "$GPGGA" ou "$GNGGA" */
        if (strncmp(ligne + 3, "GGA", 3) != 0) continue;

        nmea_champ(ligne, 6, champ, sizeof(champ));
        gps->fix_quality = (uint8_t)atoi(champ);
        nmea_champ(ligne, 7, champ, sizeof(champ));
        gps->satellites = (uint8_t)atoi(champ);

        if (gps->fix_quality > 0) {
            nmea_champ(ligne, 2, champ, sizeof(champ));
            nmea_champ(ligne, 3, hemi, sizeof(hemi));
            gps->latitude = nmea_vers_degres(champ, hemi);
            nmea_champ(ligne, 4, champ, sizeof(champ));
            nmea_champ(ligne, 5, hemi, sizeof(hemi));
            gps->longitude = nmea_vers_degres(champ, hemi);
            gps->valid = true;
        }
        return true;   /* trame GGA recue (avec ou sans position) */
    }
    return false;
}
