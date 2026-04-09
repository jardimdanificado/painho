#include "lib.c"

/* 
 * Modern Plugin - Using the new 'use' and quoted 'export' syntax
 */

// Style 1: Standalone declaration with quoted name
export echo as "repeat";

char *echo(int argc, const char **argv) {
    if (argc < 1) return "";
    return (char*)argv[0];
}

// Style 2: Prefix definition
export hello 
{
    return "Hello from the prefix style!";
}

// Style 3: Export after definition with quoted name
char *calculate_pi(int argc, const char **argv) {
    (void)argc; (void)argv;
    return "3.14159";
}
export calculate_pi as "pi";
