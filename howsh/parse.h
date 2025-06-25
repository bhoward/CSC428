#ifndef _PARSE_H
#define _PARSE_H

#include <stdbool.h>

#include "pipeline.h"

/**
 * @brief Divide the given string into whitespace-separated words.
 * 
 * @param line The null-terminated string to be split.
 * 
 * @return A pointer to an array of zero or more strings (char*), where
 * the last entry will be followed by a NULL pointer. It is the caller's
 * responsibility to free this array when done.
 */
char **split_words(char *line);

/**
 * @brief Recognize a series of commands separated by pipes, with optional
 * redirection of standard input for the first command and standard output
 * for the last.
 * 
 * @param words A null-terminated array of strings.
 * 
 * @return A struct containing the list of commands and the filenames for
 * redirecting output (see documentation for pipeline_t). Note that it is
 * the caller's responsibility to free the array of commands when done, by
 * calling free_pipeline().
 */
pipeline_t parse_pipeline(char **words);

/**
 * @brief Parse a single command (one or more words, none of which start
 * with pipe or redirection symbols: |, <, >).
 * 
 * @param currentp Pointer (reference parameter) to the current position in
 * a null-terminated array of strings.
 * 
 * @return Pointer to the first word of the command, or NULL if not found.
 */
char **parse_command(char ***currentp);

/**
 * @brief Parse a redirection following a command, either `< filename` or
 * `> filename`.
 * 
 * @param currentp Pointer (reference parameter) to the current position in
 * a null-terminated array of strings.
 * 
 * @param direct The redirection character, either '<' (input) or '>' (output).
 * 
 * @return The file name to redirect to, or NULL. The previous command will be
 * terminated with a NULL pointer, replacing the start of the redirection.
 */
char *parse_redirect(char ***currentp, char direct);

/**
 * @brief Check whether a word is part of a command or if it indicates a
 * pipe or redirection.
 * 
 * @param word The word to check. Assumed to be nonempty.
 * 
 * @return true if the word does not start with '|', '<', or '>'
 */
bool is_ordinary(char *word);

#endif