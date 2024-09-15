# 2425_ESE_Projet_TagBot

## Introduction

Vous vous trouvez actuellement dans le projet TagBot (ou chat robot), il s'intègre à un projet de classe ou chaque équipe doit concevoir un robot, ces robots doivent pouvoir jouer au chat ensemble, avec un chat et des souris, sur une table sans bords. 

Ce projet est la conception d'un chat robot il doit:
- se déplacer sur une table sans bord (sans tomber)
- pouvoir changer d'état (chat ou souris)
  - si le robot est chat il doit pouvoir attraper la souris
  - si le robot est souris il doit pouvoir échaper au chat

Nous avons plusieurs niveaux d'objectifs à atteindre: 
Niveau 0 : robot se déplace + ne tombe pas de la table
Niveau 1 : détecte un robot et s'en rapproche (chat) ou s'en éloigne (souris)
Niveau 2 : change de comportement après un contact + fonctionne avec plusieurs robots
Niveau 3 : capable de se localiser + n'est pas affecté par les obstacles hors de la table

## Systeme à microprocessuers

Les 10 premières séances sont consacrées à la création du PCB que l'on doit commander avant les vacances de Toussaint. 

Séance 1 -> Schéma architectural / BOM
Séance 2 et 3 -> Schéma électronique annoté
Séance 4 -> Corrections Schéma / BOM Finale
Séance 5 -> Placement
Séance 6 -> Placement corrigé
Séance 7 et 8 -> Routage
Séance 9 et 10 -> Corrections Routage, export


  
# Pilote Lidar X4

Le fichier `X4_driver.c` fournit une implémentation pour interfacer avec le périphérique X4 via la communication UART. Ce pilote prend en charge diverses fonctionnalités, y compris le démarrage et l'arrêt des analyses, la récupération des informations et de l'état de l'appareil, et le traitement des données d'analyse. Le pilote utilise un protocole défini dans le fichier d'en-tête `X4_driver.h` et gère les réponses du périphérique X4.

- **Bibliothèque STM32 HAL** pour la communication UART.
- **Bibliothèques C standard** (string.h, math.h, stdlib.h, stdbool.h).

<details>
  <summary><strong>Fichier header X4_driver.h</strong></summary>

  ## Fichier d'en-tête : `X4_driver.h`

  ### Macros

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

  ### Structures

  - **`X4_ResponseMessage`**
    - Contient des champs pour l'analyse du message de réponse, y compris la signature de début, la longueur de la réponse, le mode, le code de type et le contenu.

  - **`X4_DeviceInfo`**
    - Contient des champs pour le modèle de l'appareil, la version du firmware, la version matérielle et le numéro de série.

  - **`X4_ScanData`**
    - Contient des champs pour l'en-tête du paquet, le type de paquet, la quantité d'échantillons, les angles, le code de vérification, et des données échantillons, distances et angles allouées dynamiquement.

  ### Prototypes de Fonction

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

  ## Fonctions Clés

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
  ## Remarques

  - Assurez-vous que la fonction `HAL_UART_Receive` est configurée avec un délai d'attente approprié et une gestion des erreurs selon les besoins de votre application.
  - Ajustez `X4_MAX_RESPONSE_SIZE` dans le fichier d'en-tête si nécessaire en fonction de la taille de réponse attendue du périphérique.
</details>

## Le materiel

L'essentiel du materiel nous a été imposé, mais nous devons trouver par nous même le moyen par lequel nous voulons détetecter les bords pour ne pas tomber de la table. 
Pour cela on aimerait utiliser les capteurs IR ce capteur sera placer sous le robot et recevra donc une tension qui déterminera sa distance par rapport au sol, quand ce signal diminue fortement (donc la distance augmente) cela signifie qu'on est sur un bord. Comme il sera sous le robot il nous faut un capteur qui est capable de detecté des petites distances comme le capteur sharp GP2Y0A21SK0F. 

On aimerai aussi ajouter une led visible qui nous permettrait de savoir quel est l'état de notre robot chat.
