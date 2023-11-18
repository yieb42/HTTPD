#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

FILE *log_file = NULL;

void log_message(const char *message) {
    if (log_file != NULL) {
        fprintf(log_file, "%s\n", message);
        fflush(log_file);
    }
}

void close_log_file() {
    if (log_file != NULL && log_file != stdout) {
        fclose(log_file);
    }
}

void setup_logging(bool log_to_stdout, const char *log_file_path) {
    FILE *logp = NULL;

    if (log_file_path != NULL && log_to_stdout == true) {
        logp = fopen(log_file_path, "a");
        if (logp == NULL) {
            printf("couldnt open file");
            return;
        }
    }

    else if (log_file_path == NULL && log_to_stdout == true) {
        logp = stdout;
    }

    log_file = logp;

    atexit(close_log_file);
}