#ifndef BOARD_PINS_H
#define BOARD_PINS_H

/*
 * Brochage pour ARDUINO UNO + Grove Base Shield + shield SD Seeed.
 * Les ports Grove sont ceux du Base Shield (D2, D3 ... A0, A1 ... I2C).
 * Un port Grove "Dx" utilise 2 broches : x (fil jaune) et x+1 (fil blanc).
 *
 * Si vous branchez un module sur un AUTRE port, c'est ICI (et seulement ici)
 * qu'il faut changer le numero de broche.
 */

/* ---- LED RGB chainable (P9813) : port Grove D6 ---- */
#define RGB_CLK_PIN        6     /* fil jaune */
#define RGB_DATA_PIN       7     /* fil blanc */

/* ---- Double bouton Grove : port Grove D2 ---- */
#define BUTTON1_PIN        2     /* bouton ROUGE (a verifier sur le module) */
#define BUTTON2_PIN        3     /* bouton VERT  (a verifier sur le module) */
/* Niveau lu quand on APPUIE. Si les boutons semblent inverses
   (appuye/relache a l'envers), remplacer LOW par HIGH. */
#define BUTTON_PRESSED_LEVEL  LOW

/* ---- Capteur de lumiere : port Grove A2 ---- */
#define LIGHT_PIN          A2

/* ---- GPS Air530Z (liaison serie logicielle) : port Grove A0 ---- */
   #define GPS_RX_PIN         8
   #define GPS_TX_PIN         9    /* l'Arduino ENVOIE sur A1 (fil blanc) */
#define GPS_BAUDRATE       9600
/* Si le GPS ne renvoie jamais rien : inverser A0 et A1 ci-dessus. */

/* ---- Horloge RTC DS1307 et capteur BME680 : port Grove I2C ---- */
/* (I2C = broches A4/SDA et A5/SCL, rien a definir ici) */
#define DS1307_I2C_ADDR    0x68

/* ---- Shield carte SD Seeed (SPI : broches 11, 12, 13) ---- */
#define SD_CS_PIN          4

#endif
