#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <stdint.h>
#include <stdbool.h>

/* Carte SD (bibliotheque Arduino SD).
   DIFFERENCES avec la version STM32 :
   - l'ecriture fonctionne VRAIMENT (plus besoin de FatFs) ;
   - les noms de fichiers sont limites au format court "8.3" :
     8 caracteres max avant le point, 3 apres. "261015_0.LOG" est OK,
     mais "261015_10.LOG" (9 caracteres) sera REFUSE -> revisions 0 a 9 seulement. */

bool     SDLogger_Init(void);                                  /* demarrer la carte        */
void     SDLogger_End(void);                                   /* liberer la carte (retrait) */
bool     SDLogger_WriteLine(const char *filename, const char *line); /* ajouter une ligne */
bool     SDLogger_FileExists(const char *filename);
uint32_t SDLogger_FileSize(const char *filename);              /* 0 si absent              */
bool     SDLogger_CopyFile(const char *src, const char *dst);
bool     SDLogger_Truncate(const char *filename);              /* vider le fichier         */

#endif
