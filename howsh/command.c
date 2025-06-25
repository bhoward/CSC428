#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"

// Shell builtins

/**
 * @brief Change the current working directory.
 * 
 * @param args Array of command line arguments. The first argument is
 * used as the new directory (absolute or relative to the current working
 * directory); if absent, use the environment variable HOME instead.
 * 
 * @return False (this is not the exit command)
 */
static bool builtin_cd(char **args)
{
    char *dir = args[1];

    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "cd: expected directory name");
            return false;
        }
    }

    if (chdir(dir) != 0) {
        perror("cd");
    }

    return false;
}

/**
 * @brief Exit from the shell.
 * 
 * @param args Array of command line arguments (unused).
 * 
 * @return True (this is the exit command)
 */
static bool builtin_exit(char **args)
{
    return true;
}

typedef struct {
    char *name;
    bool (*function)(char **);
} builtin_t;

static builtin_t builtins[] = {
    {"cd", &builtin_cd},
    {"exit", &builtin_exit}
};

static const int NUM_BUILTINS = sizeof(builtins) / sizeof(builtin_t);

bool is_builtin(char **command) {
    for (int i = 0; i < NUM_BUILTINS; i++) {
        if (strcmp(command[0], builtins[i].name) == 0) {
            return true;
        }
    }

    return false;
}

bool execute_builtin(char **command) {
    for (int i = 0; i < NUM_BUILTINS; i++) {
        if (strcmp(command[0], builtins[i].name) == 0) {
            return builtins[i].function(command);
        }
    }

    return false;
}
