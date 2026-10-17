/* Capteur Grove BME280 - driver Arduino leger (sans bibliotheque externe).
 *
 * Principe :
 *  1. Au demarrage, on lit les "coefficients de calibration" graves en usine
 *     dans chaque capteur (ils sont differents d'un capteur a l'autre).
 *  2. A chaque mesure, on demande UNE mesure au capteur (mode "forced"),
 *     on attend qu'elle soit finie, puis on lit les valeurs brutes.
 *  3. On corrige les valeurs brutes avec les formules de Bosch (datasheet,
 *     section 4.2.3 et 8.2) pour obtenir des degres, des % et des hPa.
 *
 * ATTENTION Arduino Uno : un "int" fait 16 bits. Tous les calculs sont donc
 * faits explicitement en int32_t / uint32_t (d'ou les nombreux casts).
 */
#include <Arduino.h>
#include <Wire.h>
#include "grove_bme280.h"

#define BME280_CHIP_ID      0x60
#define REG_CALIB_T_P       0x88   /* 0x88 a 0x9F : 24 octets */
#define REG_CALIB_H1        0xA1
#define REG_CALIB_H2        0xE1   /* 0xE1 a 0xE7 : 7 octets  */
#define REG_CHIP_ID         0xD0
#define REG_RESET           0xE0
#define REG_CTRL_HUM        0xF2
#define REG_STATUS          0xF3
#define REG_CTRL_MEAS       0xF4
#define REG_CONFIG          0xF5
#define REG_DATA            0xF7   /* 0xF7 a 0xFE : 8 octets */

static uint8_t adresse = 0;        /* 0 = capteur non trouve */

/* Coefficients de calibration (noms de la datasheet Bosch) */
static uint16_t dig_T1;  static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;  static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t  dig_H1, dig_H3;
static int16_t  dig_H2, dig_H4, dig_H5;
static int8_t   dig_H6;
static int32_t  t_fine;            /* valeur intermediaire partagee par les 3 calculs */

/* ---------- Acces I2C ---------- */
static bool ecrire_registre(uint8_t reg, uint8_t valeur)
{
    Wire.beginTransmission(adresse);
    Wire.write(reg);
    Wire.write(valeur);
    return Wire.endTransmission() == 0;
}

static bool lire_registres(uint8_t reg, uint8_t *tampon, uint8_t nb)
{
    Wire.beginTransmission(adresse);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(adresse, nb) != nb) return false;
    for (uint8_t i = 0; i < nb; i++) tampon[i] = (uint8_t)Wire.read();
    return true;
}

static bool essayer_adresse(uint8_t a)
{
    uint8_t id = 0;
    adresse = a;
    if (lire_registres(REG_CHIP_ID, &id, 1) && id == BME280_CHIP_ID) return true;
    adresse = 0;
    return false;
}

/* ---------- Lecture des coefficients de calibration ---------- */
static bool lire_calibration(void)
{
    uint8_t b[24];
    if (!lire_registres(REG_CALIB_T_P, b, 24)) return false;
    dig_T1 = (uint16_t)(((uint16_t)b[1] << 8) | b[0]);
    dig_T2 = (int16_t) (((uint16_t)b[3] << 8) | b[2]);
    dig_T3 = (int16_t) (((uint16_t)b[5] << 8) | b[4]);
    dig_P1 = (uint16_t)(((uint16_t)b[7] << 8) | b[6]);
    dig_P2 = (int16_t) (((uint16_t)b[9] << 8) | b[8]);
    dig_P3 = (int16_t) (((uint16_t)b[11] << 8) | b[10]);
    dig_P4 = (int16_t) (((uint16_t)b[13] << 8) | b[12]);
    dig_P5 = (int16_t) (((uint16_t)b[15] << 8) | b[14]);
    dig_P6 = (int16_t) (((uint16_t)b[17] << 8) | b[16]);
    dig_P7 = (int16_t) (((uint16_t)b[19] << 8) | b[18]);
    dig_P8 = (int16_t) (((uint16_t)b[21] << 8) | b[20]);
    dig_P9 = (int16_t) (((uint16_t)b[23] << 8) | b[22]);

    if (!lire_registres(REG_CALIB_H1, &dig_H1, 1)) return false;

    uint8_t h[7];
    if (!lire_registres(REG_CALIB_H2, h, 7)) return false;
    dig_H2 = (int16_t)(((uint16_t)h[1] << 8) | h[0]);
    dig_H3 = h[2];
    dig_H4 = (int16_t)(((int16_t)(int8_t)h[3] * 16) | (h[4] & 0x0F));
    dig_H5 = (int16_t)(((int16_t)(int8_t)h[5] * 16) | (h[4] >> 4));
    dig_H6 = (int8_t)h[6];
    return true;
}

/* ---------- Formules de compensation Bosch (version entiers 32 bits) ---------- */

/* Resultat en centiemes de degre : 5123 = 51.23 degres C */
static int32_t compenser_T(int32_t adc_T)
{
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t d    = (adc_T >> 4) - (int32_t)dig_T1;
    int32_t var2 = (((d * d) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

/* Resultat en Pascals : 101325 = 1013.25 hPa */
static uint32_t compenser_P(int32_t adc_P)
{
    int32_t var1 = (t_fine >> 1) - (int32_t)64000;
    int32_t var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
    var1 = ((((int32_t)dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3)
            + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
    var1 = (((int32_t)32768 + var1) * ((int32_t)dig_P1)) >> 15;
    if (var1 == 0) return 0;   /* evite une division par zero */

    uint32_t p = (((uint32_t)((int32_t)1048576 - adc_P)) - (uint32_t)(var2 >> 12)) * 3125UL;
    if (p < 0x80000000UL) p = (p << 1) / (uint32_t)var1;
    else                  p = (p / (uint32_t)var1) * 2;

    var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
    return (uint32_t)((int32_t)p + ((var1 + var2 + (int32_t)dig_P7) >> 4));
}

/* Resultat en %HR x 1024 : 47445 = 46.33 % */
static uint32_t compenser_H(int32_t adc_H)
{
    int32_t v = t_fine - (int32_t)76800;
    v = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v)) + (int32_t)16384) >> 15)
         * (((((((v * ((int32_t)dig_H6)) >> 10) * (((v * ((int32_t)dig_H3)) >> 11) + (int32_t)32768)) >> 10)
              + (int32_t)2097152) * ((int32_t)dig_H2) + (int32_t)8192) >> 14));
    v = v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4);
    if (v < 0) v = 0;
    if (v > (int32_t)419430400) v = (int32_t)419430400;
    return (uint32_t)(v >> 12);
}

/* ---------- Fonctions publiques ---------- */

bool GroveBME280_Init(void)
{
    Wire.begin();
    if (!essayer_adresse(0x76) && !essayer_adresse(0x77)) return false;

    ecrire_registre(REG_RESET, 0xB6);           /* redemarrage du capteur */
    delay(10);
    uint8_t statut = 1;
    for (uint8_t i = 0; i < 20 && (statut & 0x01); i++) {   /* attente copie calibration */
        if (!lire_registres(REG_STATUS, &statut, 1)) return false;
        delay(2);
    }
    if (!lire_calibration()) { adresse = 0; return false; }

    ecrire_registre(REG_CONFIG, 0x00);          /* pas de filtre */
    return true;
}

bool GroveBME280_ReadChipId(uint8_t *chip_id)
{
    if (chip_id == NULL || adresse == 0) return false;
    return lire_registres(REG_CHIP_ID, chip_id, 1);
}

bool GroveBME280_ReadEnvironment(Env_Data *data)
{
    if (data == NULL) return false;
    if (adresse == 0 && !GroveBME280_Init()) return false;   /* capteur rebranche ? */

    /* Une mesure en mode "forced" : humidite x1 (a ecrire AVANT ctrl_meas),
       puis temperature x1, pression x1, mode forced (0b001 001 01 = 0x25). */
    if (!ecrire_registre(REG_CTRL_HUM, 0x01) || !ecrire_registre(REG_CTRL_MEAS, 0x25)) {
        adresse = 0;               /* le capteur ne repond plus : on refera l'init */
        return false;
    }

    /* Attente de la fin de la mesure (environ 10 ms, 50 ms maximum) */
    uint8_t statut = 0x08;
    unsigned long debut = millis();
    do {
        delay(2);
        if (!lire_registres(REG_STATUS, &statut, 1)) { adresse = 0; return false; }
    } while ((statut & 0x08) && (millis() - debut < 50));

    uint8_t b[8];
    if (!lire_registres(REG_DATA, b, 8)) { adresse = 0; return false; }

    int32_t adc_P = (int32_t)(((uint32_t)b[0] << 12) | ((uint32_t)b[1] << 4) | (b[2] >> 4));
    int32_t adc_T = (int32_t)(((uint32_t)b[3] << 12) | ((uint32_t)b[4] << 4) | (b[5] >> 4));
    int32_t adc_H = (int32_t)(((uint32_t)b[6] << 8) | b[7]);

    /* 0x80000 / 0x8000 = valeur "pas de mesure" renvoyee par le capteur */
    if (adc_T == 0x80000L || adc_P == 0x80000L || adc_H == 0x8000L) return false;

    data->temperature_c    = compenser_T(adc_T) / 100.0f;          /* doit etre calcule en premier */
    data->pressure_hpa     = compenser_P(adc_P) / 100.0f;
    data->humidity_percent = compenser_H(adc_H) / 1024.0f;
    return true;
}