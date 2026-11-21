#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtins.h"
#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_STAGES 16

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

/* Découpe la ligne complète selon l'opérateur de pipe personnalisé "->" */
static int split_stages(char *line, char **stages, int max_stages) {
    int count = 0;
    char *cursor = line;

    while (cursor != NULL && count < max_stages) {
        stages[count++] = cursor;
        char *sep = strstr(cursor, "->");
        if (sep != NULL) {
            *sep = '\0';      /* Termine la commande courante */
            cursor = sep + 2; /* Avance après "->" */
        } else {
            cursor = NULL;
        }
    }
    return count;
}
int main(void) {
    char command[MAX_LINE];
    init_aliases();
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

        /* Découpage par "->" (préparation Jalon 1 / Jalon 3) */
        char *stages[MAX_STAGES];
        int num_stages = split_stages(command, stages, MAX_STAGES);

        char *argv[MAX_ARGS];
        int argc = split_args(stages[0], argv, MAX_ARGS);

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
        if (executer_builtin(args)) {
            continue;
        }
        if (gerer_redirections(args)) {
            continue;
        }
        /* Note : num_stages > 1 sera câblé avec pipe() au Jalon 3 */
        if (num_stages > 1) {
            printf("Info : %d étapes détectées avec '->' (câblage pipe au jalon 3)\n", num_stages);
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
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
    }