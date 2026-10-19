#ifndef APP_ACQUISITION_H
#define APP_ACQUISITION_H

// Bibliothèques pour les types bool, uint8_t et uint32_t
#include <stdbool.h>
#include <stdint.h>

// Structures Mesure et Parametres du projet
#include "cesi_types.h"

// Détermine s'il est temps de réaliser une nouvelle mesure
bool Acq_EstLHeure(uint32_t intervalle_ms);

// Transforme les mesures en une ligne de texte
void Acq_FormaterLigne(const Mesure *m, char *ligne, uint8_t taille);

// Vérifie si les mesures respectent les seuils configurés
void Acq_VerifierSeuils(Mesure *m, const Parametres *p);

// Détermine si le GPS doit être lu en mode économique
bool Acq_DoitLireGPS(bool mode_eco);

// Lit les données des capteurs
void Acq_LireTout(Mesure *m, bool mode_eco);

#endif