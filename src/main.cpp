/*
 * Worldwide Weather Watcher - version ARDUINO UNO
 *
 * PROGRAMME DE TEST DU MATERIEL (temporaire).
 * Il verifie que chaque module Grove repond, en utilisant les outils
 * du dossier lib/cesi_grove/ (memes noms de fonctions que la version STM32).
 *
 * Plus tard, le membre 1 remplacera ce fichier par le vrai programme
 * (Modes_Init, Modes_Update, etc.).
 *
 * Sur Arduino : setup() est execute UNE fois au demarrage,
 *               loop()  est repete A L'INFINI (c'est le while(1) de la STM32).
 */
#include <Arduino.h>
#include "cesi_types.h"
#include "grove_rgb.h"
#include "grove_button.h"
#include "grove_light.h"
#include "grove_rtc_ds1307.h"
#include "grove_gps_air530z.h"
#include "grove_bme280.h"
#include "sd_logger.h"

static bool rtc_ok = false, bme_ok = false, sd_ok = false;
static unsigned long dernier_affichage = 0;

static void afficher_ok(const __FlashStringHelper *nom, bool ok)
{
    Serial.print(nom);
    Serial.println(ok ? F(" : OK") : F(" : ERREUR"));
}

static void afficher_2chiffres(uint8_t n)
{
    if (n < 10) Serial.print('0');
    Serial.print(n);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("===== TEST DU MATERIEL WWW (Arduino Uno) ====="));

    GroveRGB_Init();
    GroveButton_Init();
    GroveLight_Init();
    GroveGPS_Init();

    /* Test LED : rouge, vert, bleu */
    GroveRGB_SetColor(255, 0, 0);   delay(400);
    GroveRGB_SetColor(0, 255, 0);   delay(400);
    GroveRGB_SetColor(0, 0, 255);   delay(400);
    GroveRGB_Off();

    rtc_ok = GroveRTC_Init();
    bme_ok = GroveBME280_Init();
    sd_ok  = SDLogger_Init();
    if (sd_ok) sd_ok = SDLogger_WriteLine("TEST.LOG", "Test ecriture OK");

    afficher_ok(F("Horloge RTC"), rtc_ok);
    afficher_ok(F("Capteur BME280"), bme_ok);
    afficher_ok(F("Carte SD (ecriture TEST.LOG)"), sd_ok);
    Serial.println(F("Boutons : rouge -> LED rouge, vert -> LED verte."));
    Serial.println(F("----------------------------------------------"));
}

void loop()
{
    /* 1) Boutons : reaction immediate */
    if (GroveButton1_IsPressed())      GroveRGB_SetColor(255, 0, 0);
    else if (GroveButton2_IsPressed()) GroveRGB_SetColor(0, 255, 0);
    else                               GroveRGB_SetColor(0, 0, 40);

    /* 2) Mesures toutes les 3 secondes */
    if (millis() - dernier_affichage < 3000) return;
    dernier_affichage = millis();

    RTC_DateTime t;
    if (rtc_ok && GroveRTC_GetDateTime(&t)) {
        afficher_2chiffres(t.day);   Serial.print('/');
        afficher_2chiffres(t.month); Serial.print('/');
        Serial.print(t.year);        Serial.print(' ');
        afficher_2chiffres(t.hours);   Serial.print(':');
        afficher_2chiffres(t.minutes); Serial.print(':');
        afficher_2chiffres(t.seconds);
    } else {
        Serial.print(F("RTC NA"));
    }

    Env_Data env;
    Serial.print(F(" | BME280: "));
    if (bme_ok && GroveBME280_ReadEnvironment(&env)) {
        Serial.print(env.temperature_c, 1);    Serial.print(F(" C, "));
        Serial.print(env.humidity_percent, 1); Serial.print(F(" %, "));
        Serial.print(env.pressure_hpa, 1);     Serial.print(F(" hPa"));
    } else {
        Serial.print(F("NA"));
    }

    Serial.print(F(" | Lum: "));
    Serial.print(GroveLight_ReadRaw());

    GPS_Data gps;
    Serial.print(F(" | GPS: "));
    if (GroveGPS_ReadGGA(&gps, 1500)) {
        if (gps.valid) {
            Serial.print(gps.latitude, 5);  Serial.print(F(", "));
            Serial.print(gps.longitude, 5);
        } else {
            Serial.print(F("pas de position, satellites="));
            Serial.print(gps.satellites);
        }
    } else {
        Serial.print(F("NA (aucune trame recue)"));
    }
    Serial.println();
}