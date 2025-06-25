#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "util.h"

static const char *PROG_NAME = "howsh";

static const int INIT_BUFFER_SIZE = 10;

static const char *PROMPT = "> ";

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

void print_error(char *where) {
    fprintf(stderr, "%s (%s): %s\n", PROG_NAME, where, strerror(errno));
}

void vector_init(vector_t *vector)
{
    vector->size = 0;
    vector->capacity = INIT_BUFFER_SIZE;
    vector->data = malloc(vector->capacity * sizeof(*vector->data));
    if (!vector->data) {
        print_error("vector_init");
        exit(EXIT_FAILURE);
    }

    // Ensure that one past the last item is a null pointer
    vector->data[vector->size] = NULL;
}

void vector_add(vector_t *vector, void *item)
{
    vector->data[vector->size] = item;
    vector->size++;

    if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        vector->data = realloc(vector->data, vector->capacity * sizeof(*vector->data));
        if (!vector->data) {
            print_error("vector_add");
            exit(EXIT_FAILURE);
        }
    }

    vector->data[vector->size] = NULL;
}

void* vector_get(vector_t *vector, size_t index)
{
    return vector->data[index];
}

void vector_free(vector_t *vector)
{
    free(vector->data);
}