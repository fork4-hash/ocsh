#ifndef BUILTINS_H
#define BUILTINS_H

#include <stddef.h>

#define MAX_ALIASES 64
#define MAX_ALIAS_LEN 256
#define MAX_HISTORY 100

typedef struct {
    char nom[MAX_ALIAS_LEN];
    char commande[MAX_ALIAS_LEN];
} Alias;

void init_aliases(void);
void enregistrer_historique(const char *ligne);
int resoudre_alias(char **args, char *buffer_cmd, size_t taille_buffer);
int executer_builtin(char **args);
int gerer_redirections(char **args);

#endif