#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "command.h"
#include "pipeline.h"
#include "parse.h"
#include "util.h"

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
        done = execute_pipeline(&pipeline);

        // Execution is finished so clean up this line
        free_pipeline(&pipeline);
        free(words);
        free(line);
    }

    // Cleanup
    if (input != stdin) {
        fclose(input);
    }

    return EXIT_SUCCESS;
}
