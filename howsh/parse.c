#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "parse.h"
#include "util.h"

/**
 * @brief Parse a single command (one or more words, none of which start
 * with pipe or redirection symbols: |, <, >).
 * 
 * @param currentp Pointer (reference parameter) to the current position in
 * a null-terminated array of strings.
 * 
 * @return Pointer to the first word of the command, or NULL if not found.
 */
static char **parse_command(char ***currentp);

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
static char *parse_redirect(char ***currentp, char direct);

/**
 * @brief Check whether a word is part of a command or if it indicates a
 * pipe or redirection.
 * 
 * @param word The word to check. Assumed to be nonempty.
 * 
 * @return true if the word does not start with '|', '<', or '>'
 */
static bool is_ordinary(char *word);

char **split_words(char *line)
{
    vector_t words;
    vector_init(&words);

    char *current = line;

    // skip leading whitespace
    while (isspace(*current)) current++;

    // Invariant: buffer[numwords] is always a valid location
    while (*current) {
        vector_add(&words, current);

        // scan past characters of current word
        while (*current && !isspace(*current)) current++;

        if (*current) {
            // Replace whitespace at end of word with null character
            *current = '\0';
            current++;
        }

        // skip additional whitespace
        while (isspace(*current)) current++;
    }

    return (char **) words.data;
}

pipeline_t parse_pipeline(char **words)
{
    pipeline_t pipeline;
    init_pipeline(&pipeline);

    vector_t commands;
    vector_init(&commands);

    char **current = words;

    vector_add(&commands, parse_command(&current));

    pipeline.redirect_input = parse_redirect(&current, '<');

    while (*current && strcmp(*current, "|") == 0) {
        *current = NULL;
        current++;

        vector_add(&commands, parse_command(&current));
    }

    pipeline.commands = (command_t *) commands.data;

    pipeline.redirect_output = parse_redirect(&current, '>');

    return pipeline;
}

static char **parse_command(char ***currentp)
{
    char **start = *currentp;

    // scan past words of current command
    while (**currentp && is_ordinary(**currentp)) (*currentp)++;

    if (start == *currentp) {
        fprintf(stderr, "Empty command\n");
        return NULL;
    }

    return start;
}

static char *parse_redirect(char ***currentp, char direct)
{
    char *result = NULL;

    if (**currentp && (**currentp)[0] == direct) {
        if (strlen(**currentp) == 1) {
            // space before file name
            **currentp = NULL;
            (*currentp)++;

            if (**currentp) {
                result = **currentp;
                (*currentp)++;
            } else {
                fprintf(stderr, "Missing file name after redirection\n");
            }
        } else {
            // no space before file name
            result = **currentp + 1;
            **currentp = NULL;
            (*currentp)++;
        }
    }

    return result;
}

static bool is_ordinary(char *word)
{
    return strchr("|<>", word[0]) == NULL;
}
