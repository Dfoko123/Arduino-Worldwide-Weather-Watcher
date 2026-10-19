#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "cesi_types.h"

// Prototypes stricts exigés par l'équipe
void Config_Charger(void);
void Config_Sauver(void);
void Config_Reset(void);
const Parametres Config_Get(void);
bool Config_Modifier(const char* nom, long valeur);
void Config_Afficher(void);

#endif // APP_CONFIG_H