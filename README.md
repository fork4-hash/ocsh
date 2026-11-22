OCSH – Mini-shell en C

1. Présentation

OCSH est un mini-interpréteur de commandes développé en langage C dans le cadre du Projet 3 de Systèmes d’Exploitation à l’UIR / ESIN.

L’objectif est de construire un interpréteur de commandes en utilisant directement les appels système du noyau, notamment pour la création de processus, les redirections, les pipes et les signaux.

Le projet consiste également à implémenter des commandes renommées ainsi que plusieurs fonctionnalités inspirées de Bash.

2. Objectifs du projet

* Comprendre le fonctionnement d’un interpréteur de commandes.
* Manipuler les processus avec fork(), execvp() et wait().
* Implémenter des commandes intégrées personnalisées.
* Gérer les alias.
* Réaliser des redirections avec dup2().
* Mettre en place la communication entre processus avec pipe().
* Gérer les signaux et les processus en arrière-plan.

3. Commandes personnalisées

Commande Bash    Commande OCSH
cd    go
exit    quit
pwd    path
echo    say
help    info
history    past
ls    list
cat    show
cp    copy
mv    move
rm    del
mkdir    newdir
touch    newfile

Le shell doit également fournir la commande intégrée osinfo, qui affiche :

Systèmes d’Exploitation -- UIR/ESIN

L’opérateur de pipe utilisé dans OCSH est -> au lieu de |.

4. Jalons du projet

Jalon 1 : Boucle REPL

* Lecture des commandes.
* Découpage des commandes avec ->.
* Création de processus avec fork().
* Exécution avec execvp().
* Attente avec wait().

Jalon 2 : Builtins, alias et redirections

* Implémentation des commandes intégrées.
* Gestion des alias.
* Redirections >, >> et < avec dup2().

Jalon 3 : Pipes, arrière-plan et signaux

* Communication entre processus avec pipe().
* Exécution en arrière-plan avec &.
* Gestion des signaux SIGINT et SIGCHLD.

Jalon 4 : Variables et expansion

* Expansion de $VAR et ${VAR}.
* Commande export.
* Gestion des variables spéciales $? et $!.

Jalon 5 : Substitution et historique

* Substitution avec $(...) et les backticks.
* Fichier de configuration .ocshrc.
* Historique et autocomplétion.

Jalon 6 : Mode script

* Exécution de scripts .ocsh.
* Structures conditionnelles if.
* Boucles for et while.

Le job control (jobs, fg, bg) est optionnel.

5. Technologies utilisées

* Langage C
* Appels système Unix/Linux
* GCC
* Git et GitHub

6. Compilation et exécution

Les commandes de compilation et d’exécution seront précisées après la finalisation du Makefile et l’intégration des différents modules.

7. Tests

Le fichier test_commands.sh contient les tests automatisés destinés à vérifier le fonctionnement des commandes du shell.

Les tests seront complétés et validés après l’intégration du projet.

8. Livrables

* Code source C commenté.
* Makefile.
* README.md.
* report.pdf.
* demo.mp4 : vidéo de démonstration.
