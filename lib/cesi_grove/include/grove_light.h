#ifndef GROVE_LIGHT_H
#define GROVE_LIGHT_H

#include <stdint.h>

/* Capteur de lumiere Grove (analogique).
   DIFFERENCE avec la STM32 : sur Arduino Uno, la valeur va de 0 a 1023
   (deja la bonne echelle pour LUMIN_LOW / LUMIN_HIGH, pas besoin de diviser). */
void     GroveLight_Init(void);
uint16_t GroveLight_ReadRaw(void);

#endif
