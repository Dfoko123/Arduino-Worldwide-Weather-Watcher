#include <Arduino.h>
#include "app_modes.h"
#include "app_buttons.h"
#include "app_led.h"
/* En phase 2, quand les membres 3 et 4 auront cree leurs fichiers, ajouter :
   #include "app_logger.h"     (Logger_Suspendre / Logger_Reprendre)
   #include "app_console.h"    (Console_DerniereActivite)                */

static Mode mode_actuel    = MODE_STANDARD;
static Mode mode_precedent = MODE_STANDARD;

void Modes_Init(void)
{
                             /* BOUCHON : vrai code en phase 2 */
}

void Modes_Update(void)
{
                             /* BOUCHON */
}

void Modes_Changer(Mode m)
{
    mode_precedent = mode_actuel;   /* BOUCHON minimal */
    mode_actuel = m;
}

Mode Modes_Get(void)
{
    return mode_actuel;
}