#include "app_config.h"
#include <string.h>
#include <stdio.h>

// On inclut le pilote I2C, et on force les déclarations au cas où
// elles manqueraient dans le fichier grove_rtc_ds1307.h d'origine.
extern "C" {
    #include "grove_rtc_ds1307.h"
    bool GroveRTC_WriteRAM(uint8_t offset, const uint8_t *data, uint8_t length);
    bool GroveRTC_ReadRAM(uint8_t offset, uint8_t *data, uint8_t length);
}

#define CONFIG_MAGIC_WORD 0xABCD

// La structure mémoire utilise bien le type "Parametres" de votre équipe
struct RTC_Storage_t {
    uint16_t magic_word;
    Parametres config; 
};

// La vraie variable locale
static Parametres current_config;

// On met Sauver() et Reset() en haut pour que Charger() puisse les lire
void Config_Sauver(void) {
    RTC_Storage_t storage;
    storage.magic_word = CONFIG_MAGIC_WORD;
    memcpy(&storage.config, &current_config, sizeof(Parametres));
    GroveRTC_WriteRAM(0, (uint8_t*)&storage, sizeof(RTC_Storage_t));
}

void Config_Reset(void) {
    // Les valeurs par défaut seront à ajouter ici
    Config_Sauver(); 
}

void Config_Charger(void) {
    RTC_Storage_t storage;
    if (GroveRTC_ReadRAM(0, (uint8_t*)&storage, sizeof(RTC_Storage_t))) {
        if (storage.magic_word == CONFIG_MAGIC_WORD) {
            memcpy(&current_config, &storage.config, sizeof(Parametres));
            return; // Succès
        }
    }
    Config_Reset(); // Échec ou 1er démarrage
}

const Parametres Config_Get(void) {
    return current_config;
}

bool Config_Modifier(const char* nom, long valeur) {
    // Logique de modification à remplir
    if (strcmp(nom, "LOG_INTERVAL") == 0) {
        return true;
    }
    return false; 
}

void Config_Afficher(void) {
    printf("--- Configuration Actuelle ---\r\n");
}