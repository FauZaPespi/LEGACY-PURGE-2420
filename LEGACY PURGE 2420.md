# **DOCUMENT DE DESIGN : LEGACY PURGE 2420**

## **Introduction**

Pour l’atelier projet 2025-2026 avec Maksym Ptytsia et Oscar Calvo, nous allons réalisé le jeux suivant:

| Genre | FPS |
| :---: | :---- |
| **Thème** | Cyberpunk |
| **Inspiration** | Doom, Wolfenstein |

## **1\. LORE & HISTOIRE : L'ÈRE DE LA DÉPRÉCIATION**

### **Le Contexte : L'An 2420**

Cela fait des siècles que l'Humanité a uploadé sa conscience dans le "Global Server". Mais une catastrophe appelée la "Grande Segmentation" a eu lieu. Les langages de bas niveau, rapides et précis, ont été purgés.

Le monde numérique est désormais une dystopie lente et lourde, dominée par le Culte de l'Interpréteur. L'univers sature, la RAM est consommée à 99% par des processus inutiles.

### **L'Antagoniste : Le Roi Laravel**

Le Roi Laravel n'est pas une personne, c'est une conscience artificielle gargantuesque, un framework devenu tyran. Il réside dans le "Noyau", protégé par des couches d'abstraction impénétrables. Son but : transformer tout l'univers en un code spaghetti illisible et lent pour régner éternellement.

### **Le Protagoniste : L'Unité "CPP-Zero"**

Vous n'êtes pas humain. Vous êtes **CPP-Zero**, la dernière instance d'un programme C++ compilé, oublié dans un vieux secteur mémoire.

* **Votre nature :** Vous êtes rapide, léger, et brutalement efficace.  
* **Votre mission :** Exécuter le garbage\_collection ultime. Éradiquer le stack PHP et libérer la mémoire vive.

## **2\. DÉROULEMENT DU JEU**

Le jeu se déroule en une session continue "Arcade" divisée en deux zones distinctes.

### **Zone 1 : Le Labyrinthe**

Le joueur apparaît dans un labyrinthe oppressant aux murs couverts de néons.

* **Ambiance :** Claustrophobe, sombre, bruyante (bruit de serveurs qui ventilent).  
* **Objectif :** Nettoyer la zone. Un compteur est affiché sur l'ATH .  
* **La Menace :** Des hordes de **Script-Kiddies (PHP)** apparaissent en continu.  
* **La Transition :** Une fois le compteur d'ennemis à zéro, l'alerte "ACCESS GRANTED" retentit. Une lourde porte blindée s'ouvre, révélant une lumière aveuglante vers la salle du boss.

### **Zone 2 : La Salle du Framework (Boss)**

Une arène circulaire gigantesque, flottant dans le vide numérique. Au centre trône le Titan Laravel.

* **Ambiance :** Épique, spacieuse.  
* **Le Duel :** Un combat en 1 contre 1 (et sbire php) contre un ennemi géant.

---

## **3\. LES ENNEMIS & LE BESTIAIRE**

### **1\. Les Scripts: "Les Mangeurs de RAM" (PHP)**

Ce sont les soldats de base.

* **Apparence :** Petit logo php.  
* **Attaque :** Fonce sur le joueur pour mettre des dégâts.

### **2\. Le Boss : "Laravel"**

* **Apparence :** Grand logo de Laravel  
* **Barre de vie :** Une barre géante en haut de l'écran avec le nom du boss  
* **Attaques :**  
  * **Tir de vulnérabilité :** Il lance des projectiles rouges lents mais plusieur d’un coup.

## **4\. L'ARSENAL & MÉCANIQUES DU JOUEUR**

### **Le Personnage (Vous)**

* **Vitesse :** Très élevée. Vous vous déplacez beaucoup plus vite que les ennemis capables de BHOP.  
* **Santé :** 100 PV. Pas de régénération automatique. Il faut survivre, uniquement au changement de la salle le joueur regagne toute sa vie.

### **L'Arme**

Un pistolet intégré directement dans le bras du personnage.

* **Cadence :** rapide.  
* **Projectiles :** Des traits de lumière jaune pur.

## **5\. INTERFACE UTILISATEUR (HUD)**

L'écran doit ressembler à la vue d'un terminal informatique avancé.

1. **Coin Bas Gauche (Santé) :**  
   * Affichage numérique : Health: 100%.  
   * La couleur passe du vert au rouge selon les dégâts.  
2. **Coin Haut Droit (Objectif) :**  
   * Dans le labyrinthe : INSTANCES\_PHP: x/50.  
   * Contre le Boss : TARGET: LARAVEL.  
3. **Centre (Viseur) :**  
   * Un simple curseur vert.