#include <Arduino.h>
#include "app_led.h"
#include "app_errors.h"
#include "app_modes.h"
#include "grove_rgb.h"

void Led_AfficherMode(Mode m)
{
    (void)m;                 /* BOUCHON : vrai code en phase 2 */
}

void Led_AfficherErreur(CodeErreur e, uint32_t t_ms)
{
    (void)e;                 /* BOUCHON */
    (void)t_ms;
}

void Led_Update(void)
{
                             /* BOUCHON */
}