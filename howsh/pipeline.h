#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <stdbool.h>

#include "command.h"

/**
 * Represents a list of commands joined in a pipeline, with
 * optional redirection of standard input to the first command
 * and standard output from the last command. Each command
 * is a list of words, terminated with a NULL pointer; the list
 * of commands is itself terminated with a NULL pointer as well.
 * The redirect fields are either file names or NULL for default.
 */
typedef struct {
    char *redirect_input;
    char *redirect_output;
    command_t *commands;
} pipeline_t;

/**
 * @brief Initialize the fields of a `pipeline_t` struct.
 * 
 * @param pipeline The `pipeline_t` struct to be initialized.
 */
void init_pipeline(pipeline_t *pipeline);

/**
 * @brief Create child processes and connect pipes to execute the given pipeline.
 * 
 * @param pipeline The `pipeline_t` struct to be executed.
 * 
 * @return true if the shell should be exited after executing this pipeline.
 */
bool execute_pipeline(pipeline_t *pipeline);

/**
 * @brief Free up the memory allocated for this pipeline.
 * 
 * @param pipeline The `pipeline_t` struct to be freed.
 */
void free_pipeline(pipeline_t *pipeline);

#endif