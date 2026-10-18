// Module d'acquisition des mesures météorologiques

// Déclarations de notre module
#include "app_acquisition.h"

// Pilote du capteur de lumière
#include "grove_light.h"

// Pilote du GPS
#include "grove_gps_air530z.h"

// Pilote du capteur de température, humidité et pression
#include "grove_bme280.h"

// Gestion des erreurs du projet
#include "app_errors.h"

// Bibliothèque Arduino
#include <Arduino.h>

// Initialisation des capteurs
void AppAcquisition_Init(void)
{
    // Initialiser le capteur de lumière
    GroveLight_Init();

    // Initialiser le GPS
    GroveGPS_Init();

    // Initialiser le capteur BME280
    if (GroveBME280_Init())
    {
        // Le capteur BME280 est initialisé correctement
        Erreur_Effacer(ERR_CAPTEUR_ACCES);
    }
    else
    {
        // Signaler une erreur d'accès au capteur
        Erreur_Signaler(ERR_CAPTEUR_ACCES);
    }
}