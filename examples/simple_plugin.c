use "plibc";

/* 
 * Plugin Ultra-Simplificado
 * Demonstra a sintaxe moderna de exportação.
 */

// Comando: $echo{texto}
export echo 
{
    if (argc < 1) return "";
    
    printf("[WASM] Echoing back: %s\n", argv[0]);
    
    char *res = (char*)malloc(strlen(argv[0]) + 16);
    if (!res) return NULL;
    sprintf(res, "Echo: %s", argv[0]);
    return res;
}

// Comando: $shout{texto}
export shout as "GRITAR"
{
    if (argc < 1) return "";
    
    char *res = strdup(argv[0]);
    if (!res) return NULL;
    for (int i = 0; res[i]; i++) {
        if (res[i] >= 'a' && res[i] <= 'z') res[i] -= 32;
    }
    return res;
}
