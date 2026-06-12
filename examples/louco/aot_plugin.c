#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/papagaio.h"
#include "../../src/louro/louro.h"

#ifdef ENV_HEADER
#include ENV_HEADER
#else
#error "ENV_HEADER must be defined!"
#endif

// Count is calculated automatically from the user's louro_exports array
int global_var_count = sizeof(louro_exports) / sizeof(louro_exports[0]);

static const char* get_var_c_name(const void* ptr) {
    for (int i = 0; i < global_var_count; i++) {
        if (louro_exports[i].address == ptr && TYPE_MASK(louro_exports[i].type) == LOURO_VARIABLE)
            return louro_exports[i].name;
    }
    return "unknown_var";
}

static const char* get_func_name(const void *fn) {
    for (int i = 0; i < global_var_count; i++) {
        if (louro_exports[i].address == fn)
            return (const char*)louro_exports[i].context;
    }
    return "unknown_func";
}

/* Returns the number of lazy thunks required by this subtree. */
static int get_thunk_count(const LouroExpression *n) {
    if (!n) return 0;
    int type = TYPE_MASK(n->type);
    if (type == LOURO_CONSTANT || type == LOURO_VARIABLE) return 0;
    int count = 0;
    int arity = ARITY(n->type);
    for (int i = 0; i < arity; i++) {
        count += get_thunk_count((LouroExpression*)n->parameters[i]);
    }
    if (IS_LAZY(n->type)) count += arity;
    return count;
}

static void louro_emit_c_stream(FILE *f, const LouroExpression *n, int start_id);

/*
 * Pass 1: Emit thunk definitions into stream.
 */
static void louro_emit_thunks_stream(FILE *f, const LouroExpression *n, int start_id) {
    if (!n) return;
    int type = TYPE_MASK(n->type);
    if (type == LOURO_CONSTANT || type == LOURO_VARIABLE) return;

    int arity = ARITY(n->type);
    int is_lazy = IS_LAZY(n->type);
    
    int current_id = start_id;
    for (int i = 0; i < arity; i++) {
        louro_emit_thunks_stream(f, (LouroExpression*)n->parameters[i], current_id);
        current_id += get_thunk_count((LouroExpression*)n->parameters[i]);
    }

    if (is_lazy) {
        int my_base = current_id;
        for (int i = 0; i < arity; i++) {
            fprintf(f, "static double __thunk_%d(void* _) { return ", my_base + i);
            /* Compute child's start_id for louro_emit_c */
            int child_start = start_id;
            for (int j = 0; j < i; j++) {
                child_start += get_thunk_count((LouroExpression*)n->parameters[j]);
            }
            louro_emit_c_stream(f, (LouroExpression*)n->parameters[i], child_start);
            fprintf(f, "; }\n");
        }
    }
}

/*
 * Pass 2: Emit the expression into stream.
 */
static void louro_emit_c_stream(FILE *f, const LouroExpression *n, int start_id) {
    if (!n) return;
    int type = TYPE_MASK(n->type);
    if (type == LOURO_CONSTANT) { fprintf(f, "%f", n->value); return; }
    if (type == LOURO_VARIABLE) { fprintf(f, "%s", get_var_c_name(n->bound)); return; }

    int arity = ARITY(n->type);
    int is_lazy = IS_LAZY(n->type);
    const char *c_func = get_func_name(n->function);

    if (is_lazy) {
        int my_base = start_id;
        for (int i = 0; i < arity; i++) {
            my_base += get_thunk_count((LouroExpression*)n->parameters[i]);
        }
        
        fprintf(f, "%s(", c_func);
        for (int i = 0; i < arity; i++) {
            fprintf(f, "(double)(uintptr_t)&(LouroLazy){ __thunk_%d, NULL }", my_base + i);
            if (i < arity - 1) fprintf(f, ", ");
        }
        fprintf(f, ")");
    } else {
        fprintf(f, "%s(", c_func);
        int current_id = start_id;
        for (int i = 0; i < arity; i++) {
            louro_emit_c_stream(f, (LouroExpression*)n->parameters[i], current_id);
            current_id += get_thunk_count((LouroExpression*)n->parameters[i]);
            if (i < arity - 1) fprintf(f, ", ");
        }
        fprintf(f, ")");
    }
}

/* Papagaio Command Handlers */

char *cmd_aot_thunks(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *userdata) {
    if (argc < 1) return strdup("");
    
    int err;
    LouroExpression *compiled = louro_compile(argv[0], louro_exports, global_var_count, &err);
    if (!compiled) return strdup(""); // You could also return an error string
    
    char *buf;
    size_t size;
    FILE *f = open_memstream(&buf, &size);
    if (!f) {
        louro_free(compiled);
        return strdup("");
    }
    
    louro_emit_thunks_stream(f, compiled, 0);
    fclose(f);
    louro_free(compiled);
    return buf;
}

char *cmd_aot_c(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *userdata) {
    if (argc < 1) return strdup("");
    
    int err;
    LouroExpression *compiled = louro_compile(argv[0], louro_exports, global_var_count, &err);
    if (!compiled) return strdup("");
    
    char *buf;
    size_t size;
    FILE *f = open_memstream(&buf, &size);
    if (!f) {
        louro_free(compiled);
        return strdup("");
    }
    
    louro_emit_c_stream(f, compiled, 0);
    fclose(f);
    louro_free(compiled);
    return buf;
}

// Plugin Entry Point
int papagaio_plugin_init(Papagaio *ctx) {
    papagaio_register_command(ctx, "aot_thunks", cmd_aot_thunks, NULL);
    papagaio_register_command(ctx, "aot_c", cmd_aot_c, NULL);
    return 1;
}
