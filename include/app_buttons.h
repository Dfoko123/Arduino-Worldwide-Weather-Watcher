#ifndef APP_BUTTONS_H
#define APP_BUTTONS_H

#include <Arduino.h>

/* A appeler a CHAQUE tour de loop() : lit les 2 boutons, filtre les rebonds
   et mesure depuis combien de temps chacun est appuye. */
void Buttons_Update(void);

/* true si le bouton ROUGE etait appuye au moment de l'allumage. */
bool Buttons_RougeAuDemarrage(void);

/* true UNE SEULE FOIS quand le bouton ROUGE a ete tenu 5 secondes, puis false. */
bool Buttons_RougeLong(void);

/* true UNE SEULE FOIS quand le bouton VERT a ete tenu 5 secondes, puis false. */
bool Buttons_VertLong(void);

#endif