#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include <sys/wait.h>
#include "builtins.h"

static Alias table_aliases[MAX_ALIASES];
static int nb_aliases = 0;

static char historique[MAX_HISTORY][MAX_ALIAS_LEN];
static int nb_historique = 0;

static void ajouter_ou_modifier_alias(const char *nom, const char *cmd) {
    for (int i = 0; i < nb_aliases; i++) {
        if (strcmp(table_aliases[i].nom, nom) == 0) {
            strncpy(table_aliases[i].commande, cmd, MAX_ALIAS_LEN - 1);
            table_aliases[i].commande[MAX_ALIAS_LEN - 1] = '\0';
            return;
        }
    }
    if (nb_aliases < MAX_ALIASES) {
        strncpy(table_aliases[nb_aliases].nom, nom, MAX_ALIAS_LEN - 1);
        table_aliases[nb_aliases].nom[MAX_ALIAS_LEN - 1] = '\0';
        strncpy(table_aliases[nb_aliases].commande, cmd, MAX_ALIAS_LEN - 1);
        table_aliases[nb_aliases].commande[MAX_ALIAS_LEN - 1] = '\0';
        nb_aliases++;
    } else {
        fprintf(stderr, "ocsh: table des alias pleine\n");
    }
}

void init_aliases(void) {
    nb_aliases = 0;
    ajouter_ou_modifier_alias("list", "ls");
    ajouter_ou_modifier_alias("show", "cat");
    ajouter_ou_modifier_alias("copy", "cp");
    ajouter_ou_modifier_alias("move", "mv");
    ajouter_ou_modifier_alias("del", "rm");
    ajouter_ou_modifier_alias("newdir", "mkdir");
    ajouter_ou_modifier_alias("newfile", "touch");
}

void enregistrer_historique(const char *ligne) {
    if (ligne == NULL || ligne[0] == '\0') {
        return;
    }
    if (nb_historique < MAX_HISTORY) {
        strncpy(historique[nb_historique], ligne, MAX_ALIAS_LEN - 1);
        historique[nb_historique][MAX_ALIAS_LEN - 1] = '\0';
        nb_historique++;
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            strcpy(historique[i - 1], historique[i]);
        }
        strncpy(historique[MAX_HISTORY - 1], ligne, MAX_ALIAS_LEN - 1);
        historique[MAX_HISTORY - 1][MAX_ALIAS_LEN - 1] = '\0';
    }
}

static void afficher_historique(void) {
    for (int i = 0; i < nb_historique; i++) {
        printf("%d  %s\n", i + 1, historique[i]);
    }
}

int resoudre_alias(char **args, char *buffer_cmd, size_t taille_buffer) {
    if (args == NULL || args[0] == NULL) {
        return 0;
    }
    for (int i = 0; i < nb_aliases; i++) {
        if (strcmp(table_aliases[i].nom, args[0]) == 0) {
            snprintf(buffer_cmd, taille_buffer, "%s", table_aliases[i].commande);
            for (int j = 1; args[j] != NULL; j++) {
                strncat(buffer_cmd, " ", taille_buffer - strlen(buffer_cmd) - 1);
                strncat(buffer_cmd, args[j], taille_buffer - strlen(buffer_cmd) - 1);
            }
            return 1;
        }
    }
    return 0;
}

int executer_builtin(char **args) {
    if (args == NULL || args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "quit") == 0) {
        return -1;
    }

    if (strcmp(args[0], "go") == 0) {
        const char *cible = args[1];
        if (cible == NULL) {
            cible = getenv("HOME");
        }
        if (cible == NULL || chdir(cible) != 0) {
            perror("ocsh: go");
        }
        return 1;
    }

    if (strcmp(args[0], "path") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("ocsh: path");
        }
        return 1;
    }

    if (strcmp(args[0], "say") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            if (i > 1) {
                printf(" ");
            }
            printf("%s", args[i]);
        }
        printf("\n");
        return 1;
    }

    if (strcmp(args[0], "osinfo") == 0) {
        printf("Systèmes d'Exploitation -- UIR/ESIN\n");
        return 1;
    }

    if (strcmp(args[0], "info") == 0) {
        printf("Builtins : go, quit, path, say, info, past, osinfo, alias\n");
        printf("Alias    : list (ls), show (cat), copy (cp), move (mv), del (rm), newdir (mkdir), newfile (touch)\n");
        printf("Ops      : >, >>, <, -> (pipe), & (arriere-plan)\n");
        return 1;
    }

    if (strcmp(args[0], "past") == 0) {
        afficher_historique();
        return 1;
    }

    if (strcmp(args[0], "alias") == 0) {
        if (args[1] == NULL || args[2] == NULL) {
            fprintf(stderr, "Usage: alias <nom> <commande...>\n");
            return 1;
        }
        char cmd_reconstruite[MAX_ALIAS_LEN] = "";
        for (int i = 2; args[i] != NULL; i++) {
            if (i > 2) {
                strncat(cmd_reconstruite, " ", MAX_ALIAS_LEN - strlen(cmd_reconstruite) - 1);
            }
            strncat(cmd_reconstruite, args[i], MAX_ALIAS_LEN - strlen(cmd_reconstruite) - 1);
        }
        ajouter_ou_modifier_alias(args[1], cmd_reconstruite);
        return 1;
    }

    return 0;
}

int gerer_redirections(char **args) {
    char *fichier_in = NULL;
    char *fichier_out = NULL;
    int append_mode = 0;
    char *cmd_args[64];
    int idx = 0;
    int a_redirection = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">>") == 0 && args[i + 1] != NULL) {
            fichier_out = args[++i];
            append_mode = 1;
            a_redirection = 1;
        } else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
            fichier_out = args[++i];
            append_mode = 0;
            a_redirection = 1;
        } else if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL) {
            fichier_in = args[++i];
            a_redirection = 1;
        } else {
            cmd_args[idx++] = args[i];
        }
    }
    cmd_args[idx] = NULL;

    if (!a_redirection || cmd_args[0] == NULL) {
        return 0;
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
                exit(1);
            }
            if (dup2(fd_in, STDIN_FILENO) < 0) {
                perror("ocsh: dup2 (<)");
                close(fd_in);
                exit(1);
            }
            close(fd_in);
        }

        if (fichier_out != NULL) {
            int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
            int fd_out = open(fichier_out, flags, 0644);
            if (fd_out < 0) {
                perror("ocsh: open (>/>>)");
                exit(1);
            }
            if (dup2(fd_out, STDOUT_FILENO) < 0) {
                perror("ocsh: dup2 (>/>>)");
                close(fd_out);
                exit(1);
            }
            close(fd_out);
        }

        execvp(cmd_args[0], cmd_args);
        perror("ocsh: execvp");
        exit(1);
    }

    wait(NULL);
    return 1;
}