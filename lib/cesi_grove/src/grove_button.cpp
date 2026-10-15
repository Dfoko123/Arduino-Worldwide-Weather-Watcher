/* Double bouton Grove - version Arduino. */
#include <Arduino.h>
#include "grove_button.h"
#include "board_pins.h"

void GroveButton_Init(void)
{
    /* Si le bouton est actif a l'etat BAS, on active la resistance interne
       de tirage vers le haut (repos = HIGH). Sinon entree simple. */
    uint8_t mode = (BUTTON_PRESSED_LEVEL == LOW) ? INPUT_PULLUP : INPUT;
    pinMode(BUTTON1_PIN, mode);
    pinMode(BUTTON2_PIN, mode);
}

bool GroveButton1_IsPressed(void)
{
    return digitalRead(BUTTON1_PIN) == BUTTON_PRESSED_LEVEL;
}

bool GroveButton2_IsPressed(void)
{
    return digitalRead(BUTTON2_PIN) == BUTTON_PRESSED_LEVEL;
}
