#include "papagaio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (buf) {
        size_t rb = fread(buf, 1, sz, f);
        buf[rb] = '\0';
        if (out_len) *out_len = rb;
    }
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc >= 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
        printf("papagaio v%s\n", CONFIG_VERSION);
        return 0;
    }
    if (argc < 2 || (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        fprintf(stderr, "Usage: papagaio <file.txt> | -e \"code\" | -\n");
        return 1;
    }

    Papagaio *ctx = papagaio_open();
    papagaio_set_args(ctx, argc, argv);
    papagaio_set_cli_mode(ctx, 1);

    char *input = NULL;
    size_t len = 0;

    if (strcmp(argv[1], "-e") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: -e requires a string argument\n");
            papagaio_close(ctx);
            return 1;
        }
        input = strdup(argv[2]);
        len = strlen(input);
    } else if (strcmp(argv[1], "-") == 0) {
        /* Read from stdin */
        size_t cap = 4096;
        input = malloc(cap);
        while (1) {
            size_t r = fread(input + len, 1, cap - len - 1, stdin);
            len += r;
            if (r == 0) break;
            if (len + 1024 >= cap) {
                cap <<= 1;
                input = realloc(input, cap);
            }
        }
        input[len] = '\0';
    } else {
        input = read_file(argv[1], &len);
    }

    if (!input) {
        fprintf(stderr, "Error reading input\n");
        papagaio_close(ctx);
        return 1;
    }

    char *output = papagaio_process_text(ctx, input, len);
    
    if (output) {
        printf("%s", output);
        free(output);
    }

    free(input);
    papagaio_close(ctx);
    return 0;
}
