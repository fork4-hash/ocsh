# ocsh — Mini-shell en C (Projet 3 — UIR/ESIN)

`ocsh` est un mini-shell POSIX développé en C dans le cadre du Projet 3 (Systèmes d'Exploitation — UIR/ESIN), en s'appuyant sur les appels système du noyau (`fork()`, `execvp()`, `waitpid()`, `pipe()`, `dup2()`, `open()`, `sigaction()`).

## Builtins et alias

* **Builtins :** `go`, `quit`, `path`, `say`, `info`, `past`, `osinfo`, `alias`
* **Alias préchargés :** `list` (`ls`), `show` (`cat`), `copy` (`cp`), `move` (`mv`), `del` (`rm`), `newdir` (`mkdir`), `newfile` (`touch`)
* **Opérateurs et signaux :** `<`, `>`, `>>`, `->` (pipe), `&` (arrière-plan), gestion de `SIGINT` et `SIGCHLD`

## Compilation et exécution

```bash
make
./ocsh
make clean


```
