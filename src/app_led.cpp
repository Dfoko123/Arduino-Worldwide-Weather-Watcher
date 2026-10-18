#include <Arduino.h>
#include "app_led.h"
#include "app_errors.h"
#include "app_modes.h"
#include "grove_rgb.h"

/* Derniere couleur envoyee a la LED : on ne renvoie une couleur
   que si elle a change (envoyer une couleur prend environ 4 ms). */
static uint8_t dernier_r = 0, dernier_g = 0, dernier_b = 0;
static bool    deja_allumee = false;

/* Allume la LED, seulement si la couleur est differente de la precedente. */
static void Led_Allumer(uint8_t r, uint8_t g, uint8_t b)
{
    if (deja_allumee && r == dernier_r && g == dernier_g && b == dernier_b) return;
    GroveRGB_SetColor(r, g, b);
    dernier_r = r;  dernier_g = g;  dernier_b = b;
    deja_allumee = true;
}

void Led_AfficherMode(Mode m)
{
    switch (m) {
        case MODE_STANDARD:    Led_Allumer(0,   255, 0);   break;   /* vert   */
        case MODE_CONFIG:      Led_Allumer(255, 255, 0);   break;   /* jaune  */
        case MODE_MAINTENANCE: Led_Allumer(255, 80,  0);   break;   /* orange */
        case MODE_ECO:         Led_Allumer(0,   0,   255); break;   /* bleu   */
    }
}

void Led_AfficherErreur(CodeErreur e, uint32_t t_ms)
{
    /* 1) Choisir la 2e couleur et la duree du rouge selon l'erreur (PDF) */
    uint8_t  r2 = 0, g2 = 0, b2 = 0;    /* 2e couleur                        */
    uint16_t seuil = 500;               /* duree du rouge : 500 ms = 1:1     */

    switch (e) {
        case ERR_RTC:           b2 = 255;                          break;  /* rouge / bleu         */
        case ERR_GPS:           r2 = 255; g2 = 255;                break;  /* rouge / jaune        */
        case ERR_CAPTEUR_ACCES: g2 = 255;                          break;  /* rouge / vert         */
        case ERR_CAPTEUR_INCOH: g2 = 255;            seuil = 333;  break;  /* rouge / vert long    */
        case ERR_SD_PLEINE:     r2 = 255; g2 = 255; b2 = 255;      break;  /* rouge / blanc        */
        case ERR_SD_ACCES:      r2 = 255; g2 = 255; b2 = 255; seuil = 333; break; /* rouge / blanc long */
        default: return;
    }

    /* 2) Ou en est-on dans la seconde en cours ? (0 a 999 ms) */
    uint16_t position = (uint16_t)(t_ms % 1000UL);

    /* 3) Debut de la seconde = rouge, fin de la seconde = 2e couleur */
    if (position < seuil) Led_Allumer(255, 0, 0);
    else                  Led_Allumer(r2, g2, b2);
}

void Led_Update(void)
{
    CodeErreur e = Erreur_LaPlusGrave();
    if (e == ERR_AUCUNE) Led_AfficherMode(Modes_Get());
    else                 Led_AfficherErreur(e, millis());
}