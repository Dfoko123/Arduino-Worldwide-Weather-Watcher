/* Capteur BME680 - version Arduino.
   Les calculs de compensation Bosch sont faits par la bibliotheque
   "Adafruit BME680 Library" (installee automatiquement via platformio.ini). */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include "grove_bme680.h"

static Adafruit_BME680 bme;
static uint8_t adresse_bme = 0;

static bool lire_registre(uint8_t adresse, uint8_t reg, uint8_t *valeur)
{
    Wire.beginTransmission(adresse);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(adresse, (uint8_t)1) != 1) return false;
    *valeur = (uint8_t)Wire.read();
    return true;
}

bool GroveBME680_Init(void)
{
    Wire.begin();
    /* Comme le driver du prof : on essaie l'adresse 0x76 puis 0x77 */
    if (bme.begin(0x76))      adresse_bme = 0x76;
    else if (bme.begin(0x77)) adresse_bme = 0x77;
    else                      return false;

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(0, 0);   /* mesure de gaz desactivee : inutile pour le projet */
    return true;
}

bool GroveBME680_ReadChipId(uint8_t *chip_id)
{
    if (chip_id == NULL || adresse_bme == 0) return false;
    return lire_registre(adresse_bme, 0xD0, chip_id);   /* doit valoir 0x61 */
}

bool GroveBME680_ReadEnvironment(Env_Data *data)
{
    if (data == NULL || adresse_bme == 0) return false;
    if (!bme.performReading()) return false;            /* echec = capteur absent/debranche */

    data->temperature_c      = bme.temperature;          /* degres C */
    data->humidity_percent   = bme.humidity;             /* %        */
    data->pressure_hpa       = bme.pressure / 100.0f;    /* Pa -> hPa */
    data->gas_resistance_ohm = 0.0f;
    return true;
}
