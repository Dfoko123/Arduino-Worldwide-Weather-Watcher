#ifndef APP_LED_H
#define APP_LED_H

#include <Arduino.h>
#include "cesi_types.h"

/* Allume la couleur FIXE du mode m :
   standard = vert, configuration = jaune, maintenance = orange, economique = bleu. */
void Led_AfficherMode(Mode m);

/* Affiche l'erreur e en clignotant a 1 Hz (rouge + une autre couleur, voir PDF).
   t_ms = l'instant actuel (millis()), qui permet de savoir quelle couleur afficher. */
void Led_AfficherErreur(CodeErreur e, uint32_t t_ms);

/* A appeler a CHAQUE tour de loop() :
   s'il y a une erreur -> la fait clignoter, sinon -> affiche la couleur du mode.
   Ne bloque jamais (pas de delay). */
void Led_Update(void);

#endif