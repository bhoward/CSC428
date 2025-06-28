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

#endif