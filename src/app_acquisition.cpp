// Module d'acquisition des mesures météorologiques

// Interface du module d'acquisition
#include "app_acquisition.h"

// Pilote du capteur de lumière
#include "grove_light.h"

// Pilote du GPS
#include "grove_gps_air530z.h"

// Pilote du capteur BME280
#include "grove_bme280.h"

// Gestion des erreurs du projet
#include "app_errors.h"

// Bibliothèques Arduino et formatage du texte
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


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

void Acq_LireTout(Mesure *m, bool mode_eco)
{
    // Vérifier que le pointeur reçu est valide.
    if (m == NULL)
    {
        return;
    }

    // --------------------------------------------------------
    // A. Lecture du capteur BME280
    // --------------------------------------------------------

    Env_Data environnement;

    // Essayer de lire température, humidité et pression.
    if (GroveBME280_ReadEnvironment(&environnement))
    {
        // Copier les valeurs dans la structure Mesure.
        m->env.temperature_c = environnement.temperature_c;
        m->env.humidity_percent = environnement.humidity_percent;
        m->env.pressure_hpa = environnement.pressure_hpa;

        // Indiquer que les mesures ont été obtenues.
        m->etat_temp.valide = true;
        m->etat_hygr.valide = true;
        m->etat_pression.valide = true;
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
    // B. Lecture du capteur de lumière
    // --------------------------------------------------------

    // Le capteur n'est lu que s'il est activé.
    // Le paramètre lumin sera vérifié par la logique de
    // configuration du projet. Cette fonction ne reçoit
    // actuellement pas de Parametres en argument.

    // Lire la valeur brute du capteur.
    m->lumin = GroveLight_ReadRaw();

    // La lecture analogique seule ne permet pas de détecter
    // de façon fiable si le capteur est débranché.
    m->etat_lumin.valide = true;


    // --------------------------------------------------------
    // C. Lecture du GPS
    // --------------------------------------------------------

    // Ne lire le GPS que si le mode de fonctionnement l'autorise.
    if (Acq_DoitLireGPS(mode_eco))
    {
        // Le GPS peut attendre une trame jusqu'au timeout.
        // La valeur du timeout doit être définie par l'équipe.
        const unsigned int TIMEOUT_MS = 3000;

        bool lecture_gps = GroveGPS_ReadGGA(&m->gps, TIMEOUT_MS);

        // La fonction peut recevoir une trame sans position valide.
        if (lecture_gps && m->gps.valid)
        {
            m->etat_gps.valide = true;
        }
        else
        {
            m->etat_gps.valide = false;
        }
    }

    // Si le GPS n'a pas été lu en mode économique,
    // conserver la dernière position disponible.
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
            Erreur_Signaler(ERR_CAPTEUR_INCOH);
        }
    }


    // --------------------------------------------------------
    // B. Vérification de l'humidité
    // --------------------------------------------------------

    if (m->etat_hygr.valide)
    {
        // Les champs hygr_mint et hygr_maxt sont utilisés ici
        // comme limites d'humidité, conformément à leur rôle
        // supposé dans la configuration du projet.
        if (m->env.humidity_percent < p->hygr_mint ||
            m->env.humidity_percent > p->hygr_maxt)
        {
            m->etat_hygr.valide = false;
            Erreur_Signaler(ERR_CAPTEUR_INCOH);
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
            Erreur_Signaler(ERR_CAPTEUR_INCOH);
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
            Erreur_Signaler(ERR_CAPTEUR_INCOH);
        }
    }
}


// ============================================================
// 5. FORMATER LES MESURES EN UNE LIGNE DE TEXTE
// ============================================================

// Ajouter une valeur numérique ou "NA" à la ligne.
static void AjouterFloat(
    char *ligne,
    size_t taille,
    const char *format,
    bool valide,
    float valeur)
{
    // Refuser les valeurs indisponibles ou non finies.
    if (!valide || !isfinite(valeur))
    {
        strlcat(ligne, "NA;", taille);
        return;
    }

    // Calculer la place encore disponible.
    size_t longueur = strlen(ligne);

    // Vérifier qu'il reste de la place.
    if (longueur < taille)
    {
        // Ajouter la valeur à la suite de la ligne.
        snprintf(
            ligne + longueur,
            taille - longueur,
            format,
            valeur
        );

        // Ajouter le séparateur.
        strlcat(ligne, ";", taille);
    }
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

    // Exemple : 2026-10-19;09:30:00;
    snprintf(
        ligne,
        taille,
        "%04u-%02u-%02u;%02u:%02u:%02u;",
        m->date.year,
        m->date.month,
        m->date.day,
        m->date.hours,
        m->date.minutes,
        m->date.seconds
    );


    // --------------------------------------------------------
    // B. Température
    // --------------------------------------------------------

    AjouterFloat(
        ligne,
        taille,
        "%.2f",
        m->etat_temp.valide,
        m->env.temperature_c
    );


    // --------------------------------------------------------
    // C. Humidité
    // --------------------------------------------------------

    AjouterFloat(
        ligne,
        taille,
        "%.2f",
        m->etat_hygr.valide,
        m->env.humidity_percent
    );


    // --------------------------------------------------------
    // D. Pression
    // --------------------------------------------------------

    AjouterFloat(
        ligne,
        taille,
        "%.2f",
        m->etat_pression.valide,
        m->env.pressure_hpa
    );


    // --------------------------------------------------------
    // E. Luminosité
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
    // F. Latitude et longitude
    // --------------------------------------------------------

    if (m->etat_gps.valide && m->gps.valid)
    {
        AjouterFloat(
            ligne,
            taille,
            "%.6f",
            true,
            (float)m->gps.latitude
        );

        AjouterFloat(
            ligne,
            taille,
            "%.6f",
            true,
            (float)m->gps.longitude
        );
    }
    else
    {
        // Si la position n'est pas disponible, écrire NA.
        strlcat(ligne, "NA;NA;", taille);
    }


    // --------------------------------------------------------
    // G. Terminer la ligne
    // --------------------------------------------------------

    // Retirer le dernier point-virgule si la chaîne n'est pas vide.
    size_t longueur = strlen(ligne);

    if (longueur > 0 && ligne[longueur - 1] == ';')
    {
        ligne[longueur - 1] = '\0';
    }
}