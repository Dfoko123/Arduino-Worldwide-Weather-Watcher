#ifndef GROVE_BME280_H
#define GROVE_BME280_H

#include <stdint.h>
#include <stdbool.h>
#include "cesi_types.h"

/* Capteur Grove BME280 (I2C) : temperature, humidite, pression.
   Driver ecrit sans bibliotheque externe (plus leger pour l'Arduino Uno).
   Les calculs de compensation sont ceux de la documentation Bosch (BST-BME280-DS002). */

bool GroveBME280_Init(void);                     /* true si le capteur repond        */
bool GroveBME280_ReadChipId(uint8_t *chip_id);   /* doit valoir 0x60 pour un BME280  */
bool GroveBME280_ReadEnvironment(Env_Data *data);/* false si le capteur ne repond pas */

#endif