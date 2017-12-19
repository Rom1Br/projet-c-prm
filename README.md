# Cours ESSEC Langage C - Projet simulation

## Equipe

Marie Gorin - Romain Bruguière - Paul de Nonancourt

## Utilisation

Pour tester le projet, il est possible de le compiler à partir du fichier source dans le dossier `src/`.

Un `Makefile` a été inclus pour créer l'exécutable dans le dossier `bin/` à l'aide de la commande suivante :

```
make
```

## Remarques

- La librairie `ncurses` est utilisée pour l'affichage du jeu.
- Les données sont chargées depuis l'API gratuite de Quandl (voir fonction `download_data` du fichier `main.c`).
Ces données sont légèrement différentes de celles trouvées au départ sur le site Yahoo Finance et les données du 8 novembre sont manquantes sur la version gratuite.
L'API de Quandl ne nécessite cependant pas de passer par un token pour contourner la redirection.
