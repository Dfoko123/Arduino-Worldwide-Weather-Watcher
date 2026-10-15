/* Carte SD - version Arduino (bibliotheque SD).
   On ferme TOUJOURS le fichier apres chaque operation : si la carte est retiree
   ou l'alimentation coupee, un fichier reste ouvert peut etre abime. */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "sd_logger.h"
#include "board_pins.h"

bool SDLogger_Init(void)
{
    return SD.begin(SD_CS_PIN);
}

void SDLogger_End(void)
{
    SD.end();
}

bool SDLogger_WriteLine(const char *filename, const char *line)
{
    File f = SD.open(filename, FILE_WRITE);   /* FILE_WRITE = ajout a la fin (cree si absent) */
    if (!f) return false;
    size_t ecrit = f.println(line);
    f.close();
    return ecrit > 0;
}

bool SDLogger_FileExists(const char *filename)
{
    return SD.exists(filename);
}

uint32_t SDLogger_FileSize(const char *filename)
{
    File f = SD.open(filename, FILE_READ);
    if (!f) return 0;
    uint32_t taille = f.size();
    f.close();
    return taille;
}

bool SDLogger_CopyFile(const char *src, const char *dst)
{
    File a = SD.open(src, FILE_READ);
    if (!a) return false;
    if (SD.exists(dst)) SD.remove(dst);
    File b = SD.open(dst, FILE_WRITE);
    if (!b) { a.close(); return false; }

    uint8_t tampon[32];                        /* petit tampon : la RAM est limitee */
    int n;
    bool ok = true;
    while ((n = a.read(tampon, sizeof(tampon))) > 0) {
        if (b.write(tampon, n) != (size_t)n) { ok = false; break; }
    }
    a.close();
    b.close();
    return ok;
}

bool SDLogger_Truncate(const char *filename)
{
    if (SD.exists(filename) && !SD.remove(filename)) return false;
    File f = SD.open(filename, FILE_WRITE);    /* recree un fichier vide */
    if (!f) return false;
    f.close();
    return true;
}
