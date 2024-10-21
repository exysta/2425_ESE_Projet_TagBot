# 2425_ESE_Project_TagBot
Welcome to the TagBot project created by the amazing team consisted of Lucas Chapart, Charlotte Fricot and Marie Caronello.
The following description is in french but you can find the english description there :

- [English](README_english.md)

  # Sommaire
  
1. [Introduction](#introduction)
2. [PCB](#pcb)
3. [Code](#code)

  
  # Introduction
  
  Vous vous trouvez actuellement dans le projet TagBot (ou robot chat), il s'intègre à un projet de classe où chaque équipe doit concevoir un robot. Ces robots doivent pouvoir jouer au chat ensemble, avec un chat et des souris, sur une table sans bords. 
  
  Au terme de ce projet, notre chat robot doit pouvoir:
  - se déplacer sur une table sans bordure (sans tomber)
  - changer d'état (chat ou souris)
    - si le robot est chat il doit pouvoir attraper la souris
    - si le robot est souris il doit pouvoir échapper au chat
  
  Nous avons plusieurs niveaux d'objectifs à atteindre: 
  - Niveau 0 : robot se déplace + ne tombe pas de la table
  - Niveau 1 : détecte un robot et s'en rapproche (chat) ou s'en éloigne (souris)
  - Niveau 2 : change de comportement après un contact + fonctionne avec plusieurs robots
  - Niveau 3 : capable de se localiser + n'est pas affecté par les obstacles hors de la table

  Le projet doit être réalisé sur un semestre, du mois de septembre au mois de janvier, avec un certain nombre de livrables à fournir avant chaque étape clé. Les   différentes étapes techniques incluent des phases de conception, de développement et de validation sur un système embarqué.

  <details>
  <summary><strong>Plan détaillé</strong></summary>  

  ### Avant les vacances de la Toussaint
  
  Les 10 premières séances sont consacrées à la création du PCB que l'on doit commander avant les vacances de la Toussaint. 
  
  - Séance 1 → Schéma architectural / BOM
  - Séance 2 et 3 → Schéma électronique annoté
  - Séance 4 → Corrections Schéma / BOM Finale
  - Séance 5 → Placement
  - Séance 6 → Placement corrigé
  - Séance 7 → Routage
  - Séance 8, 9 et 10 → Corrections Routage, export

  Dans le même temps, nous avons débuté la partie informatique avec la création du projet sur CubeIDE. Nous avons configuré les pins et commencé la rédaction des librairies pour les différents composants. 

  ### Entre la Toussaint et Noël

  Cette période est centrée sur le développement logiciel et l'intégration.

  ### Après les vacances de Noël

  L'accent est mis sur les derniers ajustements et les tests finaux avant la présentation. En effet, les dernières séances consacrées à ce projet vont nous permettre de finaliser les algorithmes de stratégie et les comportements robotiques. Le challenge final aura lieu le vendredi 10 janvier, ce challenge consistera en un test et une présentation du projet où tous les robots chats de notre classe s'affronteront.

   </details>

  ## Schéma architectural et materiel

  <div align="center">
<img src="./Images/Schema_architecture.png" width="400">
</div>
  
  L'essentiel du materiel nous a été imposé, mais nous devons trouver par nous même le moyen par lequel nous voulons détetecter les bords pour ne pas tomber de la table. 
  Pour cela on aimerait utiliser les capteurs IR ce capteur sera placer sous le robot et recevra donc une tension qui déterminera sa distance par rapport au sol, quand ce signal diminue fortement (donc la distance augmente) cela signifie qu'on est sur un bord. Comme il sera sous le robot il nous faut un capteur qui est capable de detecté des petites distances comme le capteur sharp GP2Y0A21SK0F. 
  
  On aimerai aussi ajouter une led visible qui nous permettrait de savoir quel est l'état de notre robot chat.

  # PCB
  La première étape de la conception de notre PCB a été de réaliser le schéma électronique de notre carte. Voici une liste des composants utilisés :

- **Microprocesseur** : STM32G431CBU6
- **Régulateurs de tension** : 5V et 3.3V
- **Drivers de moteurs** : ZXBM5210-SP-13
- **Système de batterie** : Batterie NIMH 7.2V 1.3Ah 
- **ST Link** : Interface de programmation et débogage
- **Oscillateur Quartz** : 16MHz
- **Connecteurs** : Divers connecteurs pour l'alimentation et les signaux
- **Accéléromètre** : ADXL343

## Détail des Composants et Branchements

### 1. Microprocesseur : STM32G431CBU6

Le microprocesseur STM32G431x6 est au cœur du robot. Il gère la logique du jeu et communique avec les autres composants. Le schéma de branchement du microprocesseur est disponible [ici](./Documents/datasheets/stm32g431cb.pdf).

<div align="center">
<img src="./Images/Kicad_microprocesseur.png" width="400">
</div>

Les principaux branchements incluent :

- **VDDA** : Connecté à un filtrage LC pour lisser l'alimentation analogique.
- **VSS / VDD** : Alimentations principales.
- **GPIOs** : Utilisés pour le contrôle des LED de débogage et la communication avec les drivers de moteur.

Pour configurer les broches du microcontrôleur, nous utilisons le logiciel CubeIDE. Une fois la configuration effectuée, nous générons le fichier `.ioc` qui documente l'ensemble des options choisies pour chaque pin. Le fait d'utiliser l'ioc du microprocesseur nous permet de choisir au mieux chaque pin pour les entrée/sortie du microprocesseur.

### 2. LED

Le robot comporte 7 LEDs connectées à des résistances de limitation de courant.

- **Une LED rouge** : indique que le robot est le chat.
- **Une LED Bleue** : indique que le robot est une souris.
- **Deux LED vertes** : qui sont des LEDs de debug.
- **Une LED jaune** : 5V
- **Une LED verte** : 3.3V
- **Une LED verte** : 7.2V

<div align="center">
<img src="./Images/Kicad_microprocesseur_led.png" width="400">
</div>

### 3. Boutons

Le robot dispose de deux boutons :

- **NRST** : Bouton de réinitialisation.
- **Mode** : Bouton qui permet de changer l'état du robot entre "chat" et "souris".

<div align="center">
<img src="./Images/Kicad_microprocesseur_boutons.png" width="400">
</div>

### 4. Drivers de Moteurs : ZXBM5210-SP-13

Les drivers de moteurs permettent de contrôler les moteurs à courant continu utilisés pour déplacer le robot. Nous utilisons le driver ZXBM5210, connecté selon le schéma de la [documentation](./Documents/datasheets/driver_ZXBM5210.pdf).

<div align="center">
<img src="./Images/Kicad_drivers.png" width="400">
</div>

Le branchement des drivers inclut :

- **Vref** : Tension de référence pour le contrôle de vitesse.
- **VDD** : Alimentation des moteurs.
- **Entrées de contrôle** : Connectées aux GPIOs du microprocesseur.

<div align="center">
<img src="./Images/Kicad_driver.png" width="400">
</div>

### 5. Régulateurs de Tension

Nous utilisons deux régulateurs de tension pour convertir l'alimentation de la batterie :

<div align="center">
<img src="./Images/Kicad_regulateurs.png" width="400">
</div>

1. **Régulateur 7.2V → 5V : MP1475S**
   - Construit selon la [documentation](./Documents/datasheets/regulateur_MP1475S.pdf).
   - Assure une tension de 5V en sortie pour l'alimentation du LIDAR et des capteurs de bords.
   - On prend donc la résistance R = 7.68k cette valeur est disponible donc nous pouvons garder la valeur. Si ca n’avait pas été le cas on aurait du réaliser un diviseur de tension pour retrouver les valeurs des résistances pour qu en sortie on est du 5V et 3A.
   - Pour s'assurer que nous recevons bien une tension de 5V, nous avons ajouter une LED jaune connectée à une résistance de limitation de courant.

<div align="center">
<img src="./Images/Kicad_regulateur_MP1475S.png" width="400">
</div>

2. **Régulateur 5V → 3.3V : BU33SD5WG-TR**
   - Construit selon la [documentation](./Documents/datasheets/regulateur_buxxsd5wg-e.pdf).
   - Alimente le microprocesseur et l'écran en 3.3V.
   - Pour s'assurer que nous recevons bien une tension de 3V, nous avons ajouter une LED verte connectée à une résistance de limitation de courant.

<div align="center">
<img src="./Images/Kicad_regulateur_BU33SD5WG.png" width="400">
</div>

### 6. Oscillateur Quartz 16MHz

L'oscillateur quartz est utilisé pour fournir une horloge stable au microprocesseur. Il est connecté selon le schéma standard, avec des condensateurs de découplage.

<div align="center">
<img src="./Images/Kicad_quartz.png" width="400">
</div>

### 7. ST Link

La ST Link est utilisée pour la programmation et le débogage du microprocesseur STM32. Elle permet de flasher le code sur le microprocesseur et d'assurer une communication série pour le débogage.

- **VCC** : Alimentation de la ST Link (3.3V).
- **SWDIO / SWCLK** : Broches de communication pour le débogage.
- **NRST** : Connecté au bouton de réinitialisation pour permettre un reset matériel lors de la programmation.

<div align="center">
<img src="./Images/Kicad_STlink.png" width="400">
</div>

### 8. Accéléromètre : ADXL343

L'accéléromètre ADXL343 permet de mesurer les mouvements du robot. Il est connecté au microprocesseur via une communication **SPI**. La configuration SPI a été choisie car elle offre une communication plus rapide et fiable dans notre application. Le schéma de branchement est basé sur la [documentation](./Documents/datasheets/accelerometre_adxl343.pdf).

<div align="center">
<img src="./Images/Kicad_accelerometre.png" width="400">
</div>

### 9. Connecteurs

Le robot dispose de plusieurs connecteurs pour interfacer différents périphériques :

<div align="center">
<img src="./Images/Kicad_connecteurs.png" width="400">
</div>

1. **Connecteur Moteurs**

- Les connecteurs moteurs permettent de relier le PCB aux moteurs DC utilisés pour déplacer le robot.
- Chaque connecteur est relié à un driver de moteur ZXBM5210 pour gérer la vitesse et la direction.

<div align="center">
<img src="./Images/Kicad_connecteur_moteur.png" width="400">
</div>


2. **Connecteur LIDAR**

- Le connecteur Lidar est utilisé pour interfacer un capteur Lidar au robot, permettant la détection de la distance et des obstacles.
- Il est connecté à l'un des ports de communication du microprocesseur (SPI ou UART).

<div align="center">
<img src="./Images/Kicad_connecteur_lidar.png" width="400">
</div>

3. **Connecteur Écran**

- Le connecteur écran permet d'ajouter un petit écrat pour afficher l'état du robot (chat ou souris).
- Ce connecteur est relié à l'interface I2C du microprocesseur.

<div align="center">
<img src="./Images/Kicad_connecteur_ecran.png" width="400">
</div>

4. **Connecteur Capteur Bords**

- Le capteur bords est utilisé pour détecter les bords de la table et éviter que le robot ne tombe.
- Le capteur est relié intrinséquement à un DAC

<div align="center">
<img src="./Images/Kicad_connecteur_capteurbords.png" width="400">
</div>


### 10. Batterie

Le robot est alimenté par une batterie LiPo de 7.4V, qui fournit de l'énergie aux régulateurs de tension pour les différents composants du circuit, la documentation utilisé est [ici](./Documents/datasheets/batterie_0900766b81582941.pdf). Pour s'assurer que nous recevons bien une tension de 7,2V, nous avons ajouté une LED verte connectée à une résistance de limitation de courant.

<div align="center">
<img src="./Images/Kicad_batterie.png" width="400">
</div>

  
# Code

  ## Documentation Code

[voir la documentation](./Documents/Doxygen_Documentation/html/index.html)
  
  ## Introduction

  Dans le cadre du projet, nous devons utiliser plusieurs moyens de communications tels que le SPI pour l'accéléromètre, l'I2C pour l'écran OLED
  
  ## Pilote Lidar X4
  
  Le fichier `X4_driver.c` fournit une implémentation pour interfacer avec le périphérique X4 via la communication UART. Ce pilote prend en charge diverses fonctionnalités, y compris le démarrage et l'arrêt des analyses, la récupération des informations et de l'état de l'appareil, et le traitement des données d'analyse. Le pilote utilise un protocole défini dans le fichier d'en-tête `X4_driver.h` et gère les réponses du périphérique X4.
  
  - **Bibliothèque STM32 HAL** pour la communication UART.
  - **Bibliothèques C standard** (string.h, math.h, stdlib.h, stdbool.h).
  
  <details>
    <summary><strong>Fichier header X4_driver.h</strong></summary>
  
  ### Fichier d'en-tête : `X4_driver.h`
  
  #### Macros
  
    - **Octets de Commande :**
      - `X4_CMD_START` - Début d'une séquence de commande.
      - `X4_CMD_START_SCAN` - Commande pour démarrer une analyse.
      - `X4_CMD_STOP_SCAN` - Commande pour arrêter une analyse.
      - `X4_CMD_GET_INFO` - Commande pour obtenir des informations sur l'appareil.
      - `X4_CMD_GET_HEALTH` - Commande pour obtenir l'état de l'appareil.
      - `X4_CMD_SOFT_RESTART` - Commande pour effectuer un redémarrage à chaud.
  
    - **Protocole de Réponse :**
      - `X4_RESPONSE_START_SIGN` - Signature de début attendue d'une réponse.
      - `X4_RESPONSE_SINGLE_MODE` - Mode de réponse unique.
      - `X4_RESPONSE_CONTINUOUS_MODE` - Mode de réponse continue.
  
    - **Tailles des Réponses :**
      - `X4_RESPONSE_HEADER_SIZE` - Taille de l'en-tête de réponse.
      - `X4_MAX_RESPONSE_SIZE` - Taille maximale du contenu de la réponse.
      - `X4_SERIAL_NUMBER_SIZE` - Taille du numéro de série.
      - `X4_SERIAL_FIRMWARE_SIZE` - Taille de la version du firmware.
  
  #### Structures
  
    - **`X4_ResponseMessage`**
      - Contient des champs pour l'analyse du message de réponse, y compris la signature de début, la longueur de la réponse, le mode, le code de type et le contenu.
  
    - **`X4_DeviceInfo`**
      - Contient des champs pour le modèle de l'appareil, la version du firmware, la version matérielle et le numéro de série.
  
    - **`X4_ScanData`**
      - Contient des champs pour l'en-tête du paquet, le type de paquet, la quantité d'échantillons, les angles, le code de vérification, et des données échantillons, distances et angles allouées dynamiquement.
  
  #### Prototypes de Fonction
  
    - **`void X4_StartScan(void);`**
      - Démarre une analyse sur le périphérique X4.
  
    - **`void X4_StopScan(void);`**
      - Arrête l'analyse en cours.
  
    - **`void X4_GetDeviceInfo(void);`**
      - Récupère les informations sur l'appareil.
  
    - **`void X4_GetHealthStatus(void);`**
      - Récupère l'état de l'appareil.
  
    - **`void X4_SoftRestart(void);`**
      - Effectue un redémarrage à chaud de l'appareil.
  
    - **`void X4_HandleResponse(void);`**
      - Gère la réponse reçue du périphérique X4.
  
    - **`void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response);`**
      - Analyse un message brut en une réponse structurée.
  
    - **`uint16_t convertBytesToUint16(const uint8_t* byte_array);`**
      - Convertit un tableau de 2 octets en un entier non signé de 16 bits.
    
  </details>
  
  <details>
    <summary><strong>Fichier Source : X4_driver.c</strong></summary>
  
  ### Fonctions Clés
  
    - **`static void X4_SendCommand(uint8_t command)`**
      - Envoie une commande au périphérique X4 via UART.
  
    - **`void X4_StartScan(void)`**
      - Envoie la commande de démarrage de l'analyse et gère la réponse.
  
    - **`void X4_StopScan(void)`**
      - Envoie la commande d'arrêt de l'analyse.
  
    - **`void X4_SoftRestart(void)`**
      - Envoie la commande de redémarrage à chaud.
  
    - **`void X4_GetDeviceInfo(void)`**
      - Envoie la commande pour obtenir des informations sur l'appareil et traite la réponse.
  
    - **`void X4_GetDeviceHealth(void)`**
      - Envoie la commande pour obtenir l'état de l'appareil et traite la réponse.
  
    - **`void X4_HandleDeviceInfoResponse(const X4_ResponseMessage* response)`**
      - Gère et analyse la réponse d'information sur l'appareil.
  
    - **`void X4_HandleDeviceHealthResponse(const X4_ResponseMessage* response)`**
      - Gère et analyse la réponse de l'état de l'appareil.
  
    - **`void X4_HandleScanResponse(const X4_ResponseMessage* response)`**
      - Gère et analyse la réponse des données d'analyse.
  
    - **`void X4_HandleResponse(void)`**
      - Reçoit et traite les données UART, les envoie au gestionnaire approprié.
  
    - **`void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response)`**
      - Analyse un message brut en une réponse structurée.
  
    - **`void X4_HandleScanDataDistances(X4_ScanData *scan_data)`**
      - Traite et calcule les distances à partir des données d'analyse.
  
    - **`void X4_HandleScanDataAngles(X4_ScanData *scan_data)`**
      - Traite et calcule les angles à partir des données d'analyse.
  
    - **`uint16_t calculateXOR(const X4_ScanData *scan_data, size_t packet_length)`**
      - Calcule le XOR de tous les octets dans le paquet, à l'exception du code de vérification.
  
    - **`bool verifyCheckCode(const X4_ScanData *scan_data, size_t packet_length)`**
      - Vérifie l'intégrité des données d'analyse en utilisant le code de vérification.
  
    - **`uint16_t convertBytesToUint16(const uint8_t* byte_array)`**
      - Convertit un tableau de 2 octets en un entier non signé de 16 bits.
  </details>
  
  <details>
    <summary><strong>Utilisation </strong></summary>
    
    1. **Initialisation :**
       - Assurez-vous que UART est correctement initialisé et configuré dans votre application principale.
  
    2. **Envoi de Commandes :**
       - Utilisez des fonctions telles que `X4_StartScan()`, `X4_StopScan()` et `X4_GetDeviceInfo()` pour interagir avec le périphérique X4.
  
    3. **Gestion des Réponses :**
       - Appelez `X4_HandleResponse()` pour recevoir et traiter les réponses. Implémentez des gestionnaires appropriés pour les informations sur l'appareil, l'état de l'appareil et les données d'analyse.
  
    4. **Traitement des Données d'Analyse :**
       - Utilisez `X4_HandleScanDataDistances()` et `X4_HandleScanDataAngles()` pour traiter les données d'analyse et calculer les distances et les angles.
  </details>
  
  <details>
    <summary><strong>Remarques </strong></summary>
    
  ### Remarques
  
    - Assurez-vous que la fonction `HAL_UART_Receive` est configurée avec un délai d'attente approprié et une gestion des erreurs selon les besoins de votre application.
    - Ajustez `X4_MAX_RESPONSE_SIZE` dans le fichier d'en-tête si nécessaire en fonction de la taille de réponse attendue du périphérique.
  </details>

## 
