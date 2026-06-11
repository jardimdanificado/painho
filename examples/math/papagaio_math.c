#include "papagaio_math.h"
#include "../../src/louro/louro.h"
#include "../../src/louro/libs/louro_std.h"
#include "../../src/louro/libs/louro_math.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct {
    LouroVariable *math_funcs;
    int math_count;
    int math_cap;
} MathState;

static LouroVariable math_plugin_lookup(void *context, const char *name, int len) {
    MathState *state = (MathState*)context;
    for (int i = 0; i < state->math_count; ++i) {
        if (strlen(state->math_funcs[i].name) == (size_t)len && strncmp(state->math_funcs[i].name, name, len) == 0) {
            return state->math_funcs[i];
        }
    }
    LouroVariable null_var = {0};
    return null_var;
}

int papagaio_register_math_generic(Papagaio *ctx, const char *name, void *func, int arity, int is_closure, int is_pure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) {
        va_list args;
        va_start(args, is_pure);
        userdata = va_arg(args, void*);
        va_end(args);
    }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap ? state->math_cap << 1 : 8;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    if (is_closure) {
        v->type = LOURO_CLOSURE0 + arity;
        if (is_pure) v->type |= LOURO_FLAG_PURE;
        v->context = userdata;
    } else {
        v->type = LOURO_FUNCTION0 + arity;
        if (is_pure) v->type |= LOURO_FLAG_PURE;
    }
    return 0;
}

void papagaio_clear_math(Papagaio *ctx) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return;
    for (int i = 0; i < state->math_count; i++) {
        if (state->math_funcs[i].name) free((void*)state->math_funcs[i].name);
        if (state->math_funcs[i].separator2) free((void*)state->math_funcs[i].separator2);
        if (state->math_funcs[i].separator3) free((void*)state->math_funcs[i].separator3);
    }
    state->math_count = 0;
}

int papagaio_register_math_infix(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) { va_list args; va_start(args, is_closure); userdata = va_arg(args, void*); va_end(args); }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap ? state->math_cap << 1 : 8;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    v->type = LOURO_OPERATOR | LOURO_FLAG_INFIX | (is_closure ? LOURO_CLOSURE2 : (LOURO_FUNCTION2 | LOURO_FLAG_PURE)) | (precedence << 16);
    v->context = userdata;
    return 0;
}

int papagaio_register_math_prefix(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) { va_list args; va_start(args, is_closure); userdata = va_arg(args, void*); va_end(args); }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap ? state->math_cap << 1 : 8;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    v->type = LOURO_OPERATOR | LOURO_FLAG_PREFIX | (is_closure ? LOURO_CLOSURE1 : (LOURO_FUNCTION1 | LOURO_FLAG_PURE)) | (precedence << 16);
    v->context = userdata;
    return 0;
}

int papagaio_register_math_postfix(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) { va_list args; va_start(args, is_closure); userdata = va_arg(args, void*); va_end(args); }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap ? state->math_cap << 1 : 8;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    v->type = LOURO_OPERATOR | LOURO_FLAG_POSTFIX | (is_closure ? LOURO_CLOSURE1 : (LOURO_FUNCTION1 | LOURO_FLAG_PURE)) | (precedence << 16);
    v->context = userdata;
    return 0;
}

int papagaio_register_math_infix_right(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) { va_list args; va_start(args, is_closure); userdata = va_arg(args, void*); va_end(args); }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap << 1;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    v->type = LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FLAG_RIGHT_ASSOC | (is_closure ? LOURO_CLOSURE2 : (LOURO_FUNCTION2 | LOURO_FLAG_PURE)) | (precedence << 16);
    v->context = userdata;
    return 0;
}

int papagaio_register_math_ternary(Papagaio *ctx, const char *name, const char *sep2, void *func, int precedence, int is_closure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) { va_list args; va_start(args, is_closure); userdata = va_arg(args, void*); va_end(args); }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap ? state->math_cap << 1 : 8;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    v->type = LOURO_OPERATOR | LOURO_FLAG_TERNARY | LOURO_FLAG_INFIX | (is_closure ? LOURO_CLOSURE3 : (LOURO_FUNCTION3 | LOURO_FLAG_PURE)) | (precedence << 16);
    v->separator = v->name;
    v->separator2 = sep2 ? strdup(sep2) : NULL;
    v->context = userdata;
    return 0;
}

int papagaio_register_math_quaternary(Papagaio *ctx, const char *name, const char *sep2, const char *sep3, void *func, int precedence, int is_closure, ...) {
    MathState *state = (MathState*)papagaio_get_command_userdata(ctx, "math");
    if (!state) return -1;
    void *userdata = NULL;
    if (is_closure) { va_list args; va_start(args, is_closure); userdata = va_arg(args, void*); va_end(args); }
    if (!name || !func) return -1;
    if (state->math_count >= state->math_cap) {
        state->math_cap = state->math_cap ? state->math_cap << 1 : 8;
        state->math_funcs = (LouroVariable *)realloc(state->math_funcs, sizeof(LouroVariable) * state->math_cap);
    }
    LouroVariable *v = &state->math_funcs[state->math_count++];
    memset(v, 0, sizeof(LouroVariable));
    v->name = strdup(name);
    v->address = func;
    v->type = LOURO_OPERATOR | LOURO_FLAG_QUATERNARY | LOURO_FLAG_PREFIX | (is_closure ? LOURO_CLOSURE3 : (LOURO_FUNCTION3 | LOURO_FLAG_PURE)) | (precedence << 16);
    v->separator = v->name;
    v->separator2 = sep2 ? strdup(sep2) : NULL;
    v->separator3 = sep3 ? strdup(sep3) : NULL;
    v->context = userdata;
    return 0;
}

static char *math_cmd_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata) {
    if (argc != 1) return NULL;
    MathState *state = (MathState*)userdata;
    
    char *arg = papagaio_process_text(ctx, argv[0], argl[0]);
    if (!arg) return NULL;
    
    int err = 0;
    LouroVariable scope[] = { LOURO_STD, LOURO_MATH };
    int count = sizeof(scope) / sizeof(scope[0]);
    LouroExpression *expr = louro_compile_ex(arg, scope, count, math_plugin_lookup, state, &err);
    free(arg);
    
    if (expr) {
        double result = louro_evaluate(expr);
        louro_free(expr);
        char res_buf[64];
        if (result == (long long)result) {
            snprintf(res_buf, sizeof(res_buf), "%lld", (long long)result);
        } else {
            snprintf(res_buf, sizeof(res_buf), "%g", result);
        }
        return strdup(res_buf);
    }
    return strdup("");
}

static void math_plugin_finalizer(void *userdata) {
    MathState *state = (MathState*)userdata;
    if (state) {
        for (int i = 0; i < state->math_count; i++) {
            if (state->math_funcs[i].name) free((void*)state->math_funcs[i].name);
            if (state->math_funcs[i].separator2) free((void*)state->math_funcs[i].separator2);
            if (state->math_funcs[i].separator3) free((void*)state->math_funcs[i].separator3);
        }
        if (state->math_funcs) free(state->math_funcs);
        free(state);
    }
}

__attribute__((visibility("default")))
int papagaio_plugin_init(Papagaio *ctx) {
    if (papagaio_has_command(ctx, "math")) return 0; // Already loaded
    
    MathState *state = (MathState*)malloc(sizeof(MathState));
    if (!state) return -1;
    
    state->math_funcs = NULL;
    state->math_count = 0;
    state->math_cap = 0;
    
    papagaio_register_command(ctx, "math", math_cmd_handler, state);
    papagaio_add_finalizer(ctx, math_plugin_finalizer, state);
    
    return 0;
}
