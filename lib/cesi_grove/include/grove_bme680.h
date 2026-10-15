#ifndef GROVE_BME680_H
#define GROVE_BME680_H

#include <stdint.h>
#include <stdbool.h>
#include "cesi_types.h"

/* Capteur BME680 (I2C) : temperature, humidite, pression.
   Memes fonctions que la version STM32, mais ICI la lecture est REELLE
   (calculs faits par la bibliotheque Adafruit BME680). */
bool GroveBME680_Init(void);                     /* true si le capteur repond */
bool GroveBME680_ReadChipId(uint8_t *chip_id);   /* doit valoir 0x61 */
bool GroveBME680_ReadEnvironment(Env_Data *data);

#endif
