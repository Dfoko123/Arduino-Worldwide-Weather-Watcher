#ifndef APP_MODES_H
#define APP_MODES_H

#include <Arduino.h>
#include "cesi_types.h"

/* Au demarrage : bouton rouge appuye -> mode CONFIGURATION, sinon -> STANDARD. */
void Modes_Init(void);

/* A appeler a CHAQUE tour de loop() : regarde les appuis longs (et le delai
   de 30 min en configuration) et change de mode selon les regles du PDF. */
void Modes_Update(void);

/* Change de mode : retient le mode precedent, enregistre le nouveau,
   change la LED et l'affiche sur la console.
   (En phase 2 : suspend / reprend la carte SD pour la maintenance.) */
void Modes_Changer(Mode m);

/* Renvoie le mode actuel. */
Mode Modes_Get(void);

#endif