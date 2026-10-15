/* LED RGB chainable Grove (P9813) - version Arduino.
   Meme principe que le driver STM32 du prof : on "pousse" les bits un par un
   sur la broche DATA en donnant un coup d'horloge sur CLK a chaque bit. */
#include <Arduino.h>
#include "grove_rgb.h"
#include "board_pins.h"

static void GroveRGB_ClockPulse(void)
{
    digitalWrite(RGB_CLK_PIN, LOW);
    delayMicroseconds(20);
    digitalWrite(RGB_CLK_PIN, HIGH);
    delayMicroseconds(20);
}

static void GroveRGB_SendByte(uint8_t b)
{
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(RGB_DATA_PIN, (b & 0x80) ? HIGH : LOW);  /* bit de poids fort d'abord */
        GroveRGB_ClockPulse();
        b <<= 1;
    }
}

static void GroveRGB_SendColor(uint8_t r, uint8_t g, uint8_t b)
{
    /* Octet de controle du P9813 : 2 bits a 1 puis les bits 7 et 6 de B, G, R inverses */
    uint8_t prefix = 0xC0;
    if ((b & 0x80) == 0) prefix |= 0x20;
    if ((b & 0x40) == 0) prefix |= 0x10;
    if ((g & 0x80) == 0) prefix |= 0x08;
    if ((g & 0x40) == 0) prefix |= 0x04;
    if ((r & 0x80) == 0) prefix |= 0x02;
    if ((r & 0x40) == 0) prefix |= 0x01;

    GroveRGB_SendByte(prefix);
    GroveRGB_SendByte(b);
    GroveRGB_SendByte(g);
    GroveRGB_SendByte(r);
}

void GroveRGB_Init(void)
{
    pinMode(RGB_CLK_PIN, OUTPUT);
    pinMode(RGB_DATA_PIN, OUTPUT);
    GroveRGB_Off();
}

void GroveRGB_SetColor(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint8_t i = 0; i < 4; i++) GroveRGB_SendByte(0x00);   /* debut de trame */
    GroveRGB_SendColor(r, g, b);
    for (uint8_t i = 0; i < 4; i++) GroveRGB_SendByte(0x00);   /* fin de trame   */
}

void GroveRGB_Off(void)
{
    GroveRGB_SetColor(0, 0, 0);
}
