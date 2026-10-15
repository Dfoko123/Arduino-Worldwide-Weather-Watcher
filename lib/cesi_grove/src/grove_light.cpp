/* Capteur de lumiere Grove - version Arduino (valeur 0 a 1023). */
#include <Arduino.h>
#include "grove_light.h"
#include "board_pins.h"

void GroveLight_Init(void)
{
    pinMode(LIGHT_PIN, INPUT);
}

uint16_t GroveLight_ReadRaw(void)
{
    return (uint16_t)analogRead(LIGHT_PIN);
}
