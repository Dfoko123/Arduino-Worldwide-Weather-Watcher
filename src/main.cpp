/*
 * Worldwide Weather Watcher - programme principal (Arduino Uno)
 *
 * setup() : demarre tous les composants, une seule fois.
 * loop()  : repete a l'infini :
 *             1. lire les boutons
 *             2. changer de mode si besoin
 *             3. faire le travail du mode actuel
 *             4. mettre la LED a jour
 */
#include <Arduino.h>
#include "cesi_types.h"

/* Outils (drivers) */
#include "grove_rgb.h"
#include "grove_button.h"
#include "grove_light.h"
#include "grove_rtc_ds1307.h"
#include "grove_gps_air530z.h"
#include "grove_bme280.h"

/* Modules du membre 1 */
#include "app_errors.h"
#include "app_led.h"
#include "app_buttons.h"
#include "app_modes.h"

/* Module du membre 2 (acquisition) : branche et teste */
#include "app_acquisition.h"

/* ===========================================================================
 * Fonctions des membres 3 et 4 (pas encore ecrites).
 * PHASE 3 : quand leurs fichiers existent, REMPLACER tout ce bloc par :
 *     #include "app_logger.h"
 *     #include "app_config.h"
 *     #include "app_console.h"
 * puis supprimer les versions "weak" ci-dessous.
 * ======================================================================== */
void          Config_Charger(void);
const Parametres *Config_Get(void);
bool          Logger_Init(void);
bool          Logger_Ecrire(const char *ligne, const RTC_DateTime *date);
void          Console_Init(void);
bool          Console_LireLigne(char *ligne, uint8_t taille);
void          Console_Executer(const char *cmd);
void          Console_AfficherMesure(const Mesure *m);

/* Versions de secours (remplacees automatiquement par les vraies) */
/* Valeurs par defaut du sujet (en attendant app_config du membre 4) */
static const Parametres PARAMS_DEFAUT = { 10, 2048, 30, 1, 255, 768, 1, -10, 60, 1, 0, 50, 1, 850, 1080 };
__attribute__((weak)) void Config_Charger(void) { }
__attribute__((weak)) const Parametres *Config_Get(void) { return &PARAMS_DEFAUT; }
__attribute__((weak)) bool Logger_Init(void) { return true; }
/* En attendant la carte SD (membre 3) : la ligne est affichee sur la console */
__attribute__((weak)) bool Logger_Ecrire(const char *ligne, const RTC_DateTime *)
{
    Serial.print(F("[SD] "));
    Serial.println(ligne);
    return true;
}
__attribute__((weak)) void Console_Init(void) { }
__attribute__((weak)) bool Console_LireLigne(char *, uint8_t) { return false; }
__attribute__((weak)) void Console_Executer(const char *) { }
/* En attendant la console (membre 4) : la mesure est affichee telle quelle */
__attribute__((weak)) void Console_AfficherMesure(const Mesure *m)
{
    char l[96];
    Acq_FormaterLigne(m, l, sizeof(l));
    Serial.print(F("[MAINT] "));
    Serial.println(l);
}
/* ======================================================================== */

/* En maintenance, les mesures sont affichees "en direct" : toutes les 5 s
   (choix du groupe, a confirmer avec le tuteur). */
#define MAINTENANCE_INTERVALLE_MS  5000UL

/* Fait UNE mesure complete et l'ecrit sur la SD ou l'affiche sur la console */
static void Faire_Mesure(bool mode_eco, bool vers_console)
{
    Mesure m;
    char ligne[96];

    Acq_LireTout(&m, mode_eco);                    /* membre 2 */
    Acq_VerifierSeuils(&m, Config_Get());          /* membre 2 */

    if (vers_console) {
        Console_AfficherMesure(&m);                /* membre 4 */
    } else {
        Acq_FormaterLigne(&m, ligne, sizeof(ligne)); /* membre 2 */
        Logger_Ecrire(ligne, &m.date);             /* membre 3 */
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println(F("=== Worldwide Weather Watcher ==="));

    /* 1) Les outils */
    GroveRGB_Init();
    GroveButton_Init();
    GroveLight_Init();
    GroveGPS_Init();
    if (!GroveRTC_Init())    Erreur_Signaler(ERR_RTC);
    if (!GroveBME280_Init()) Erreur_Signaler(ERR_CAPTEUR_ACCES);

    /* 2) Les modules du groupe */
    Config_Charger();      /* membre 4 : reglages depuis l'EEPROM */
    Logger_Init();         /* membre 3 : carte SD                 */
    Console_Init();        /* membre 4 : console serie            */
    Modes_Init();          /* membre 1 : standard ou configuration */
}

void loop()
{
    /* 1) et 2) : boutons puis changement de mode eventuel */
    Buttons_Update();
    Modes_Update();

    /* 3) Le travail du mode actuel */
    uint32_t intervalle = Config_Get()->log_interval * 60000UL;   /* minutes -> ms */

    switch (Modes_Get()) {

        case MODE_STANDARD:
            if (Acq_EstLHeure(intervalle))     Faire_Mesure(false, false);
            break;

        case MODE_ECO:
            /* PDF : intervalle x 2 (et GPS 1 fois sur 2, gere par le membre 2) */
            if (Acq_EstLHeure(intervalle * 2)) Faire_Mesure(true, false);
            break;

        case MODE_MAINTENANCE:
            /* PDF : pas d'ecriture SD, donnees consultees sur la console */
            if (Acq_EstLHeure(MAINTENANCE_INTERVALLE_MS)) Faire_Mesure(false, true);
            break;

        case MODE_CONFIG: {
            /* PDF : pas de mesures, on attend les commandes tapees */
            char commande[40];
            if (Console_LireLigne(commande, sizeof(commande))) Console_Executer(commande);
            break;
        }
    }

    /* 4) La LED (couleur du mode ou clignotement d'erreur) */
    Led_Update();
}