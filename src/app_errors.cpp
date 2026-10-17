#include <Arduino.h>
#include "app_errors.h"

/* Une case vrai/faux par erreur (true = erreur en cours).
   Toutes les cases valent false au demarrage. */
static bool erreurs_actives[ERR_NB];

void Erreur_Signaler(CodeErreur e)
{
    /* On refuse les codes invalides pour ne pas ecrire hors du tableau */
    if (e <= ERR_AUCUNE || e >= ERR_NB) return;
    erreurs_actives[e] = true;
}

void Erreur_Effacer(CodeErreur e)
{
    if (e <= ERR_AUCUNE || e >= ERR_NB) return;
    erreurs_actives[e] = false;
}

CodeErreur Erreur_LaPlusGrave(void)
{
    /* On parcourt de la plus grave (ERR_RTC) a la moins grave (ERR_SD_ACCES) :
       la premiere case a true est l'erreur a afficher. */
    for (uint8_t i = ERR_RTC; i < ERR_NB; i++) {
        if (erreurs_actives[i]) return (CodeErreur)i;
    }
    return ERR_AUCUNE;
}