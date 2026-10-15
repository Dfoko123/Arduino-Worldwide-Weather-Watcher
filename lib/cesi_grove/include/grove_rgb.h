#ifndef GROVE_RGB_H
#define GROVE_RGB_H

#include <stdint.h>

/* LED RGB chainable Grove (puce P9813). Memes fonctions que la version STM32. */
void GroveRGB_Init(void);
void GroveRGB_SetColor(uint8_t r, uint8_t g, uint8_t b);   /* 0 a 255 */
void GroveRGB_Off(void);

#endif
