#ifndef _COMMAND_H
#define _COMMAND_H

#include <stdbool.h>

/**
 * A command is a null-terminated array of strings.
 */
typedef char** command_t;

/**
 * @brief Check whether the given command is a builtin.
 * 
 * @param command Null-terminated list of words for the command.
 * 
 * @return true if command name (first word) is in the list of known builtins.
 */
bool is_builtin(command_t command);

/**
 * @brief Execute the given builtin command.
 * 
 * @param command Null-terminated list of words for the command.
 * 
 * @return true if the shell should be exited after executing this command.
 */
bool execute_builtin(command_t command);

#endif