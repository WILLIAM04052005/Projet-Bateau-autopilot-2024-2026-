# Programmes de test

Chaque dossier est un sketch Arduino autonome : le fichier `.ino` porte le nom du dossier, il s'ouvre donc directement dans Arduino IDE ou VS Code. Chaque fichier commence par un en-tête qui donne son rôle, la carte utilisée et sa source.

> **Origine du code.** Une partie des tests ne se trouvait que dans les rapports (PDF ou captures d'écran). Ils ont été recopiés puis réindentés. Quand une capture était coupée, la partie manquante a été complétée, et c'est signalé dans l'en-tête ou par un commentaire `// --- complété`.

## Année 1 (2024-2025) : conception et banc de commande · Arduino UNO

| # | Test | Domaine | Ce qui est validé |
|---|---|---|---|
| 01 | [`pont_h_double_moteur`](annee-1_2024-2025/01_pont_h_double_moteur) | EEE | Câblage du pont en H : avant, arrière, stop |
| 02 | [`commande_moteur_lcd_capteurs`](annee-1_2024-2025/02_commande_moteur_lcd_capteurs) | EEE | Vitesse au potentiomètre, courant (ACS712), tr/min, LCD |
| 03 | [`mesure_courant_vitesse_lcd_i2c`](annee-1_2024-2025/03_mesure_courant_vitesse_lcd_i2c) | EEE | Idem avec calibration du zéro, moyenne de 50 mesures, bouton de sens, LCD I2C |
| 04 | [`regulation_vitesse_P`](annee-1_2024-2025/04_regulation_vitesse_P) | EEE | Asservissement en vitesse (correcteur P, codeur 4500 points/tour) |
| 05 | [`regulation_position_P`](annee-1_2024-2025/05_regulation_position_P) | EEE | Asservissement en position angulaire (correcteur P) |
| 06 | [`mia_servo_gps`](annee-1_2024-2025/06_mia_servo_gps) | MIA | Safran piloté au potentiomètre, positions GPS (TinyGPS++) |
| 07 | [`mia_autopilote_hmc5883l`](annee-1_2024-2025/07_mia_autopilote_hmc5883l) | MIA | Premier autopilote : correction proportionnelle du cap |
| 08 | [`maquette_autopilote`](annee-1_2024-2025/08_maquette_autopilote) | EEE/MIA | Programme de la maquette (⚠ incomplet tel que récupéré) |

## Année 2 (2025-2026) : prototype · ESP32

**Banc de test individuel** (rapport *Test bench — MIA & EEE*) : chaque composant est validé seul avant l'intégration.

| # | Test | Résultat |
|---|---|---|
| 01 | [`banc_servo`](annee-2_2025-2026/01_banc_servo) | Débattement 0-180° validé |
| 02 | [`banc_pont_h_bts7960`](annee-2_2025-2026/02_banc_pont_h_bts7960) | Marche avant et arrière, vitesse réglable |
| 03 | [`banc_magnetometre_bmm150`](annee-2_2025-2026/03_banc_magnetometre_bmm150) | Cap mesuré, lectures encore instables |
| 04 | [`banc_capteur_vitesse`](annee-2_2025-2026/04_banc_capteur_vitesse) | tr/min mesurés |
| 05 | [`banc_capteur_courant`](annee-2_2025-2026/05_banc_capteur_courant) | Courant moteur mesuré |

**Prototypes de communication et d'intégration :**

| # | Programme | Carte | Étape |
|---|---|---|---|
| 06 | [`uno_moteur_courant_vitesse`](annee-2_2025-2026/06_uno_moteur_courant_vitesse) | UNO WiFi Rev2 | Reprise de la commande moteur de l'année 1 |
| 07 | [`uno_servo_potentiometre`](annee-2_2025-2026/07_uno_servo_potentiometre) | UNO WiFi Rev2 | Test du servo |
| 08 | [`uno_wifi_commandes`](annee-2_2025-2026/08_uno_wifi_commandes) | UNO WiFi Rev2 | Commandes HTTP depuis l'application MIT App Inventor |
| 09 | [`uno_wifi_bts7960_boussole`](annee-2_2025-2026/09_uno_wifi_bts7960_boussole) | UNO WiFi Rev2 | Tout le système sur UNO (BTS7960, QMC5883L, capteurs) |
| 10 | [`esp32_moteur_page_web`](annee-2_2025-2026/10_esp32_moteur_page_web) | **ESP32** | Passage à l'ESP32 : serveur web et page de commande (HTML séparé dans `web/`) |

La suite (versions v1 → v3 puis code final modulaire) est dans [`firmware/`](../firmware).
