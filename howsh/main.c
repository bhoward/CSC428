#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static const char *PROMPT = "> ";
static const char *PROG_NAME = "howsh";
static const int INIT_BUFFER_SIZE = 10;

// Shell builtins
bool builtin_cd(char **args);
bool builtin_exit(char **args);

typedef struct {
    char *name;
    bool (*function)(char **);
} builtin_t;

builtin_t builtins[] = {
    {"cd", &builtin_cd},
    {"exit", &builtin_exit}
};

static const int NUM_BUILTINS = sizeof(builtins) / sizeof(builtin_t);

bool builtin_cd(char **args)
{
    if (args[1] == NULL) {
        args[1] = getenv("HOME");
        if (args[1] == NULL) {
            fprintf(stderr, "cd: expected directory name");
            return false;
        }
    }

    if (chdir(args[1]) != 0) {
        perror("cd");
    }

    return false;
}

bool builtin_exit(char **args)
{
    return true;
}

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
    char ***commands;
} pipeline_t;

/**
 * @brief Print a prompt to the user on standard output.
 */
void print_prompt();

/**
 * @brief Read one line from the given input stream.
 * 
 * @param input The input stream.
 * 
 * @return A null-terminated string (char*). It is the caller's
 * responsibility to free this buffer when done.
 */
char *read_line(FILE *input);

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

/**
 * @brief Create child processes and connect pipes to execute the given pipeline.
 * 
 * @param pipeline The `pipeline_t` struct to be executed.
 * 
 * @return true if the shell should be exited after executing this pipeline.
 */
bool execute_pipeline(pipeline_t pipeline);

/**
 * @brief Check whether the given command is a builtin.
 * 
 * @param command Null-terminated list of words for the command.
 * 
 * @return true if command name (first word) is in the list of known builtins.
 */
bool is_builtin(char **command);

/**
 * @brief Execute the given builtin command.
 * 
 * @param command Null-terminated list of words for the command.
 * 
 * @return true if the shell should be exited after executing this command.
 */
bool execute_builtin(char **command);

/**
 * @brief Free up the memory allocated for the commands of this pipeline.
 * 
 * @param pipeline The `pipeline_t` struct to be freed.
 */
void free_pipeline(pipeline_t pipeline);

/**
 * @brief Utility function to print an error message.
 * 
 * @param where Name of the function where the error has occured.
 */
void print_error(char *where);

int main(int argc, char **argv)
{
    // Initialize and process arguments
    FILE *input = stdin;
    
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            print_error("main");
            return EXIT_FAILURE;
        }
    }

    // Main loop
    bool done = false;
    while (!done) {
        if (input == stdin) print_prompt();

        char *line = read_line(input);
        if (line == NULL) {
            printf("\n");
            break;
        }

        char **words = split_words(line);
        if (*words == NULL) continue;

        pipeline_t pipeline = parse_pipeline(words);
        done = execute_pipeline(pipeline);

        // Execution is finished so clean up this line
        free_pipeline(pipeline);
        free(words);
        free(line);
    }

    // Cleanup
    if (input != stdin) {
        fclose(input);
    }

    return EXIT_SUCCESS;
}

void print_prompt()
{
    printf("%s", PROMPT);
}

char *read_line(FILE *input)
{
    char *line = NULL;
    size_t buffer_size = 0;

    if (getline(&line, &buffer_size, input) == -1) {
        if (feof(input)) {
            // EOF detected
            return NULL;
        } else {
            print_error("read_line");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

char **split_words(char *line)
{
    int buffer_size = INIT_BUFFER_SIZE;
    char **buffer = malloc(buffer_size * sizeof(*buffer));
    if (!buffer) {
        print_error("split_words");
        exit(EXIT_FAILURE);
    }

    int numwords = 0;
    char *current = line;

    // skip leading whitespace
    while (isspace(*current)) current++;

    // Invariant: buffer[numwords] is always a valid location
    while (*current) {
        buffer[numwords] = current;
        numwords++;

        if (numwords >= buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size * sizeof(*buffer));
            if (!buffer) {
                print_error("split_words");
                exit(EXIT_FAILURE);
            }
        }

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

    buffer[numwords] = NULL;

    return buffer;
}

pipeline_t parse_pipeline(char **words)
{
    pipeline_t pipeline;
    pipeline.redirect_input = NULL;
    pipeline.redirect_output = NULL;

    int buffer_size = INIT_BUFFER_SIZE;
    pipeline.commands = malloc(buffer_size * sizeof(*pipeline.commands));
    if (!pipeline.commands) {
        print_error("parse_pipeline");
        exit(EXIT_FAILURE);
    }

    char **current = words;
    int numcommands = 0;

    pipeline.commands[numcommands] = parse_command(&current);
    numcommands++;

    pipeline.redirect_input = parse_redirect(&current, '<');

    // Invariant: pipeline.commands[numcommands] is always a valid position
    while (*current && strcmp(*current, "|") == 0) {
        *current = NULL;
        current++;

        pipeline.commands[numcommands] = parse_command(&current);
        numcommands++;

        if (numcommands >= buffer_size) {
            buffer_size *= 2;
            pipeline.commands = realloc(pipeline.commands, buffer_size * sizeof(*pipeline.commands));
            if (!pipeline.commands) {
                print_error("parse_pipeline");
                exit(EXIT_FAILURE);
            }
        }
    }

    pipeline.commands[numcommands] = NULL;

    pipeline.redirect_output = parse_redirect(&current, '>');

    return pipeline;
}

char **parse_command(char ***currentp)
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

char *parse_redirect(char ***currentp, char direct)
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

bool is_ordinary(char *word)
{
    return strchr("|<>", word[0]) == NULL;
}

bool execute_pipeline(pipeline_t pipeline)
{
    if (pipeline.commands[0] == NULL) {
        return false;
    }

    if (is_builtin(pipeline.commands[0]) && !pipeline.commands[1]) {
        return execute_builtin(pipeline.commands[0]);
    }

    int in = STDIN_FILENO;
    if (pipeline.redirect_input) {
        in = open(pipeline.redirect_input, O_RDONLY);

        if (in == -1) {
            print_error("execute_pipeline");
            return false;
        }
    }

    int out = STDOUT_FILENO;
    if (pipeline.redirect_output) {
        out = open(pipeline.redirect_output, O_CREAT | O_TRUNC | O_WRONLY);

        if (out == -1) {
            print_error("execute_pipeline");
            return false;
        }
    }

    pid_t pid = 0;
    for (char ***p = pipeline.commands; *p; p++) {
        int current_out = out;
        int next_in = 0;

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

            if (in != STDIN_FILENO) {
                if (dup2(in, STDIN_FILENO) == -1) {
                    print_error("execute_pipeline");
                    exit(EXIT_FAILURE);
                }
                close(in);
            }

            if (current_out != STDOUT_FILENO) {
                if (dup2(current_out, STDOUT_FILENO) == -1) {
                    print_error("execute_pipeline");
                    exit(EXIT_FAILURE);
                }
                close(current_out);
            }

            if (execvp(**p, *p) == -1) {
                print_error("execute_pipeline");
                exit(EXIT_FAILURE);
            }
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

void print_error(char *where) {
    fprintf(stderr, "%s (%s): %s\n", PROG_NAME, where, strerror(errno));
}

void free_pipeline(pipeline_t pipeline)
{
    free(pipeline.commands);
}
