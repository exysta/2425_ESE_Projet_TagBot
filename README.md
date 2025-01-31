# 2425_ESE_Project_TagBot
Welcome to the TagBot project created by the amazing team consisted of Lucas Chapart, Charlotte Fricot and Marie Caronello.
The following description is in french but you can find the english description there but it is not up to date for now :

- [English](README_english.md)

  # Sommaire
  
1. [Introduction](#introduction)
2. [PCB](#pcb)
3. [Code](#code)

  
  # Introduction
  
Vous vous trouvez actuellement dans le projet TagBot (ou robot chat), qui s'intègre à un projet de classe où chaque équipe doit concevoir un robot. Ces robots doivent pouvoir jouer au chat ensemble, avec un chat et des souris, sur une table sans bords. 
  
Au terme de ce projet, notre chat robot doit pouvoir:
 - se déplacer sur une table sans bordure (sans tomber)
 - changer d'état (chat ou souris)
  - si le robot est chat, il doit pouvoir attraper la souris
  - si le robot est souris, il doit pouvoir échapper au chat
  
Nous avons plusieurs niveaux d'objectifs à atteindre : 
  - Niveau 0 : le robot se déplace + ne tombe pas de la table
  - Niveau 1 : détection d'un robot et s'en rapprocher (chat) ou s'en éloigner (souris)
  - Niveau 2 : changement de comportement après un contact + fonctionnement avec plusieurs robots
  - Niveau 3 : capacité de se localiser + ne pas être affecté par les obstacles hors de la table

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
  
  L'essentiel du matériel a été imposé, mais il restait à déterminer le moyen par lequel le robot détecterait les bords pour éviter de tomber de la table. Pour cela, des capteurs infrarouges (IR) ont été choisis. Placés sous le robot, ces capteur reçoivent une tension qui détermine la distance par rapport au sol. Lorsque ce signal diminue fortement (ce qui correspond à une augmentation de la distance), cela indique la présence d'un bord. Étant donné que les capteurs sont situés sous le robot, il est nécessaire de détecter de faibles distances, comme le permet le capteur Sharp GP2Y0A21SK0F.

  Il est également envisagé d'ajouter une LED visible par l'utilisateur, permettant de connaître l'état du robot chat à tout moment.

  # PCB
  La première étape de la conception du PCB a été de réaliser le schéma électronique de la carte. Voici une liste des composants utilisés :

- **Microprocesseur** : STM32G431CBU6
- **Régulateurs de tension** : 5V et 3.3V
- **Drivers de moteurs** : ZXBM5210-SP-13
- **Système de batterie** : Batterie NIMH 7.2V 1.3Ah 
- **ST Link** : Interface de programmation et débogage
- **Oscillateur Quartz** : 16MHz
- **Connecteurs** : Divers connecteurs pour l'alimentation et les signaux
- **Accéléromètre** : ADXL343

## Détail des composants et branchements

### 1. Microprocesseur : STM32G431CBU6

Le microprocesseur STM32G431x6 est au cœur du robot. Il gère la logique du jeu et communique avec les autres composants. Le schéma de branchement du microprocesseur est disponible [ici](./Documents/datasheets/stm32g431cb.pdf).

<div align="center">
<img src="./Images/Kicad_microprocesseur.png" width="400">
</div>

Les principaux branchements incluent :

- **VDDA** : Connecté à un filtrage LC pour lisser l'alimentation analogique.
- **VSS / VDD** : Alimentations principales.
- **GPIOs** : Utilisés pour le contrôle des LED de débogage et la communication avec les drivers de moteur.

Pour configurer les broches du microcontrôleur, le logiciel CubeIDE est utilisé. Une fois la configuration effectuée, la génération du fichier `.ioc` permet de documenter l'ensemble des options choisies pour chaque broche. Le fait d'utiliser l'ioc du microprocesseur permet de choisir au mieux chaque broche pour les entrée/sortie du microprocesseur.

### 2. LED

Le robot comporte 7 LEDs connectées à des résistances de limitation de courant.

- **Une LED rouge** : indique que le robot est le chat.
- **Une LED Bleue** : indique que le robot est une souris.
- **Deux LED vertes** : sont des LEDs de débogage.
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

Les drivers de moteurs permettent de contrôler les moteurs à courant continu utilisés pour déplacer le robot. Le driver ZXBM5210 est connecté selon le schéma de la [documentation](./Documents/datasheets/driver_ZXBM5210.pdf).

<div align="center">
<img src="./Images/Kicad_drivers.png" width="400">
</div>

Le branchement des drivers inclut :

- **Vref** : Tension de référence pour le contrôle de vitesse
- **VDD** : Alimentation des moteurs
- **Entrées de contrôle** : Connectées aux GPIOs du microprocesseur

<div align="center">
<img src="./Images/Kicad_driver.png" width="400">
</div>

### 5. Régulateurs de Tension

Deux régulateurs de tension sont utilisés pour convertir l'alimentation de la batterie :

<div align="center">
<img src="./Images/Kicad_regulateurs.png" width="400">
</div>

1. **Régulateur 7.2V → 5V : MP1475S**
   - Construit selon la [documentation](./Documents/datasheets/regulateur_MP1475S.pdf).
   - Assure une tension de 5V en sortie pour l'alimentation du LIDAR et des capteurs de bords.
   - Concernant la résistance R = 7.68k, cette valeur est disponible et a donc été conservée. Si cela n’avait pas été le cas, il aurait été nécessaire de réaliser un diviseur de tension pour déterminer les valeurs des résistances permettant d'obtenir une sortie de 5V et 3A.
   - Pour vérifier la présence de la tension de 5V, une LED jaune a été ajoutée, et a été connectée à une résistance de limitation de courant.

<div align="center">
<img src="./Images/Kicad_regulateur_MP1475S.png" width="400">
</div>

2. **Régulateur 5V → 3.3V : BU33SD5WG-TR**
   - Construit selon la [documentation](./Documents/datasheets/regulateur_buxxsd5wg-e.pdf).
   - Alimente le microprocesseur et l'écran en 3.3V.
   - Pour vérifier la précence de la tension de 3V, une LED verte a été ajoutée et a été connectée à une résistance de limitation de courant.

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

- Le connecteur écran permet d'ajouter un petit écran pour afficher l'état du robot (chat ou souris).
- Ce connecteur est relié à l'interface I2C du microprocesseur.

<div align="center">
<img src="./Images/Kicad_connecteur_ecran.png" width="400">
</div>

4. **Connecteur Capteur Bords**

- Le capteur bords est utilisé pour détecter les bords de la table et éviter que le robot ne tombe.
- Le capteur est relié intrinséquement à un ADC.

<div align="center">
<img src="./Images/Kicad_connecteur_capteurbords.png" width="400">
</div>


### 10. Batterie

Le robot est alimenté par une batterie LiPo de 7.2V, qui fournit de l'énergie aux régulateurs de tension pour les différents composants du circuit, la documentation utilisé est [ici](./Documents/datasheets/batterie_0900766b81582941.pdf). Pour vérifier la présence de la tension de 7.2V, une LED verte a été ajoutée et connectée à une résistance de limitation de courant.

<div align="center">
<img src="./Images/Kicad_batterie.png" width="400">
</div>


## Ajustements à considérer pour un routage optimal
<details>
  <summary><strong> Détails</strong></summary>
  
### Répartitions des roles des couches du PCB

Répartition des 4 couches du PCB :
- **Couche 1 (Front)** :
Couche sur laquelle sont placés tous nos composants, la plupart étant en CMS, à l'exception des connecteurs qui sont en traversants. La majoté des pistes de données se trouvent sur cette couche, tandis qu'une partie d'entre elles passent par la couche bottom. Cela permet de maintenir l'homogénéité des plans de masse (couche 2) et d'alimentation (couche 3), ce qui protège l'intégrité des signaux de données.
  
- **Couche 2** :
Plan de masse. Il est important d'espacer nos vias pour ne pas couper le plan de masse.
  
- **Couche 3** :
Couche d'alimentation. Elle contient les plans d'alimentations pour les composants à forte consommation de puissance. Cela permet une meilleure dissipation thermique.
  
- **Couche 4 (Bottom)** :
Une partie des pistes de données ainsi que les points de test se situent sur cette couche. Lors du routage des pistes sur cette couche, il faut essayer de ne pas faire passer les pistes par les discontinuités des plans d'alimentations présents sur la couche 3. En effet, cette discontinuité peut poser des problèmes de CEM pour les signaux de données les plus rapides tels que : UART, I2C, SPI et signaux de debug.
  
### Bonnes pratiques pour les plans

- **Attention aux antennes** :
Les pistes ou zones de cuivre non connectées peuvent se comporter comme des antennes et générer des interférences électromagnétiques (EMI). Pour minimiser ce risque, il est essentiel d’éviter la création de segments de cuivre flottants, en particulier dans les plans de masse et d’alimentation. Sur l'image ci-dessous, des antennes sont visibles. Ce problème peut être évité en ajoutant des vias aux extrémités de ces antennes ou en définissant une zone d'exclusion pour empêcher le remplissage.

<div align="center">
<img src="./Images/Kicad/PCB/antennes.png" width="400">
</div>

- **Éviter les discontuinités des plans de masse** :
Les interruptions dans les plans de masse peuvent créer des chemins de retour de courant indésirables, affectant la performance globale du circuit, notamment pour les signaux à haute fréquence. Un plan de masse continu garantit un retour propre des courants et prévient les problèmes d’intégrité du signal. Il est recommandé de s'assurer que les pistes de signal, d’alimentation ou les vias ne créent pas de coupures dans ce plan, comme on peut le voir sur l'image ci-dessous.

<div align="center">
<img src="./Images/Kicad/PCB/coupure_plan_via.png" width="400">
</div>

### Bonnes pratiques pour les pistes et vias

- **Placer les vias proches des pads lorsque cela est possible** :
Les vias doivent être placés aussi près que possible des pads des composants pour réduire la longueur des pistes et ainsi limiter l'inductance parasite.

<div align="center">
<img src="./Images/Kicad/PCB/via_près_pad.png" width="400">
</div>
  
- **Adapter la largeur des pistes en fonction du type de signal pour des raisons thermiques**:
Il est important de dimensionner correctement la largeur des pistes selon le type de courant qui les traverse. Les pistes de puissance, par exemple, doivent être suffisamment larges pour assurer une bonne dissipation thermique et limiter les pertes résistives. Une largeur insuffisante pourrait entraîner une surchauffe ou une dégradation des performances du circuit. À l'inverse, les pistes de signal, qui véhiculent des courants plus faibles, peuvent être plus fines.

<div align="center">
<img src="./Images/Kicad/PCB/comparaison_taille_piste.png" width="400">
</div>

- **Éviter les angles droits ou inférieurs à 90° sur les pistes** :
Les pistes présentant des angles droits ou très aigus peuvent entraîner des discontinuités d’impédance, causant des réflexions de signal et des perturbations électromagnétiques, notamment dans les circuits haute fréquence. Ces angles peuvent également constituer des points de faiblesse thermique ou mécanique. Il est recommandé d’utiliser des angles de 45° ou des courbes douces pour maintenir la continuité du signal et réduire les effets de réflexion et d’interférence.

- **Position des vias par rapport aux pastilles de brasage**:
Sur l'image ci-dessous, les vias ne sont pas placés directement sur les pastilles de brasage. Cette configuration a été choisie pour éviter des problèmes de soudure et pour permettre une meilleure dissipation thermique.

<div align="center">
<img src="./Images/Kicad/PCB/via_et_pad.png" width="400">
</div>

###  Bonnes pratiques pour les composants

- **Condensateurs de découplage à faible valeur** :
Les condensateurs de plus petite capacité (souvent des céramiques, typiquement dans les gammes de 100nF à 1µF) sont destinés à filtrer les hautes fréquences et doivent être placés aussi près que possible des pins d'alimentation des composants. Leur faible constante de temps leur permet de répondre rapidement aux fluctuations rapides de la tension causées par des commutations rapides dans les circuits numériques.

- **Condensateurs de plus grande valeur**:
Les condensateurs de plus grande capacité (souvent dans les gammes de 10µF à 100µF ou plus, et souvent des électrolytiques ou des tantales) servent à fournir une énergie plus conséquente pour des fluctuations de tension plus lentes, mais ne sont pas aussi efficaces pour les hautes fréquences. Ils peuvent être placés un peu plus loin des composants, mais idéalement, ils devraient rester dans le même plan d'alimentation. Leur rôle est de compléter les condensateurs de faible valeur, en offrant une stabilisation à des fréquences plus basses.

### Tests points
L'ajout de points de test accessibles facilite la vérification des tensions d'alimentation, des signaux critiques ou d'autres tests en cours de fabrication et de débogage.

</details>

# Code

  ## Documentation Code
Le détail des fonctions structures et autres peuvent être visualiser en utilisant la documentation Doxygen :

[voir la documentation Doxygen](https://exysta.github.io/2425_ESE_Projet_TagBot/Documents/Doxygen_Documentation/html/index.html)

La documentation qui suit est un résumé du fonctionement et ne rentre pas trop dans les détails déjà couvert par la documentation DOxygen.
  
  ## Introduction

  Dans le cadre du projet, nous devons utiliser plusieurs moyens de communications tels que le SPI pour l'accéléromètre, l'I2C pour l'écran OLED, l'UART pour le lidar et une lecture de tension avec un ADC pour les capteurs de distance.

  ## FreeRTOS config 
  
  ![image](https://github.com/user-attachments/assets/a79ae845-7e1e-4802-bca1-e244e7265806)

  ## Task Priority
  - Motor task = 11
  - Distance sensor task = 10
  - Robot strategy task = 9
  - Accelerometer task = 8
  - Lidar task = 7
  - Shell task = 6

  Cette ordre de priorité nous permet de nous assurer un controle des moteurs prioritaire est assure une bonne détection d'un potententiel vide par notre tâche Distance sensor
## exemple d'utilisation dans un main.c
  ```c
/* USER CODE BEGIN Includes */
#include "stm32g4xx_hal.h"
#include "semphr.h"
#include "X4LIDAR_driver.h"
#include "DCMotor_driver.h"
#include "SSD1306.h"
#include "SSD1306_fonts.h"
#include "shell.h"
#include "RobotStrategy.h"
#include "distSensor_driver.h"
#include "ADXL343_driver.h"

/* USER CODE BEGIN PV */
X4LIDAR_handle_t X4LIDAR_handle;
DualDrive_handle_t DualDrive_handle;
__TARGET_HandleTypeDef Target_Handle;

int main(void)
{
...

	printf(" _____________________________\r\n");
	printf("|                             |\r\n");
	printf("|                             |\r\n");
	printf("|  WELCOME ON TAGBOT PROJECT  |\r\n");
	printf("|                             |\r\n");
	printf("|_____________________________|\r\n");
	//**********************************************************

	/* Ce code initialise l'adc en dma*/
	distSensor_TaskCreate(NULL);
	printf("Démarrage du test des capteurs de distance...\r\n");

	//**********************************************************

	/* Code init l'accélérometre*/

	ADXL343_TaskCreate(NULL);
	printf("adxl task creat \r\n");

	//**********************************************************
	DCMotor_CreateTask(&DualDrive_handle);
	printf("dcmotor task creat \r\n");

	//**********************************************************
	// Init SCREEN OLED
	if(HAL_OK == SCREEN_SSD1306_Init(&hscreen1, &hi2c1))
	{
		SCREEN_SSD1306_DrawBitmap(&hscreen1, Nyan_115x64px, 115, 64, White);
		//SCREEN_SSD1306_DrawBitmap(&hscreen1, Jerry_50x64px, 120, 64, White);
		SCREEN_SSD1306_Update_Screen(&hscreen1);
	}
	//**********************************************************
	//LIDAR

	X4LIDAR_create_task(&X4LIDAR_handle);
	printf("lidar task creat \r\n");
	//**********************************************************

	RobotStrategy_CreateTask();

	//**********************************************************

	printf("Tasks creation finished... \r\n");
	//**********************************************************

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
...
}
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{

	X4LIDAR_HAL_UART_RxHalfCpltCallback(huart,&X4LIDAR_handle);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	shell_drv_uart_HAL_UART_RxCpltCallback(huart);
	X4LIDAR_HAL_UART_RxCpltCallback(huart,&X4LIDAR_handle);

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	shell_drv_uart_HAL_UART_TxCpltCallback(huart);
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	  DCMotor_EncoderCallback(htim ,&DualDrive_handle);


  /* USER CODE END Callback 1 */
}
````

  ## Pilote X4 LIDAR

  ### Aperçu
  Cette implémentation est un pilote pour le capteur X4 LIDAR, conçu pour fonctionner sur des systèmes embarqués utilisant FreeRTOS. Le pilote gère la communication avec le capteur LIDAR via UART et fournit des fonctionnalités pour la configuration du dispositif, le balayage et le traitement des données.
  
  ### Fonctionnalités
  - Communication UART avec le capteur X4 LIDAR
  - Récupération des informations du dispositif (modèle, firmware, version matérielle, numéro de série)
  - Opérations de balayage avec support DMA
  - Gestion des tâches FreeRTOS
  - Analyse et validation des en-têtes de réponse
  - Détection et traitement efficace des trames de données
  
  ### Dépendances
  - FreeRTOS
  - STM32 HAL (Hardware Abstraction Layer)
  - Bibliothèques C standard (stdlib.h, math.h)
  
  ### Fonctions Principales
  - `X4LIDAR_init`: Initialise le pilote LIDAR et effectue la configuration initiale
  - `X4LIDAR_start_scan`: Démarre le processus de balayage LIDAR
  - `X4LIDAR_get_device_info`: Récupère les informations du dispositif
  - `X4LIDAR_create_task`: Crée une tâche FreeRTOS pour les opérations LIDAR
  - `X4LIDAR_send_command`: Gère la transmission des commandes au capteur
  
  ### Exemple d'Utilisation
  main.c:
  ```c
  // Initialisation de la structure LIDAR
  X4LIDAR_handle_t lidar_handle;
  UART_HandleTypeDef huart3;  // Votre handle UART
  
  // Création de la tâche LIDAR
  X4LIDAR_create_task(&lidar_handle);
  /* Start scheduler */
  osKernelStart();

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{

	X4LIDAR_HAL_UART_RxHalfCpltCallback(huart,&X4LIDAR_handle);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	X4LIDAR_HAL_UART_RxCpltCallback(huart,&X4LIDAR_handle);
}

````
X4LIDAR_driver.c:
  ```c
//ensure the correct uart is used for the init
void X4LIDAR_task(void *argument)
{
    // Retrieve the LiDAR handle passed as argument
    X4LIDAR_handle_t *X4LIDAR_handle = (X4LIDAR_handle_t*) argument;

    // Initialize the LiDAR hardware
    X4LIDAR_init(X4LIDAR_handle, &huart3);
    X4LIDAR_start_scan(X4LIDAR_handle);
...
}
````
For the uart config : 

![image](https://github.com/user-attachments/assets/d79d6af8-9061-4248-a122-c5ccf1e52a77)

For the DMA config :

![image](https://github.com/user-attachments/assets/fb1ce63d-a179-4899-a914-27c800d36bd9)

## Robot Strategy

Ce fichier a pour but de centraliser et synchroniser les différentes fonctionalitées du robot. C'est dans ce fichier que le traitement des donnés du Lidar est fait pour déterminer la cible la plus proche.On y gère également les états chat/souris.


Idéalement il aurait fallu tout centraliser sur cette tache mais par manque de temps et pour s'assurer que les actions critiques telles que le freinage des moteurs se faisaient à temps, une partie à des actions s'effectuent dans les autres taches.

## DC Motor Driver
Ce fichier contient des fonctions pour controler les pwm envoyer aux deux moteurs. Il contient également les fonctions pour les encodeurs des moteurs.

Une petie rampe pour les pwm a été utilisé et l'acquisition de la vitesse des moteurs se fait toutes les secondes via le callback d'un timer.

## Shell

La shell est uniquement utilisé pour le debug étant donné que la connexion STLink n'est pas possible si le robot doit se déplacer. 
des fonctions peuvent y etre ajouté en appellant shell_add(). exemple d'utilisation :

  ```c
	shell_init(&h_shell);
	shell_add(&h_shell, "print_dist", print_lidar_distances,
			"print lidar buffer containing scanned distances");
	shell_add(&h_shell, "print_motor_speed", print_motor_speed,
		"print_motor_speed");
	shell_createShellTask(&h_shell);
	...
  /* Start scheduler */
  osKernelStart();
````
## L'écran OLED SSD1306
L'écran est uniquement utilisé pour le fun afin d'avoir un moyen de visualiser si le robot est en mode chat ou sours. La librairie utilisé à été légérement adapté pour prendre en charge l'affichage d'images.

![cat](https://github.com/user-attachments/assets/c3513fa3-4ee4-4a78-8b18-8f9ab8b6f54b)

## L'accéléromètre ADXL343
  
  Nous allons à présent établir le code pour l'accéléromètre ADXL343, il va permettre de déteter un TAP et donc un changement d'état du robot, lorsque ce dernier entre en collision avec un autre robot. Pour pouvoir communiquer avec l'ADXL343 nous allons utiliser la communication SPI, ainsi on va définir deux fonctions principale pour lire et ecrire dans les registres. 
  
  ### Paramètres
  
  Dans le header en plus de définir les différents registres qui seront utilisés dans le code, et les différents structures, on peut également définir des paramètres clés pour la détection du TAP tel que :
  
  ````ruby
  #define TAP_THRESHOLD 			0x20											
  #define TAP_DURATION 			0x10 										
  #define TAP_LATENT 				0xC8
  ````
  
  On définit alors le seuil de détection du tap ici 2g, la durée entre deux tap différents ici 10ms et la durée total d'un single TAP.
  
  ### Lire et Ecrire dans les registres
  
  Pour pouvoir lire dans les registres on doit d'abord activer le NSS, ensuite on envoie l'adresse à laquelle on veut lire la donnée, puis on lit la donnée associée à l'adresse du registre et enfin on désactive le NSS. 
  
  ````ruby
  void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length)
  ````
  
  De la même manière pour écrire dans un registre on active le NSS ensuite on transmet les données que que l'on veut transmettre dans le registre qui nous intéresse et puis on désactive le NSS.
  
  ````ruby
  void ADXL343_WriteRegister(uint8_t reg, uint8_t data)
  ````
  
  ### Initialisation de l'accéléromètre
  
  Pour l'initialisation de l 'accéléromètre, on commence tout d'abord en vérifiant que la connexion est corréctement établit, pour cela on vérifie que la valeur du registre ADXL343_REG_DEVID est bien 0xE5. Ensuite on configure les différents registres pour mettre l'accéléromètre en mode veille, définir le format des données, et activer le mode mesure.
  
  ````ruby
  int ADXL343_Init(ADXL343_Handle_t* handle)
  ````
  
  Ensuite on va spécifiquement configurer les seuils, la durée, et les axes pour détecter les tapotements. On va également activer les interruptions pour les événements de tapotement simple sur n'importe quel axe.
  
  ````ruby
  void ADXL343_Configure(ADXL343_Handle_t* handle)
  ````
  
  ### Détection du TAP
  
  Le but de l'accéléromètre est de détecter lorsque le robot entre en collision pour ensuite pouvoir changer d'état. Pour cela on va lire la valeur du registre ADXL343_REG_INT_SOURCE et lorsque le 6 eme bit est à 1 cela signifie que l'on a détecter un single TAP.
  
  ````ruby
  void ADXL343_DetectTap(ADXL343_Handle_t* handle)
  ````
  
  ### Détection du TAP dans une tâche
  
  Avec l'activation de freeRTOS le fonctionnement est géré dans une tâche périodique permettant une exécution asynchrone et une intégration facile dans un système temps réel.
  
  ````ruby
  void ADXL343_TaskCreate(void * unused)
  ````
  
  
  
  
  ## Capteurs de distance
  
  Dans cette partie on va expliquer le code pour les capteurs de distance. Les capteurs de distance vont permettre de détecter le vide et ainsi d'arréter les moteurs pour éviter que le robot tombe de la table. On va donc utiliser 4 capteurs Nord, West, Sud, Est on utilisera deux canaux ADC1 et ADC2 sur la STM32. ADC1 lit les capteurs de distance situés à l'ouest (west) et au nord et ADC2 lit les capteurs de distance situés au sud et à l'est. Chaque capteur est représenté par une structure(DistSensor_handle_t) qui contient des informations telles que l'état du capteur et les valeurs mesurées. Les données des ADC sont transférées en mémoire via DMA pour améliorer les performances et réduire la charge du CPU.
  
  ### Initialisation
  
  Pour l'initialisation on va démarrer les conversions ADC en mode DMA pour ADC1 et ADC2 et activer le timer associé htim6 pour synchroniser les conversions. On initialise les capteurs avec les valeurs par défaut.
  
  ````ruby
  void distSensor_initADC_DMA(void)
  ````
  
  ### Lire les valeurs de l'ADC
  
  On va lire les valeurs de l'adc pour pouvoir déterminer si la distance qu'on évalue de l'objet le plus proche est le vide ou pas. On établit deux fonctions l'une en mode polling tel qu'on effectue la lecture manuelle d'une valeur ADC en mode blocant. Mais on établie aussi la focntion de lecture des données en mode DMA. La lecture des données des buffers DMA  s'éffectue pour chaque capteur avec la mise à jour des états  en fonction d'un seuil (VOID_TRESHOLD). On vérifie si le capteur détecte un sol ou du vide .
  
  ````ruby
  uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc)
  ````
  
  ````ruby
  HAL_StatusTypeDef distSensor_ReadADC_DMA(void)
  ````
  
  On appelle la fonction CallBack lorsque la conversion ADC est terminée. 
  ````ruby
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
  ````
  
  ### Fonctionnement des capteurs de distance en tâche
  
  On commence par créer une fonction exécutée en tâche FreeRTOS pour gérer les capteurs en continu, cette fonction va lit les données des capteurs via la fonction évoquée précédemment et on vavérifier si un ou plusieurs capteurs détectent du vide. Si oui, on déclenche une procédure d'arrêt d'urgence en activant le freinage des moteurs, et on réinitialise leur vitesse à 0 après un délai. L'arret d'ugence s'effectue directement ici pour éviter d'éventuels délais/préemption qui pourrait ralentir l'arret.
  
  ### Note Bonus
  Attention, il faut penser à mettre le continuous conversion mode de l'adc en disable sinon le programme bloque.
  
  Autre point important, les adc sont sur 12 bits, il est donc préférable de configurer le dma pour le transfert de half-word i.e 16 bits. Cependant, il faut alors penser à définir le buffer du DMA comme uint16_t adc1_dma_buffer[ADC1_CHANNEL_COUNT];.
  
  En effet, si l'on défini le buffer avec des uint32_t, les valeurs de 2 channels sur 16 bits se retrouveront concaténées sur une valeur de 32 bits. Pour des raisons obsucre il y avait encore des bugs de cette facon.
  
  En mettant les transferts du dma en format word avec  uint32_t adc1_dma_buffer[ADC1_CHANNEL_COUNT] cela semble régler le problème.
