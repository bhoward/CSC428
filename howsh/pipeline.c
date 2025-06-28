#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "pipeline.h"
#include "util.h"

/**
 * @brief Start the given command with provided I/O.
 * 
 * @param command A NULL terminated array of strings.
 * `command[0]` is the name of the command, and the rest
 * of the array is its arguments.
 * 
 * @param in The file descriptor to be used for the command's
 * standard input.
 * 
 * @param out The file descriptor to be used for the command's
 * standard output.
 */
static void start_child(char **command, int in, int out);

void init_pipeline(pipeline_t *pipeline)
{
    pipeline->redirect_input = NULL;
    pipeline->redirect_output = NULL;
    pipeline->commands = NULL;
}

bool execute_pipeline(pipeline_t *pipeline)
{
    if (pipeline->commands[0] == NULL) {
        return false;
    }

    // A builtin must be the only command in the pipeline
    if (is_builtin(pipeline->commands[0]) && !pipeline->commands[1]) {
        return execute_builtin(pipeline->commands[0]);
    }

    int in = STDIN_FILENO;
    if (pipeline->redirect_input) {
        in = open(pipeline->redirect_input, O_RDONLY);

        if (in == -1) {
            print_error("execute_pipeline");
            return false;
        }
    }

    int out = STDOUT_FILENO;
    if (pipeline->redirect_output) {
        out = open(pipeline->redirect_output, O_CREAT | O_TRUNC | O_WRONLY);

        if (out == -1) {
            print_error("execute_pipeline");
            return false;
        }
    }

    pid_t pid = 0;
    for (char ***p = pipeline->commands; *p; p++) {
        int current_out = out;
        int next_in = STDIN_FILENO;

        if (*(p + 1)) {
            // Not the last command in the pipeline yet
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1) {
                print_error("execute_pipeline");
                return false;
            }

            current_out = pipe_fd[1];
            next_in = pipe_fd[0];
        }

        pid = fork();
        if (pid == 0) {
            // Successful child
            start_child(*p, in, current_out);
        } else if (pid < 0) {
            // Error in fork
            print_error("execute_pipeline");
            return false;
        }

        // Parent; close unneeded fds
        if (in != STDIN_FILENO) {
            close(in);
        }
        if (current_out != STDOUT_FILENO) {
            close(current_out);
        }

        in = next_in;
    }

    if (in != STDIN_FILENO) {
        close(in);
    }
    if (out != STDOUT_FILENO) {
        close(out);
    }

    // Wait for last command
    if (pid > 0) {
        int status;

        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return false;
}

static void start_child(char **command, int in, int out)
{
    if (in != STDIN_FILENO) {
        if (dup2(in, STDIN_FILENO) == -1) {
            print_error("start_child");
            exit(EXIT_FAILURE);
        }
        close(in);
    }

    if (out != STDOUT_FILENO) {
        if (dup2(out, STDOUT_FILENO) == -1) {
            print_error("start_child");
            exit(EXIT_FAILURE);
        }
        close(out);
    }

    if (execvp(*command, command) == -1) {
        print_error("start_child");
        exit(EXIT_FAILURE);
    }
}

void free_pipeline(pipeline_t *pipeline)
{
    free(pipeline->commands);
}
