#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtins.h"
#include "pipes_signals.h"

#define MAX_LINE 1024
#define MAX_ARGS 64

/* Découpe une chaîne en arguments séparés par espaces/tabulations */
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

int main(void) {
    char command[MAX_LINE];

    init_aliases();
    configurer_signaux();

    while (1) {
        printf("ocsh> ");
        fflush(stdout);

        if (fgets(command, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break; // Ctrl+D (EOF)
        }

        command[strcspn(command, "\r\n")] = '\0';

        if (strlen(command) == 0) {
            continue;
        }

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

        if (strcmp(args[0], "quit") == 0) {
            printf("Fermeture d'ocsh...\n");
            break;
        }

        int arriere_plan = verifier_arriere_plan(args);
        if (args[0] == NULL) {
            continue;
        }

        if (executer_builtin(args)) {
            continue;
        }

        if (contient_pipe(args)) {
            executer_pipe_perso(args);
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