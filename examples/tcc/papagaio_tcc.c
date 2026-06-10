#include "../../src/papagaio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtcc.h>

typedef struct {
    TCCState **states;
    int count;
    int cap;
} TCCPool;

static TCCPool pool = {0};

static char *tcc_eval_command(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata) {
    (void)name;
    (void)userdata;
    if (argc < 1) return strdup("");
    
    TCCState *s = tcc_new();
    if (!s) return strdup("TCC_ERROR: failed to create context");
    
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    
    const char *header = 
        "#include <string.h>\n"
        "#include <stdlib.h>\n"
        "#include <stdio.h>\n"
        "typedef struct Papagaio Papagaio;\n"
        "void papagaio_register_command(Papagaio *ctx, const char *name, void *func, void *userdata);\n"
        "void papagaio_register_math_generic(Papagaio *ctx, const char *name, void *func, int arity, int is_closure, int is_pure, ...);\n";
        
    char *code = malloc(strlen(header) + argl[0] + 10);
    strcpy(code, header);
    strncat(code, argv[0], argl[0]);
    
    if (tcc_compile_string(s, code) == -1) {
        free(code);
        tcc_delete(s);
        return strdup("TCC_ERROR: compilation failed");
    }
    free(code);
    
    tcc_add_symbol(s, "papagaio_register_command", papagaio_register_command);
    tcc_add_symbol(s, "papagaio_register_math_generic", papagaio_register_math_generic);
    
    if (tcc_relocate(s) < 0) {
        tcc_delete(s);
        return strdup("TCC_ERROR: relocation failed");
    }
    
    int (*init_func)(Papagaio*) = tcc_get_symbol(s, "init");
    if (init_func) {
        init_func(ctx);
    }
    
    if (pool.count >= pool.cap) {
        pool.cap = pool.cap ? pool.cap * 2 : 4;
        pool.states = realloc(pool.states, pool.cap * sizeof(TCCState*));
    }
    pool.states[pool.count++] = s;
    
    return strdup("");
}

// Optional cleanup when papagaio context closes
static void tcc_cleanup(void *userdata) {
    (void)userdata;
    for (int i = 0; i < pool.count; i++) {
        tcc_delete(pool.states[i]);
    }
    if (pool.states) {
        free(pool.states);
        pool.states = NULL;
    }
    pool.count = 0;
    pool.cap = 0;
}

int papagaio_plugin_init(Papagaio *ctx) {
    papagaio_register_command(ctx, "c", tcc_eval_command, NULL);
    papagaio_add_finalizer(ctx, tcc_cleanup, NULL);
    return 0;
}
