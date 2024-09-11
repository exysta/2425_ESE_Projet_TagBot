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

### Le materiel

L'essentiel du materiel nous a été imposé, mais nous devons trouver par nous même le moyen par lequel nous voulons détetecter les bords pour ne pas tomber de la table. 
Pour cela on aimerait utiliser les capteurs IR ce capteur sera placer sous le robot et recevra donc une tension qui déterminera sa distance par rapport au sol, quand ce signal diminue fortement (donc la distance augmente) cela signifie qu'on est sur un bord. Comme il sera sous le robot il nous faut un capteur qui est capable de detecté des petites distances comme le capteur sharp GP2Y0A21SK0F. 

On aimerai aussi ajouter une led visible qui nous permettrait de savoir quel est l'état de notre robot chat.
