#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "pipes_signals.h"

static void gestionnaire_sigint(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

static void gestionnaire_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
}

void configurer_signaux(void) {
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = gestionnaire_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = gestionnaire_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}

int contient_pipe(char **args) {
    if (args == NULL) {
        return 0;
    }
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "->") == 0) {
            return 1;
        }
    }
    return 0;
}

int executer_pipe_perso(char **args) {
    int pos_pipe = -1;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "->") == 0) {
            pos_pipe = i;
            break;
        }
    }

    if (pos_pipe <= 0 || args[pos_pipe + 1] == NULL) {
        return 0;
    }

    args[pos_pipe] = NULL;
    char **cmd_gauche = args;
    char **cmd_droite = &args[pos_pipe + 1];

    int fd[2];
    if (pipe(fd) < 0) {
        perror("ocsh: pipe");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("ocsh: fork (gauche)");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid1 == 0) {
        signal(SIGINT, SIG_DFL);
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) < 0) {
            perror("ocsh: dup2 (gauche)");
            close(fd[1]);
            exit(1);
        }
        close(fd[1]);
        execvp(cmd_gauche[0], cmd_gauche);
        perror("ocsh: execvp (gauche)");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("ocsh: fork (droite)");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0) {
        signal(SIGINT, SIG_DFL);
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) < 0) {
            perror("ocsh: dup2 (droite)");
            close(fd[0]);
            exit(1);
        }
        close(fd[0]);
        execvp(cmd_droite[0], cmd_droite);
        perror("ocsh: execvp (droite)");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 1;
}

int verifier_arriere_plan(char **args) {
    if (args == NULL) {
        return 0;
    }
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        args[i - 1] = NULL;
        return 1;
    }
    return 0;
}