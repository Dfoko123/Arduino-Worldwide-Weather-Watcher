/* Horloge RTC DS1307 - version Arduino (bibliotheque Wire = I2C).
   Meme principe que le driver STM32 du prof : les registres 0x00 a 0x06
   contiennent secondes, minutes, heures, jour de semaine, jour, mois, annee
   au format BCD (chaque chiffre decimal sur 4 bits). */
#include <Arduino.h>
#include <Wire.h>
#include "grove_rtc_ds1307.h"
#include "board_pins.h"

static uint8_t bcd_to_dec(uint8_t v) { return (uint8_t)((v >> 4) * 10 + (v & 0x0F)); }
static uint8_t dec_to_bcd(uint8_t v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }

bool GroveRTC_Init(void)
{
    Wire.begin();
    Wire.beginTransmission(DS1307_I2C_ADDR);
    return Wire.endTransmission() == 0;      /* 0 = l'horloge a repondu */
}

bool GroveRTC_GetDateTime(RTC_DateTime *dt)
{
    if (dt == NULL) return false;

    Wire.beginTransmission(DS1307_I2C_ADDR);
    Wire.write((uint8_t)0x00);               /* on veut lire a partir du registre 0 */
    if (Wire.endTransmission() != 0) return false;

    if (Wire.requestFrom((uint8_t)DS1307_I2C_ADDR, (uint8_t)7) != 7) return false;

    dt->seconds   = bcd_to_dec(Wire.read() & 0x7F);
    dt->minutes   = bcd_to_dec(Wire.read() & 0x7F);
    dt->hours     = bcd_to_dec(Wire.read() & 0x3F);
    dt->dayOfWeek = bcd_to_dec(Wire.read() & 0x07);
    dt->day       = bcd_to_dec(Wire.read() & 0x3F);
    dt->month     = bcd_to_dec(Wire.read() & 0x1F);
    dt->year      = (uint16_t)(2000 + bcd_to_dec(Wire.read()));
    return true;
}

bool GroveRTC_SetDateTime(const RTC_DateTime *dt)
{
    if (dt == NULL) return false;

    Wire.beginTransmission(DS1307_I2C_ADDR);
    Wire.write((uint8_t)0x00);
    Wire.write(dec_to_bcd(dt->seconds) & 0x7F);   /* bit CH = 0 : l'horloge tourne */
    Wire.write(dec_to_bcd(dt->minutes));
    Wire.write(dec_to_bcd(dt->hours) & 0x3F);     /* format 24 h */
    Wire.write(dec_to_bcd(dt->dayOfWeek));
    Wire.write(dec_to_bcd(dt->day));
    Wire.write(dec_to_bcd(dt->month));
    Wire.write(dec_to_bcd((uint8_t)(dt->year % 100)));
    return Wire.endTransmission() == 0;
}
