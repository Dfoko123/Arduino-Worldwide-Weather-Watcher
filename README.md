# Worldwide Weather Watcher : station météo embarquée (Arduino Uno)

Projet CESI A2 : station météo embarquée pour navires, sur **Arduino Uno** avec des modules **Grove**.

## Matériel
Arduino Uno, Grove Base Shield, shield carte SD Seeed, LED RGB chaînable (P9813), double bouton,
horloge RTC DS1307, capteur BME680 (température, humidité, pression), capteur de lumière, GPS Air530Z.
Le brochage est dans `include/board_pins.h`.

## Organisation du code
- `lib/cesi_grove/` : les **outils** (drivers), un par composant. Ne pas modifier sans prévenir le coordinateur.
- `include/` et `src/` : **notre programme** (fichiers `app_...`).
- `src/main.cpp` : `setup()` (démarrage) et `loop()` (boucle infinie).

## Compiler et téléverser
Dans VS Code avec PlatformIO : la coche ✓ (compiler), la flèche → (téléverser sur la carte),
la prise 🔌 (console série, 115200 bauds).

Le fonctionnement attendu est décrit dans le PDF « Modes de fonctionnement ».
La répartition du travail est dans l'onglet **Issues**.
