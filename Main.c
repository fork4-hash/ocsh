#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtins.h"
#include "pipes_signals.h"

#define MAX_LINE 1024
#define MAX_ARGS 64

static int split_args(char *cmd_str, char **argv, int max_args) {
    int argc = 0;
    char *token = strtok(cmd_str, " \t\r\n");

    while (token != NULL && argc < max_args - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    argv[argc] = NULL;
    return argc;
}

/* Vérifie si args contient une redirection <, > ou >> */
static int a_redirection_args(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0 ||
            strcmp(args[i], ">>") == 0 ||
            strcmp(args[i], "<") == 0) {
            return 1;
        }
    }
    return 0;
}

/* Exécute une commande (builtin ou externe) dans un processus enfant avec redirections */
static int executer_avec_redirections_enfant(char **args) {
    char *fichier_in = NULL;
    char *fichier_out = NULL;
    int append_mode = 0;
    char *cmd_args[MAX_ARGS];
    int idx = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">>") == 0 && args[i + 1] != NULL) {
            fichier_out = args[++i];
            append_mode = 1;
        } else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
            fichier_out = args[++i];
            append_mode = 0;
        } else if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL) {
            fichier_in = args[++i];
        } else {
            cmd_args[idx++] = args[i];
        }
    }
    cmd_args[idx] = NULL;

    if (cmd_args[0] == NULL) {
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("ocsh: fork");
        return 1;
    }

    if (pid == 0) {
        if (fichier_in != NULL) {
            int fd_in = open(fichier_in, O_RDONLY);
            if (fd_in < 0) {
                perror("ocsh: open (<)");
                _exit(1);
            }
            if (dup2(fd_in, STDIN_FILENO) < 0) {
                perror("ocsh: dup2 (<)");
                close(fd_in);
                _exit(1);
            }
            close(fd_in);
        }

        if (fichier_out != NULL) {
            int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
            int fd_out = open(fichier_out, flags, 0644);
            if (fd_out < 0) {
                perror("ocsh: open (>/>>)");
                _exit(1);
            }
            if (dup2(fd_out, STDOUT_FILENO) < 0) {
                perror("ocsh: dup2 (>/>>)");
                close(fd_out);
                _exit(1);
            }
            close(fd_out);
        }

        int st = executer_builtin(cmd_args);
        if (st != 0) {
            _exit(0);
        }

        execvp(cmd_args[0], cmd_args);
        perror("ocsh: execvp");
        _exit(1);
    }

    waitpid(pid, NULL, 0);
    return 1;
}

int main(void) {
    char command[MAX_LINE];

    init_aliases();
    configurer_signaux();

    while (1) {
        printf("ocsh> ");
        fflush(stdout);

        if (fgets(command, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break;
        }

        command[strcspn(command, "\r\n")] = '\0';

        if (strlen(command) == 0) {
            continue;
        }

        enregistrer_historique(command);

        char *argv[MAX_ARGS];
        int argc = split_args(command, argv, MAX_ARGS);

        if (argc == 0) {
            continue;
        }

        char cmd_alias[MAX_LINE];
        char *args_alias[MAX_ARGS];
        char **args = argv;

        if (resoudre_alias(args, cmd_alias, sizeof(cmd_alias))) {
            int n = split_args(cmd_alias, args_alias, MAX_ARGS);
            if (n > 0) {
                args = args_alias;
            }
        }

        int arriere_plan = verifier_arriere_plan(args);
        if (args[0] == NULL) {
            continue;
        }

        if (contient_pipe(args)) {
            executer_pipe_perso(args);
            continue;
        }

        if (a_redirection_args(args)) {
            executer_avec_redirections_enfant(args);
            continue;
        }

        int status_builtin = executer_builtin(args);
        if (status_builtin < 0) {
            printf("Fermeture d'ocsh...\n");
            break;
        }
        if (status_builtin > 0) {
            continue;
        }

        if (gerer_redirections(args)) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Erreur lors du fork");
            continue;
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("ocsh: commande introuvable");
            _exit(1);
        } else {
            if (arriere_plan) {
                printf("[Arrière-plan] PID %d\n", pid);
            } else {
                waitpid(pid, NULL, 0);
            }
        }
    }

    return 0;
}