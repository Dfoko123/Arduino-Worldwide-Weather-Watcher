// Module d'acquisition des mesures météorologiques

// Interface du module d'acquisition
#include "app_acquisition.h"

// Pilote du capteur de lumière
#include "grove_light.h"

// Pilote du GPS
#include "grove_gps_air530z.h"

// Pilote du capteur BME280
#include "grove_bme280.h"

// Pilote de l'horloge RTC (date et heure de la mesure)
#include "grove_rtc_ds1307.h"

// Gestion des erreurs du projet
#include "app_errors.h"

// Bibliothèques Arduino et formatage du texte
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Réglages du projet (membre 4).
// Quand app_config.h existera, remplacer cette ligne par :
//     #include "app_config.h"
const Parametres *Config_Get(void);


// ============================================================
// 1. DÉTERMINER S'IL EST TEMPS DE FAIRE UNE MESURE
// ============================================================

// Cette variable mémorise l'heure de la dernière mesure.
static uint32_t derniere_mesure_ms = 0;


// Retourne true lorsque l'intervalle est écoulé.
bool Acq_EstLHeure(uint32_t intervalle_ms)
{
    // millis() indique le temps écoulé depuis le démarrage.
    uint32_t maintenant = millis();

    // Vérifier si le temps écoulé atteint l'intervalle demandé.
    if ((uint32_t)(maintenant - derniere_mesure_ms) >= intervalle_ms)
    {
        // Mémoriser le moment de cette mesure.
        derniere_mesure_ms = maintenant;

        return true;
    }

    return false;
}


// ============================================================
// 2. DÉTERMINER SI LE GPS DOIT ÊTRE LU
// ============================================================

// Cette variable compte les appels en mode économique.
static bool prochain_tour_gps = false;


bool Acq_DoitLireGPS(bool mode_eco)
{
    // En mode normal, le GPS est lu à chaque acquisition.
    if (!mode_eco)
    {
        prochain_tour_gps = false;
        return true;
    }

    // En mode économique, une acquisition sur deux est ignorée.
    prochain_tour_gps = !prochain_tour_gps;

    return prochain_tour_gps;
}


// ============================================================
// 3. LIRE LES CAPTEURS
// ============================================================

// Dernière position GPS lue (gardée pour le mode économique,
// quand le GPS n'est pas lu à cette mesure).
static GPS_Data dernier_gps;
static bool     dernier_gps_valide = false;


void Acq_LireTout(Mesure *m, bool mode_eco)
{
    // Vérifier que le pointeur reçu est valide.
    if (m == NULL)
    {
        return;
    }

    // Réglages actuels (capteurs actifs, timeout).
    const Parametres *p = Config_Get();

    // --------------------------------------------------------
    // A. Date et heure de la mesure (horloge RTC)
    // --------------------------------------------------------

    if (GroveRTC_GetDateTime(&m->date))
    {
        Erreur_Effacer(ERR_RTC);
    }
    else
    {
        // Date inconnue : l'année 0 sera affichée "NA".
        memset(&m->date, 0, sizeof(m->date));
        Erreur_Signaler(ERR_RTC);
    }


    // --------------------------------------------------------
    // B. Lecture du capteur BME280
    // --------------------------------------------------------

    Env_Data environnement;

    // Essayer de lire température, humidité et pression.
    if (GroveBME280_ReadEnvironment(&environnement))
    {
        // Copier les valeurs dans la structure Mesure.
        m->env = environnement;

        // Une mesure n'est valide que si le capteur est activé
        // dans les réglages (TEMP_AIR, HYGR, PRESSURE).
        m->etat_temp.valide     = (p->temp_air != 0);
        m->etat_hygr.valide     = (p->hygr != 0);
        m->etat_pression.valide = (p->pressure != 0);

        // Le capteur répond : effacer une éventuelle erreur.
        Erreur_Effacer(ERR_CAPTEUR_ACCES);
    }
    else
    {
        // Indiquer que les mesures ne sont pas disponibles.
        m->etat_temp.valide = false;
        m->etat_hygr.valide = false;
        m->etat_pression.valide = false;

        // Signaler une erreur d'accès au capteur.
        Erreur_Signaler(ERR_CAPTEUR_ACCES);
    }


    // --------------------------------------------------------
    // C. Lecture du capteur de lumière
    // --------------------------------------------------------

    if (p->lumin != 0)
    {
        // Lire la valeur brute du capteur (0 à 1023).
        m->lumin = GroveLight_ReadRaw();
        m->etat_lumin.valide = true;
    }
    else
    {
        // Capteur désactivé dans les réglages (LUMIN = 0).
        m->lumin = 0;
        m->etat_lumin.valide = false;
    }


    // --------------------------------------------------------
    // D. Lecture du GPS
    // --------------------------------------------------------

    // Ne lire le GPS que si le mode de fonctionnement l'autorise.
    if (Acq_DoitLireGPS(mode_eco))
    {
        // TIMEOUT est en secondes dans les réglages.
        uint32_t timeout_ms = (uint32_t)p->timeout * 1000UL;

        // (le driver attend un unsigned int : 60 s max = 60000, ça tient)
        bool lecture_gps = GroveGPS_ReadGGA(&dernier_gps, (unsigned int)timeout_ms);

        // La fonction peut recevoir une trame sans position valide.
        dernier_gps_valide = (lecture_gps && dernier_gps.valid);
    }

    // Mode économique sans lecture : on reprend la dernière position.
    m->gps = dernier_gps;
    m->etat_gps.valide = dernier_gps_valide;
}


// ============================================================
// 4. VÉRIFIER LES SEUILS DES MESURES
// ============================================================

void Acq_VerifierSeuils(Mesure *m, const Parametres *p)
{
    // Vérifier les pointeurs.
    if (m == NULL || p == NULL)
    {
        return;
    }

    bool incoherent = false;


    // --------------------------------------------------------
    // A. Vérification de la température
    // --------------------------------------------------------

    if (m->etat_temp.valide)
    {
        if (m->env.temperature_c < p->min_temp_air ||
            m->env.temperature_c > p->max_temp_air)
        {
            // La mesure est disponible mais hors des limites.
            m->etat_temp.valide = false;
            incoherent = true;
        }
    }


    // --------------------------------------------------------
    // B. Humidité
    // --------------------------------------------------------

    // HYGR_MINT et HYGR_MAXT sont des TEMPERATURES : en dehors de
    // cette plage de température, l'humidité n'est pas fiable.
    // On ne l'enregistre pas (NA), mais ce n'est PAS une erreur.
    if (m->etat_hygr.valide)
    {
        if (m->env.temperature_c < p->hygr_mint ||
            m->env.temperature_c > p->hygr_maxt)
        {
            m->etat_hygr.valide = false;
        }
    }


    // --------------------------------------------------------
    // C. Vérification de la pression
    // --------------------------------------------------------

    if (m->etat_pression.valide)
    {
        if (m->env.pressure_hpa < p->pressure_min ||
            m->env.pressure_hpa > p->pressure_max)
        {
            m->etat_pression.valide = false;
            incoherent = true;
        }
    }


    // --------------------------------------------------------
    // D. Vérification de la luminosité
    // --------------------------------------------------------

    if (m->etat_lumin.valide)
    {
        // La plage brute normale du capteur est de 0 à 1023.
        if (m->lumin > 1023)
        {
            m->etat_lumin.valide = false;
            incoherent = true;
        }
    }


    // --------------------------------------------------------
    // E. Signaler ou effacer l'erreur "données incohérentes"
    // --------------------------------------------------------

    if (incoherent)
    {
        Erreur_Signaler(ERR_CAPTEUR_INCOH);
    }
    else
    {
        Erreur_Effacer(ERR_CAPTEUR_INCOH);
    }
}


// ============================================================
// 5. FORMATER LES MESURES EN UNE LIGNE DE TEXTE
// ============================================================

// Ajouter une valeur à virgule ou "NA" à la ligne.
// ATTENTION : sur Arduino, snprintf ne sait pas afficher "%f"
// (il écrit "?"). On utilise donc dtostrf pour la conversion.
static void AjouterFloat(
    char *ligne,
    size_t taille,
    uint8_t decimales,
    bool valide,
    float valeur)
{
    // Refuser les valeurs indisponibles ou non finies.
    if (!valide || !isfinite(valeur))
    {
        strlcat(ligne, "NA;", taille);
        return;
    }

    // Convertir le nombre en texte (ex. 26.53).
    char texte[16];
    dtostrf(valeur, 1, decimales, texte);

    // Ajouter la valeur puis le séparateur.
    strlcat(ligne, texte, taille);
    strlcat(ligne, ";", taille);
}


// Préparer la ligne contenant les mesures.
void Acq_FormaterLigne(const Mesure *m, char *ligne, uint8_t taille)
{
    // Vérifier les arguments.
    if (m == NULL || ligne == NULL || taille == 0)
    {
        return;
    }

    // Commencer avec une chaîne vide.
    ligne[0] = '\0';


    // --------------------------------------------------------
    // A. Date et heure
    // --------------------------------------------------------

    if (m->date.year >= 2000)
    {
        // Exemple : 2026-10-19;09:30:00;
        snprintf(
            ligne,
            taille,
            "%04u-%02u-%02u;%02u:%02u:%02u;",
            (unsigned int)m->date.year,
            (unsigned int)m->date.month,
            (unsigned int)m->date.day,
            (unsigned int)m->date.hours,
            (unsigned int)m->date.minutes,
            (unsigned int)m->date.seconds
        );
    }
    else
    {
        // Horloge illisible.
        strlcat(ligne, "NA;NA;", taille);
    }


    // --------------------------------------------------------
    // B. Température, humidité, pression (2 décimales)
    // --------------------------------------------------------

    AjouterFloat(ligne, taille, 2, m->etat_temp.valide,     m->env.temperature_c);
    AjouterFloat(ligne, taille, 2, m->etat_hygr.valide,     m->env.humidity_percent);
    AjouterFloat(ligne, taille, 2, m->etat_pression.valide, m->env.pressure_hpa);


    // --------------------------------------------------------
    // C. Luminosité
    // --------------------------------------------------------

    if (m->etat_lumin.valide)
    {
        char valeur_lumin[8];

        snprintf(
            valeur_lumin,
            sizeof(valeur_lumin),
            "%u",
            (unsigned int)m->lumin
        );

        strlcat(ligne, valeur_lumin, taille);
        strlcat(ligne, ";", taille);
    }
    else
    {
        strlcat(ligne, "NA;", taille);
    }


    // --------------------------------------------------------
    // D. Latitude et longitude (6 décimales)
    // --------------------------------------------------------

    if (m->etat_gps.valide && m->gps.valid)
    {
        AjouterFloat(ligne, taille, 6, true, (float)m->gps.latitude);
        AjouterFloat(ligne, taille, 6, true, (float)m->gps.longitude);
    }
    else
    {
        // Si la position n'est pas disponible, écrire NA.
        strlcat(ligne, "NA;NA;", taille);
    }


    // --------------------------------------------------------
    // E. Terminer la ligne
    // --------------------------------------------------------

    // Retirer le dernier point-virgule si la chaîne n'est pas vide.
    size_t longueur = strlen(ligne);

    if (longueur > 0 && ligne[longueur - 1] == ';')
    {
        ligne[longueur - 1] = '\0';
    }
}