#ifndef GROVE_RTC_DS1307_H
#define GROVE_RTC_DS1307_H

#include <stdbool.h>
#include "cesi_types.h"

/* Horloge RTC DS1307 (I2C). Memes fonctions que la version STM32. */
bool GroveRTC_Init(void);                              /* true si l'horloge repond */
bool GroveRTC_GetDateTime(RTC_DateTime *dt);           /* lit la date et l'heure  */
bool GroveRTC_SetDateTime(const RTC_DateTime *dt);     /* regle la date et l'heure */

#endif
