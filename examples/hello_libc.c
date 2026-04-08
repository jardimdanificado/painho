#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Bridge helper: get argument as string from the offset table */
static const char *get_arg(int argc, uint32_t argv_ptr, int i) {
    if (i >= argc) return NULL;
    uint32_t *table = (uint32_t *)argv_ptr;
    return (const char *)table[i];
}

/* 
 * command: $reverse{text}
 * Demonstrates: printf (via __host_write), malloc, strlen, strrev.
 */
__attribute__((export_name("pap_cmd_reverse")))
char* pap_reverse(int argc, uint32_t argv_ptr) {
    const char *input = get_arg(argc, argv_ptr, 0);
    if (!input) return NULL;

    size_t len = strlen(input);
    
    /* Print message to host terminal (stdout) */
    printf("[WASM] Reversing: '%s' (len: %zu)\n", input, len);

    /* Allocate memory for result */
    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;

    strcpy(result, input);
    strrev(result); /* built-in wasm-libc extension */

    return result;
}

/* 
 * command: $calc{a}{op}{b}
 * Demonstrates: snprintf, atoi.
 */
__attribute__((export_name("pap_cmd_calc")))
char* pap_calc(int argc, uint32_t argv_ptr) {
    if (argc < 3) return NULL;

    int a = atoi(get_arg(argc, argv_ptr, 0));
    const char *op = get_arg(argc, argv_ptr, 1);
    int b = atoi(get_arg(argc, argv_ptr, 2));
    
    int res = 0;
    if (strcmp(op, "+") == 0) res = a + b;
    else if (strcmp(op, "-") == 0) res = a - b;
    else if (strcmp(op, "*") == 0) res = a * b;
    else if (strcmp(op, "/") == 0 && b != 0) res = a / b;

    char *out = (char*)malloc(64);
    snprintf(out, 64, "Result: %d %s %d = %d", a, op, b, res);
    return out;
}
