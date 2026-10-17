#ifndef APP_ERRORS_H
#define APP_ERRORS_H

#include <Arduino.h>
#include "cesi_types.h"

/* Note qu'une erreur est en cours (ex : carte SD retiree). */
void Erreur_Signaler(CodeErreur e);

/* Note que le probleme est regle (ex : carte SD remise). */
void Erreur_Effacer(CodeErreur e);

/* Renvoie l'erreur la plus grave en cours, ou ERR_AUCUNE s'il n'y en a pas.
   Ordre de gravite : celui de l'enum CodeErreur (ERR_RTC = la plus grave). */
CodeErreur Erreur_LaPlusGrave(void);

#endif