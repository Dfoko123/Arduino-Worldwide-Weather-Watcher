#ifndef GROVE_BUTTON_H
#define GROVE_BUTTON_H

#include <stdbool.h>

/* Double bouton Grove. Memes fonctions que la version STM32.
   Bouton 1 = rouge, bouton 2 = vert (a verifier sur le module). */
void GroveButton_Init(void);
bool GroveButton1_IsPressed(void);   /* true = appuye A CET INSTANT */
bool GroveButton2_IsPressed(void);

#endif
