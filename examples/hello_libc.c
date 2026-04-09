use "plibc";

/* 
 * Hello Libc - Plugin Demonstrativo
 * Mostra o uso de funções da biblioteca padrão (printf, malloc, strrev, etc).
 */

// Comando: $reverse{texto}
export reverse 
{
    if (argc < 1) return "";
    const char *input = argv[0];
    size_t len = strlen(input);
    
    printf("[WASM] Reversing: '%s' (len: %zu)\n", input, len);

    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;

    strcpy(result, input);
    strrev(result); // Extensão nativa da wasm-libc do Papagaio

    return result;
}

// Comando: $calc{a}{op}{b}
export calc as "calcular"
{
    if (argc < 3) return "Erro: faltam argumentos";

    int a = atoi(argv[0]);
    const char *op = argv[1];
    int b = atoi(argv[2]);
    
    int res = 0;
    if (strcmp(op, "+") == 0) res = a + b;
    else if (strcmp(op, "-") == 0) res = a - b;
    else if (strcmp(op, "*") == 0) res = a * b;
    else if (strcmp(op, "/") == 0 && b != 0) res = a / b;

    char *out = (char*)malloc(64);
    snprintf(out, 64, "Resultado: %d %s %d = %d", a, op, b, res);
    return out;
}
