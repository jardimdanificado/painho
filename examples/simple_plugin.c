#include "lib.c"

/* 
 * Ultra-Simplified Plugin
 * Demonstrates the auto-registration of commands using the 
 * "papagaio_" prefix in vanilla C.
 */

// This will be registered as the command: $hello{...}
char* papagaio_hello(int argc, char** argv) {
    return "Hi from Wasm!";
}

// This will be registered as: $greet{...}
char* papagaio_greet(int argc, char** argv) {
    if (argc < 1) return "Hello!";
    
    char* buf = malloc(strlen(argv[0]) + 10);
    sprintf(buf, "Hello, %s", argv[0]);
    return buf;
}
