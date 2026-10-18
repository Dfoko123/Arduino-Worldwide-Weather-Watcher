#ifndef APP_ACQUISITION_H
#define APP_ACQUISITION_H

// Définit les types de données du projet
#include "cesi_types.h"

// Permet d'utiliser le type size_t
#include <stddef.h>

// Déclare le type bool
#include <stdbool.h>

// Initialise les capteurs
void AppAcquisition_Init(void);

// Lit les mesures des capteurs
void AppAcquisition_Lire(Mesure *mesure, const Parametres *parametres);

// Vérifie les mesures
void AppAcquisition_Verifier(Mesure *mesure, const Parametres *parametres);

// Prépare une ligne de texte contenant les mesures
bool AppAcquisition_Formater(const Mesure *mesure, char *buffer, size_t taille);

#endif