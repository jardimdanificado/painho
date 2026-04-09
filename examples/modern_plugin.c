#include "lib.c"

/* 
 * Plugin Example - Naming Convention
 * 
 * Commands exported with the 'papagaio_' prefix are automatically 
 * registered as Papagaio commands.
 */

// Registered as: $shout{text}
char* papagaio_shout(int argc, char **argv) {
    if (argc < 1) return "";
    
    char *input = argv[0];
    size_t len = strlen(input);
    char *res = malloc(len + 4);
    
    strcpy(res, input);
    for (size_t i = 0; i < len; i++) {
        if (res[i] >= 'a' && res[i] <= 'z') res[i] -= 32;
    }
    strcat(res, "!!!");
    
    return res;
}
