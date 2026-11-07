# Bateau autopiloté : de la conception à la navigation sur l'eau

> Projet intégrateur sur **deux années académiques (2024-2026)**, ICAM · Bachelor *General Engineering, Innovation & Technology*
> **2024-2025** : conception, maquette et prototype de la commande · **2025-2026** : construction et tests du bateau autopiloté sur une mare

En deux ans, nous sommes partis d'une étude théorique de coque et d'un banc de commande moteur sur Arduino pour arriver à un **bateau de loisir de 72 cm en fibre de verre**. Il est **piloté en Wi-Fi depuis un smartphone** et capable de **tenir un cap seul** grâce à un magnétomètre et à un régulateur PID embarqué sur ESP32.

<p align="center">
  <img src="media/photos/test-mare-bateau.jpg" height="300" alt="Bateau sur la mare">
  <img src="media/photos/test-bassin-application.jpg" height="300" alt="Test en bassin, pilotage depuis l'application">
  <img src="media/photos/equipe-p5-coque.jpg" height="300" alt="Équipe et coque">
</p>

---

## Sommaire

1. [En bref](#-en-bref)
2. [Mon rôle](#-mon-rôle)
3. [Année 1 (2024-2025) : conception et prototype de commande](#-année-1--2024-2025--conception-et-prototype-de-commande)
4. [Année 2 (2025-2026) : construction et tests du bateau autopiloté](#-année-2--2025-2026--construction-et-tests-du-bateau-autopiloté)
5. [Architecture logicielle finale](#-architecture-logicielle-finale)
6. [Démarrage rapide](#-démarrage-rapide)
7. [Organisation du dépôt](#-organisation-du-dépôt)
8. [Bilan et pistes d'amélioration](#-bilan-et-pistes-damélioration)

---

## En bref

| | Année 1 (2024-2025) | Année 2 (2025-2026) |
|---|---|---|
| **Objectif** | Concevoir le bateau et valider la commande moteur et la direction | Construire, câbler, programmer et tester le bateau sur l'eau |
| **Coque** | Monocoque 70 cm, étude ABS/ASA, flottabilité et stabilité | Composite fibre de verre / polyester, moulée sur un moule PLA imprimé en 3D · 72 × 22 cm · 6 kg |
| **Carte** | Arduino UNO | Arduino UNO WiFi Rev2, puis **ESP32** |
| **Propulsion** | Moteur DC 12 V, pont en H L298N | Moteur **775** 12 V / 100 W, pont en H **BTS7960** 43 A, hélice laiton 3 pales |
| **Capteurs** | ACS712, capteur IR de vitesse, potentiomètres, GPS, HMC5883L | **ACS758** 50 A, **LM393**, magnétomètre **BMM150** |
| **Commande** | LCD + potentiomètres, simulation SimulIDE | **Interface web** embarquée (smartphone), **autopilote PID** |
| **Énergie** | Alimentation de laboratoire 12 V | Batterie **LiFePO4 12 V 8 Ah** (≈ 58 min d'autonomie calculée) |
| **Résultat** | Banc de commande simulé et testé | Bateau testé en bassin puis sur une mare |

## Mon rôle

**William JUNIOR-ABO**
- **Année 1 :** responsable du domaine **EEE** (électronique et électrotechnique) : choix du moteur et des composants, blocs fonctionnels, chaînes d'information et d'énergie, simulation SimulIDE, banc de commande moteur avec capteurs de courant et de vitesse.
- **Année 2 :** responsable des domaines **EEE et MIA** (électronique et informatique) : dimensionnement électrique (moteur, batterie, câbles, protections), câblage, banc de test de chaque composant, **firmware ESP32**, **interface web** et **autopilote**.

| Année 1 : groupe 1 (B1) | Domaine |
|---|---|
| NYA'ABE NDO Andy Raphaëlle | Animatrice · MEO (gestion de projet) |
| **ABO William Junior** | **EEE** |
| MAKOSSO Jean Claude Mignon | MIA |
| NJOUGUEP WANDJI Emmanuelle | ESE (stabilité, impact environnemental) |
| LOUNDOU-LOUNDOU Yvon Ézéchiel | MME (matériaux et mécanique) |

| Année 2 : groupe P5 « Leisure Boat » (B2) |
|---|
| **William JUNIOR-ABO** (EEE & MIA) · Lucas LE GOFF · Théo SCHLOESSER-GRIMAUD · Lilou PICHON |

---

## Année 1 (2024-2025) : conception et prototype de commande

### Analyse et conception

Le cahier des charges demandait un bateau de compétition à l'échelle maquette. L'étude était répartie par domaine : **MME** (choix de l'ABS/ASA pour la coque, arbre en inox 316L, engrenages en POM, CAO), **ESE** (flottabilité d'Archimède, hauteur métacentrique, masse de 3,5 kg, immersion de 5,9 cm, analyse du cycle de vie), **EEE** et **MIA**.

| CAO de la transmission | Stabilité de la coque |
|---|---|
| ![](media/images/annee-1/mec-cao-transmission.jpg) | ![](media/images/annee-1/ese-stabilite.jpg) |

### EEE : commande du moteur et capteurs

**Semestre 1 : comprendre et simuler**
- Étude du moteur DC 12 V (4289 tr/min, 36,9 W, 5,3 A) et du **pont en H** pour inverser le sens.
- Découpage en **blocs fonctionnels** : commande (bouton, potentiomètre, microcontrôleur), puissance (batterie, régulation) et propulsion (pont en H, moteur).
- **Simulation sous SimulIDE** : réglage de la vitesse par **PWM**, puis essai sur le vrai circuit.

| Blocs fonctionnels | Simulation SimulIDE |
|---|---|
| ![](media/images/annee-1/eee-blocs-fonctionnels.jpg) | [![Simulation](media/images/annee-1/apercu-simulation-simulide.jpg)](media/videos/annee-1_simulation-simulide.mp4) |

**Semestre 2 : boucle de commande et capteurs**
- **Capteur de courant** ACS712 (185 mV/A) et **capteur de vitesse** infrarouge (roue codeuse de 20 trous), affichage sur LCD.
- Étude d'une **boucle de régulation** (correcteur PD sur FPGA) et d'un **multiplexeur manuel/automatique** pour la direction.
- **Asservissements** en vitesse et en position (correcteur P, codeur de 4500 points par tour).

![Chaîne d'information et d'énergie, année 1](media/images/annee-1/eee-chaine-information-energie.png)

| Boucle de régulation avec capteur de courant | Système de contrôle directionnel |
|---|---|
| ![](media/images/annee-1/eee-boucle-fpga-capteur-courant.jpg) | ![](media/images/annee-1/eee-controle-directionnel-mux.jpg) |

### MIA : premier autopilote

Un **potentiomètre** fixe le cap voulu et un **magnétomètre HMC5883L** mesure le cap réel. Le safran corrige proportionnellement à l'erreur :

```
erreur = consigne − cap              (ramenée dans [−180°, 180°])
angle_safran = 90° + erreur          ex. consigne 120°, cap 80° → safran 130°
```

| Principe | Correction du cap |
|---|---|
| ![](media/images/annee-1/mia-principe-autopilote.jpg) | ![](media/images/annee-1/mia-correction-cap.jpg) |

Code : [`tests/annee-1_2024-2025`](tests/annee-1_2024-2025) (8 programmes) · Rapports : [`docs/rapports/annee-1_2024-2025`](docs/rapports/annee-1_2024-2025)

---

## Année 2 (2025-2026) : construction et tests du bateau autopiloté

### Le bateau

![Vue d'ensemble](media/images/annee-2/vue-ensemble.jpg)

- **Coque en composite** : moule en 8 parties imprimé en 3D (PLA), fibre de verre, résine polyester et gel coat, ponçage et finition. L'intérieur est en balsa découpé au laser et le safran imprimé en PLA.
- **Stabilité** (simulation SolidWorks) : angle le plus stable à 20°, chavirement à 99°. Flottabilité : 59,4 N.
- **Économie circulaire** : en fin de projet, le moule a été réutilisé comme bac de potager.

| Moulage de la coque | Stratification | Intégration de l'électronique |
|---|---|---|
| ![](media/photos/moule-pla.jpg) | ![](media/photos/stratification-2.jpg) | ![](media/photos/integration-electronique-1.jpg) |

<details>
<summary>Plus de photos de fabrication</summary>

| | | |
|---|---|---|
| ![](media/photos/impression-3d-moule.jpg) | ![](media/photos/stratification-1.jpg) | ![](media/photos/stratification-fibre.jpg) |
| ![](media/photos/stratification-3.jpg) | ![](media/photos/fibre-de-verre-detail.jpg) | ![](media/photos/coque-interieur.jpg) |
| ![](media/photos/coque-sur-support.jpg) | ![](media/photos/coque-safran.jpg) | ![](media/photos/integration-electronique-2.jpg) |
| ![](media/photos/thermoformage-1.jpg) | ![](media/photos/piece-composite-proue.jpg) | ![](media/photos/plaque-gravee-leisure-boat.jpg) |

![Étapes de fabrication](media/images/annee-2/fabrication-etapes.jpg)
</details>

### EEE : dimensionnement et électronique de puissance

**Motorisation.** Moteur 775 12 V, 100 W, 12 000 tr/min à vide.
- En charge : ≈ 9500 tr/min, soit ω ≈ 995 rad/s et C ≈ 0,101 N·m.
- Rendement d'environ 80 %, d'où P_entrée ≈ 125 W et **I ≈ 10,4 A**.
- Avec l'hélice (laiton, 3 pales, Ø 5,85 cm, pas de 3,9 cm) : P = 86,1 W × 1,2 (marge de sécurité) = **103,3 W**.

**Énergie.** Batterie **LiFePO4 12 V 8 Ah** (886 g) : autonomie = 8 Ah / 8,33 A ≈ **0,96 h, soit environ 58 min**.

| Élément | Choix | Justification |
|---|---|---|
| Pont en H | **BTS7960** | 43 A crête, 5,5–27 V, entrées logiques compatibles ESP32 |
| Capteur de courant | **ACS758LCB-050B** | 50 A, 40 mV/A, effet Hall (isolé du circuit de puissance) |
| Capteur de vitesse | **LM393** (fourche optique) | Sortie numérique, comptage par interruption |
| Cap | **BMM150** (I2C) | Magnétomètre 3 axes 3,3 V (remplace le QMC5883L prévu) |
| Safran | Servo SG90 + **convertisseur DC-DC** | Alimentation stable, indépendante du moteur |
| Protections | **Fusible 15–20 A**, diode, interrupteur | Courts-circuits et inversion de polarité, coupure d'urgence |
| Câbles | **12–14 AWG** (puissance) | 30 A crête, longueur totale ≈ 190 cm, peu de pertes |

**Budget total : 252,13 €** (fabrication 141 €, électronique 111,13 €).

| Chaîne d'information et d'énergie | Schéma de câblage ESP32 |
|---|---|
| ![](media/images/annee-2/eee-chaine-information-energie.jpg) | ![](media/images/annee-2/schema-cablage-esp32.png) |

| Répartition des composants | Calculs moteur | Batterie |
|---|---|---|
| ![](media/images/annee-2/repartition-composants.jpg) | ![](media/images/annee-2/calculs-moteur.jpg) | ![](media/images/annee-2/batterie.jpg) |

### Banc de test : chaque composant validé seul

Avant l'intégration, chaque composant a été testé seul sur l'ESP32, au multimètre et avec une alimentation de laboratoire :

| Test | Objectif | Résultat |
|---|---|---|
| 1. Servo | Débattement du safran | 0-180° |
| 2. Pont en H BTS7960 | Vitesse et sens du moteur | avant / arrière, PWM 0-255 |
| 3. Magnétomètre BMM150 | Cap du bateau | fonctionne, lectures à stabiliser |
| 4. Capteur de vitesse | tr/min du moteur | Bon |
| 5. Capteur de courant | Consommation du moteur | Bon |

Bilan du banc : **environ 85 % des objectifs atteints**. Code : [`tests/annee-2_2025-2026`](tests/annee-2_2025-2026)

### MIA : du bouton App Inventor à l'autopilote PID

**1. Première commande (Arduino UNO WiFi Rev2 + MIT App Inventor).** L'application envoie des requêtes HTTP (`/avant`, `/stop`, `/gauche`…). Limite rencontrée : la carte ne pouvait pas créer son propre point d'accès Wi-Fi.

**2. Passage à l'ESP32.** L'ESP32 crée son réseau, **héberge une interface web** et expose une **API HTTP**. Plus besoin d'application à installer : un navigateur suffit.

**3. Version finale.** Firmware modulaire, télémétrie en temps réel et **autopilote PID** :

```mermaid
flowchart LR
    U["Smartphone<br/>(navigateur)"] -- "Wi-Fi · HTTP<br/>/motor /dir /speed /servo /autopilot" --> E
    E -- "JSON · 200/500 ms<br/>/mag /speed_data" --> U
    subgraph E["ESP32"]
      W[Serveur web + API] --> A[Autopilote PID 20 Hz]
      S[Capteurs] --> A
    end
    A -- "PWM servo" --> R[Safran]
    W -- "PWM 20 kHz" --> H[BTS7960 → moteur 775]
    M[BMM150] -- I2C --> S
    V[LM393] -- interruption --> S
    I[ACS758] -- ADC --> S
```

| Interface réelle sur téléphone | Mode autopilote |
|---|---|
| ![](media/images/annee-2/interface-web-reelle.png) | ![](media/images/annee-2/ui-autopilote.jpg) |

**Autopilote.** À 20 Hz : erreur de cap ramenée dans ±180°, zone morte de ±8°, puis angle du safran = 90° + Kp·e + Ki·∫e + Kd·ė (Kp = 0,35, Ki = 0,01, Kd = 0,05). Le safran est borné entre 65° et 115°, se déplace progressivement (3° toutes les 80 ms), et le cap est lissé sur 10 mesures.

Détails, brochage et API : [`firmware/README.md`](firmware/README.md)

### Tests sur l'eau

1. **Bassin** : pilotage manuel depuis le smartphone, réglage de la direction et de la puissance.
2. **Mare** : navigation avec les bateaux des autres groupes de la promotion.

| Bassin | Mare | Mare |
|---|---|---|
| ![](media/photos/test-bassin-application.jpg) | ![](media/photos/test-mare-bateaux.jpg) | ![](media/photos/test-mare-bateau.jpg) | 
| ![](media/videos/annee-2_test-bassin-application.mp4) | ![](media/videos/annee-2_test-mare-1.mp4) | ![](media/videos/annee-2_test-mare-2.mp4) |

---

## Architecture logicielle finale

| Couche | Technologie | Fichiers |
|---|---|---|
| Embarqué | C++ / Arduino (ESP32 core 3.x), interruptions, PWM LEDC, I2C, ADC | [`firmware/boat_control_final/`](firmware/boat_control_final) |
| Serveur | Serveur HTTP sur `WiFiServer`, API REST (GET) et JSON | [`web_server.cpp`](firmware/boat_control_final/web_server.cpp) |
| Front-end | HTML5, CSS3, JavaScript (`fetch`, SVG, événements tactiles) | [`firmware/web/`](firmware/web) |
| Outillage | Python (assemblage de l'interface en en-tête C++), Node.js (simulateur de l'API) | [`firmware/tools/`](firmware/tools) |

L'interface est développée comme une application web classique, puis **embarquée dans le firmware** par `build_web.py`. Le simulateur `mock_server.js` permet de la développer **sans le bateau**.

## Démarrage rapide
```bash
# Interface web avec le simulateur (Node.js, sans matériel)
cd firmware && node tools/mock_server.js        # → http://localhost:8080

# Firmware
cp firmware/boat_control_final/secrets.example.h firmware/boat_control_final/secrets.h
python firmware/tools/build_web.py              # après toute modification de firmware/web/
# Ouvrir firmware/boat_control_final/boat_control_final.ino → carte « ESP32 Dev Module » → Téléverser
# Bibliothèques : ESP32Servo, BMM150
```

## Organisation du dépôt

```
projet-bateau-autopilote/
├── README.md
├── firmware/                       code final (ESP32)
│   ├── boat_control_final/          sketch modulaire (.ino + .h/.cpp)
│   ├── web/                         interface : index.html, style.css, app.js
│   ├── tools/                       build_web.py, mock_server.js
│   └── historique/                  versions v1 → v3 et prototype d'API REST
├── tests/
│   ├── annee-1_2024-2025/           8 programmes (Arduino UNO)
│   └── annee-2_2025-2026/           banc de test (5) + prototypes (5)
├── docs/rapports/                   présentations et livrables (PDF), par année
└── media/
    ├── images/annee-1, annee-2/     schémas, diagrammes, captures
    ├── photos/                      fabrication et tests
    └── videos/                      vidéos (voir le README du dossier)
```

## Bilan et pistes d'amélioration

**Ce que le projet m'a appris :** mener un système de la feuille blanche jusqu'à l'eau : dimensionner (puissance, autonomie, câbles, protections), valider chaque composant seul avant de tout intégrer, et faire évoluer l'architecture logicielle (Arduino + application, puis ESP32 avec serveur web, API et interface). J'ai aussi appris à travailler dans une équipe pluridisciplinaire, sur deux années et avec deux équipes différentes.

**Pistes d'amélioration :**
- **Cap** : calibrer le magnétomètre en *hard/soft iron* et le fusionner avec un gyroscope (IMU + filtre complémentaire) pour stabiliser les lectures.
- **Navigation** : ajouter un GPS pour suivre des points de passage (*waypoints*), pas seulement un cap.
- **Logiciel** : passer à un serveur asynchrone (ESPAsyncWebServer) et à des WebSockets pour la télémétrie, servir l'interface depuis LittleFS, mettre la configuration à jour par OTA.
- **Sécurité** : retour automatique au neutre en cas de perte du Wi-Fi (*failsafe*), mesure de la tension batterie et alerte.

## Rapports

| Année 1 (2024-2025) | Année 2 (2025-2026) |
|---|---|
| [Présentation semestre 1](docs/rapports/annee-1_2024-2025/presentation-semestre-1.pdf) | [Présentation Go / No Go](docs/rapports/annee-2_2025-2026/presentation-go-no-go.pdf) |
| [Présentation semestre 2](docs/rapports/annee-1_2024-2025/presentation-semestre-2.pdf) | [Pitch fabrication](docs/rapports/annee-2_2025-2026/pitch-fabrication.pdf) |
| [Livrable EEE semestre 1](docs/rapports/annee-1_2024-2025/eee-livrable-semestre-1.pdf) | [Dossier technique EEE & MIA](docs/rapports/annee-2_2025-2026/eee-mia-dossier-technique.pdf) |
| [Livrable EEE semestre 2](docs/rapports/annee-1_2024-2025/eee-livrable-semestre-2.pdf) | [Rapport du banc de test EEE & MIA](docs/rapports/annee-2_2025-2026/eee-mia-banc-de-test.pdf) |
| [Synthèse et simulations](docs/rapports/annee-1_2024-2025/eee-synthese-simulations.pdf) | |

---

<p align="center"><i>ICAM · Bachelor General Engineering, Innovation & Technology · Projet Bateau autopiloté · 2024-2026</i></p>
