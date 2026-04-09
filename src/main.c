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
    if (argc < 2) {
        fprintf(stderr, "Usage: papagaio <file.txt>\n");
        return 1;
    }

    Papagaio *ctx = papagaio_open();
    papagaio_set_args(ctx, argc, argv);

    size_t len = 0;
    char *input = read_file(argv[1], &len);
    if (!input) {
        fprintf(stderr, "Error reading file: %s\n", argv[1]);
        papagaio_close(ctx);
        return 1;
    }

    char *output = papagaio_process_text(ctx, input, len);
    
    if (output) {
        printf("%s", output);
        free(output);
    } else {
        fprintf(stderr, "Error during papagaio processing.\n");
    }

    free(input);
    papagaio_close(ctx);
    return 0;
}
