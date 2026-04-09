#include "lib.c"

/* 
 * Hello Libc - Demonstration Plugin
 * Shows the use of standard library functions (printf, malloc, strrev, etc)
 * using the vanilla "papagaio_" naming convention.
 */

// Command: $reverse{text}
char* papagaio_reverse(int argc, char** argv) {
    if (argc < 1) return "";
    const char *input = argv[0];
    size_t len = strlen(input);
    
    printf("[WASM] Reversing: '%s' (len: %zu)\n", input, len);

    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;

    strcpy(result, input);
    strrev(result); 

    return result;
}

// Command: $calculate{a}{op}{b}
char* papagaio_calculate(int argc, char** argv) {
    if (argc < 3) return "Error: missing arguments";

    int a = atoi(argv[0]);
    const char *op = argv[1];
    int b = atoi(argv[2]);
    
    int res = 0;
    if (strcmp(op, "+") == 0) res = a + b;
    else if (strcmp(op, "-") == 0) res = a - b;
    else if (strcmp(op, "*") == 0) res = a * b;
    else if (strcmp(op, "/") == 0 && b != 0) res = a / b;

    char *out = (char*)malloc(64);
    snprintf(out, 64, "Result: %d %s %d = %d", a, op, b, res);
    return out;
}
