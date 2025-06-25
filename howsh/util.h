#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>

/**
 * @brief Utility function to print an error message.
 * 
 * @param where Name of the function where the error has occured.
 */
void print_error(char *where);

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
 * Dynamically resizeable array, like a C++ vector.
 */
typedef struct {
    size_t size;
    size_t capacity;
    void **data;
} vector_t;

/**
 * @brief Initialize the given vector.
 * 
 * @param vector Pointer to the `vector_t` to be initialized.
 */
void vector_init(vector_t *vector);

/**
 * @brief Append a new element to the given vector.
 * 
 * @param vector Pointer to the `vector_t` to be appended. The
 * capacity will be enlarged to contain all of the appended items,
 * plus a NULL pointer after the last item.
 * 
 * @param item The item to be added (passed as a `void *`).
 */
void vector_add(vector_t *vector, void *item);

// TODO the following are not currently used -- we just append to
// a vector and then extract its data field for further use...
void* vector_get(vector_t *vector, size_t index);

void vector_free(vector_t *vector);

#endif