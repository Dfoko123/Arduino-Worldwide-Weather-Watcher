#include <Arduino.h>
#include "app_modes.h"
#include "app_buttons.h"
#include "app_led.h"

/* Duree sans activite avant de quitter le mode configuration (PDF : 30 min).
   Pour TESTER, on peut mettre temporairement 30000UL (30 s). */
#define CONFIG_TIMEOUT_MS   (30UL * 60UL * 1000UL)

/* ---------------------------------------------------------------------------
 * Fonctions des AUTRES membres utilisees ici.
 * Les membres 3 et 4 ne les ont pas encore ecrites : on fournit des versions
 * "de secours" marquees __attribute__((weak)) ("faibles").
 * Des que le vrai code existe dans app_logger.cpp / app_console.cpp,
 * le compilateur utilise AUTOMATIQUEMENT le vrai, sans rien changer ici.
 * ------------------------------------------------------------------------- */
void          Logger_Suspendre(void);
bool          Logger_Reprendre(void);
unsigned long Console_DerniereActivite(void);

__attribute__((weak)) void          Logger_Suspendre(void)         { }
__attribute__((weak)) bool          Logger_Reprendre(void)         { return true; }
__attribute__((weak)) unsigned long Console_DerniereActivite(void) { return 0; }

/* --------------------------------------------------------------------------- */

static Mode mode_actuel    = MODE_STANDARD;
static Mode mode_precedent = MODE_STANDARD;

/* Affiche le nom du mode sur la console */
static void Afficher_NomMode(Mode m)
{
    Serial.print(F("Mode : "));
    switch (m) {
        case MODE_STANDARD:    Serial.println(F("STANDARD"));      break;
        case MODE_CONFIG:      Serial.println(F("CONFIGURATION")); break;
        case MODE_MAINTENANCE: Serial.println(F("MAINTENANCE"));   break;
        case MODE_ECO:         Serial.println(F("ECONOMIQUE"));    break;
    }
}

void Modes_Init(void)
{
    /* PDF : demarrage avec le bouton rouge appuye = mode configuration */
    if (Buttons_RougeAuDemarrage()) mode_actuel = MODE_CONFIG;
    else                            mode_actuel = MODE_STANDARD;
    mode_precedent = mode_actuel;

    Led_AfficherMode(mode_actuel);
    Afficher_NomMode(mode_actuel);
}

void Modes_Changer(Mode m)
{
    if (m == mode_actuel) return;

    /* Sortie de maintenance : on peut a nouveau utiliser la carte SD */
    if (mode_actuel == MODE_MAINTENANCE) Logger_Reprendre();
    /* Entree en maintenance : on libere la carte SD (retrait sans risque) */
    if (m == MODE_MAINTENANCE) Logger_Suspendre();

    mode_precedent = mode_actuel;
    mode_actuel    = m;

    Led_AfficherMode(m);
    Afficher_NomMode(m);
}

void Modes_Update(void)
{
    /* On lit les DEUX drapeaux a chaque tour, pour qu'un appui inutile
       (ex : vert long en maintenance) ne reste pas "en attente". */
    bool rouge_long = Buttons_RougeLong();
    bool vert_long  = Buttons_VertLong();

    switch (mode_actuel) {

        case MODE_STANDARD:
            if (rouge_long)     Modes_Changer(MODE_MAINTENANCE);
            else if (vert_long) Modes_Changer(MODE_ECO);
            break;

        case MODE_ECO:
            /* Choix du groupe (a confirmer avec le tuteur) : rouge 5 s -> standard */
            if (rouge_long) Modes_Changer(MODE_STANDARD);
            break;

        case MODE_MAINTENANCE:
            /* PDF : rouge 5 s -> retour au mode PRECEDENT (standard ou eco) */
            if (rouge_long) Modes_Changer(mode_precedent);
            break;

        case MODE_CONFIG:
            /* Choix du groupe : rouge 5 s -> on quitte tout de suite la configuration */
            if (rouge_long) {
                Modes_Changer(MODE_STANDARD);
            }
            /* PDF : 30 min sans activite -> retour automatique au mode standard */
            else if (millis() - Console_DerniereActivite() >= CONFIG_TIMEOUT_MS) {
                Modes_Changer(MODE_STANDARD);
            }
            break;
    }
}

Mode Modes_Get(void)
{
    return mode_actuel;
}