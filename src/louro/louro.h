// SPDX-License-Identifier: Zlib
/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2020 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * NOTICE: This is a *HEAVILY* modified version of the original TinyExpr library.
 */

#ifndef LOURO_H
#define LOURO_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#ifndef NAN
#define NAN (0.0/0.0)
#endif

#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LouroExpression {
    int type;
    union {double value; const double *bound; const void *function;};
    void *parameters[1];
} LouroExpression;


enum {
    LOURO_VARIABLE = 0,

    LOURO_FUNCTION0 = 32, LOURO_FUNCTION1, LOURO_FUNCTION2, LOURO_FUNCTION3,
    LOURO_FUNCTION4, LOURO_FUNCTION5, LOURO_FUNCTION6, LOURO_FUNCTION7,
    LOURO_FUNCTION8, LOURO_FUNCTION9, LOURO_FUNCTION10, LOURO_FUNCTION11,
    LOURO_FUNCTION12, LOURO_FUNCTION13, LOURO_FUNCTION14, LOURO_FUNCTION15,

    LOURO_CLOSURE0 = 64, LOURO_CLOSURE1, LOURO_CLOSURE2, LOURO_CLOSURE3,
    LOURO_CLOSURE4, LOURO_CLOSURE5, LOURO_CLOSURE6, LOURO_CLOSURE7,
    LOURO_CLOSURE8, LOURO_CLOSURE9, LOURO_CLOSURE10, LOURO_CLOSURE11,
    LOURO_CLOSURE12, LOURO_CLOSURE13, LOURO_CLOSURE14, LOURO_CLOSURE15,

    LOURO_FLAG_PURE = 128,
    LOURO_OPERATOR = 256,
    LOURO_FLAG_RIGHT_ASSOC = 512,
    
    LOURO_FLAG_INFIX = 1024,
    LOURO_FLAG_PREFIX = 2048,
    LOURO_FLAG_POSTFIX = 4096,
    LOURO_FLAG_TERNARY = 8192,
    LOURO_FLAG_LAZY = 16384,
    LOURO_FLAG_QUATERNARY = 32768
};

/* Lazy evaluation: a thunk wrapping an unevaluated expression */
typedef double (*LouroThunk)(void*);
typedef struct { LouroThunk thunk; void *data; } LouroLazy;
static inline double louro_lazy_eval(double l_val) { LouroLazy *l = (LouroLazy*)(uintptr_t)l_val; return l->thunk(l->data); }
#define IS_LAZY(TYPE) (((TYPE) & LOURO_FLAG_LAZY) != 0)

/* Builtin function descriptor (used internally; no variable binding). */
typedef struct LouroVariable {
    const char *name;
    const void *address;
    int type;
    void *context;
    const char *separator;
    const char *separator2;
    const char *separator3;
} LouroVariable;
#define LOURO_VAR(name, ptr) {name, (const void*)(ptr), LOURO_VARIABLE, 0, NULL}

#define LOURO_PURE(name, func, arity)         {name, (const void*)(func), (LOURO_FUNCTION0 + (arity)) | LOURO_FLAG_PURE, (void*)#func, NULL}
#define LOURO_IMPURE(name, func, arity)        {name, (const void*)(func), (LOURO_FUNCTION0 + (arity)), (void*)#func, NULL}
#define LOURO_PURE_LAZY(name, func, arity)     {name, (const void*)(func), (LOURO_FUNCTION0 + (arity)) | LOURO_FLAG_PURE | LOURO_FLAG_LAZY, (void*)#func, NULL}
#define LOURO_IMPURE_LAZY(name, func, arity)   {name, (const void*)(func), (LOURO_FUNCTION0 + (arity)) | LOURO_FLAG_LAZY, (void*)#func, NULL}

#define LOURO_OP(name, func, prec)             {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, NULL}
#define LOURO_OP_RIGHT(name, func, prec)       {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FLAG_RIGHT_ASSOC | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, NULL}
#define LOURO_OP_PREFIX(name, func, prec)      {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_PREFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, NULL}
#define LOURO_OP_POSTFIX(name, func, prec)     {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_POSTFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, NULL}
#define LOURO_OP_LAZY(name, func, prec)        {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | LOURO_FLAG_LAZY | ((prec) << 16), (void*)#func, NULL}
#define LOURO_OP_PREFIX_LAZY(name, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_PREFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | LOURO_FLAG_LAZY | ((prec) << 16), (void*)#func, NULL}

#define LOURO_TERNARY(name, sep, func, prec)        {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_TERNARY | LOURO_FUNCTION3 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, sep}
#define LOURO_TERNARY_LAZY(name, sep, func, prec)   {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_TERNARY | LOURO_FUNCTION3 | LOURO_FLAG_PURE | LOURO_FLAG_LAZY | ((prec) << 16), (void*)#func, sep}
#define LOURO_TERNARY_PREFIX(name, sep, func, prec)      {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_TERNARY | LOURO_FLAG_PREFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, sep}
#define LOURO_TERNARY_PREFIX_LAZY(name, sep, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_TERNARY | LOURO_FLAG_PREFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | LOURO_FLAG_LAZY | ((prec) << 16), (void*)#func, sep}
#define LOURO_QUATERNARY_PREFIX(name, sep, sep2, sep3, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_QUATERNARY | LOURO_FLAG_PREFIX | LOURO_FUNCTION3 | LOURO_FLAG_PURE | ((prec) << 16), (void*)#func, sep, sep2, sep3}
#define LOURO_QUATERNARY_PREFIX_LAZY(name, sep, sep2, sep3, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_QUATERNARY | LOURO_FLAG_PREFIX | LOURO_FUNCTION3 | LOURO_FLAG_PURE | LOURO_FLAG_LAZY | ((prec) << 16), (void*)#func, sep, sep2, sep3}

/* Dynamic capacity context support */
typedef LouroVariable (*LouroLookupCallback)(void *context, const char *name, int len);

/* Parses the input expression with dynamic lookup support. */
/* Returns NULL on error. */
static inline LouroExpression *louro_compile_ex(const char *expression, const LouroVariable *variables, int var_count, LouroLookupCallback lookup_cb, void *lookup_ctx, int *error);

/* Parses the input expression. */
/* Returns NULL on error. */
static inline LouroExpression *louro_compile(const char *expression, const LouroVariable *variables, int var_count, int *error) {
    return louro_compile_ex(expression, variables, var_count, NULL, NULL, error);
}

/* Evaluates the expression. */
static inline double louro_evaluate(const LouroExpression *n);

/* Lazy thunk wrapper — calls louro_evaluate on a stored node. */
static double louro_thunk_wrapper(void *node) { return louro_evaluate((const LouroExpression*)node); }

/* Frees the expression. */
/* This is safe to call on NULL pointers. */
static inline void louro_free(LouroExpression *n);


/* 
 * ============================================================================
 * IMPLEMENTATION
 * ============================================================================
 */

typedef double (*lr_fun2)(double, double);

enum {
    TOK_NULL = LOURO_CLOSURE15+1, TOK_ERROR, TOK_END, TOK_SEP,
    TOK_OPEN, TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_OPERATOR,
    TOK_TERNARY_SEP
};


enum {LOURO_CONSTANT = 1};


typedef struct state {
    const char *start;
    const char *next;
    int type;
    union {double value; const double *bound; const void *function;};
    void *context;

    const LouroVariable *lookup;
    int lookup_len;
    
    LouroLookupCallback lookup_cb;
    void *lookup_ctx;
    
    int expecting_operator;
    int op_precedence;
    int op_flags;
    const char *op_separator;
    const char *op_separator2;
    const char *op_separator3;
} state;


#define TYPE_MASK(TYPE) ((TYPE)&0x0000007F)

#define IS_PURE(TYPE) (((TYPE) & LOURO_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE) & LOURO_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE) (((TYPE) & LOURO_CLOSURE0) != 0)
#define ARITY(TYPE) ( ((TYPE) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((TYPE) & 0x0000001F) : 0 )
#define NEW_EXPR(type, ...) new_expr((type), (const LouroExpression*[]){__VA_ARGS__})
#define CHECK_NULL(ptr, ...) if ((ptr) == NULL) { __VA_ARGS__;  { printf("NULL at %d\n", __LINE__); return NULL; }; }

static inline LouroExpression *new_expr(const int type, const LouroExpression *parameters[]) {
    const int arity = ARITY(type);
    const int psize = sizeof(void*) * arity;
    const int size = (sizeof(LouroExpression) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
    LouroExpression *ret = (LouroExpression*)malloc(size);
    CHECK_NULL(ret);

    memset(ret, 0, size);
    if (arity && parameters) {
        memcpy(ret->parameters, parameters, psize);
    }
    ret->type = type;
    ret->bound = 0;
    return ret;
}


static inline void louro_free_parameters(LouroExpression *n) {
    if (!n) return;
    switch (TYPE_MASK(n->type)) {
        case LOURO_FUNCTION15: case LOURO_CLOSURE15: louro_free((LouroExpression*)n->parameters[14]);     /* Falls through. */
        case LOURO_FUNCTION14: case LOURO_CLOSURE14: louro_free((LouroExpression*)n->parameters[13]);     /* Falls through. */
        case LOURO_FUNCTION13: case LOURO_CLOSURE13: louro_free((LouroExpression*)n->parameters[12]);     /* Falls through. */
        case LOURO_FUNCTION12: case LOURO_CLOSURE12: louro_free((LouroExpression*)n->parameters[11]);     /* Falls through. */
        case LOURO_FUNCTION11: case LOURO_CLOSURE11: louro_free((LouroExpression*)n->parameters[10]);     /* Falls through. */
        case LOURO_FUNCTION10: case LOURO_CLOSURE10: louro_free((LouroExpression*)n->parameters[9]);     /* Falls through. */
        case LOURO_FUNCTION9: case LOURO_CLOSURE9: louro_free((LouroExpression*)n->parameters[8]);     /* Falls through. */
        case LOURO_FUNCTION8: case LOURO_CLOSURE8: louro_free((LouroExpression*)n->parameters[7]);     /* Falls through. */
        case LOURO_FUNCTION7: case LOURO_CLOSURE7: louro_free((LouroExpression*)n->parameters[6]);     /* Falls through. */
        case LOURO_FUNCTION6: case LOURO_CLOSURE6: louro_free((LouroExpression*)n->parameters[5]);     /* Falls through. */
        case LOURO_FUNCTION5: case LOURO_CLOSURE5: louro_free((LouroExpression*)n->parameters[4]);     /* Falls through. */
        case LOURO_FUNCTION4: case LOURO_CLOSURE4: louro_free((LouroExpression*)n->parameters[3]);     /* Falls through. */
        case LOURO_FUNCTION3: case LOURO_CLOSURE3: louro_free((LouroExpression*)n->parameters[2]);     /* Falls through. */
        case LOURO_FUNCTION2: case LOURO_CLOSURE2: louro_free((LouroExpression*)n->parameters[1]);     /* Falls through. */
        case LOURO_FUNCTION1: case LOURO_CLOSURE1: louro_free((LouroExpression*)n->parameters[0]);
    }
}


static inline void louro_free(LouroExpression *n) {
    if (!n) return;
    louro_free_parameters(n);
    free(n);
}


/* Built-in functions and their declarations for TinyExpr */

static inline const LouroVariable *find_lookup(const state *s, const char *name, int len) {
    int iters;
    const LouroVariable *var;
    if (!s->lookup) return 0;

    for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) {
        if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0') {
            return var;
        }
    }
    return 0;
}

static inline void next_token(state *s) {
    s->type = TOK_NULL;

    do {
        if (!*s->next){
            s->type = TOK_END;
            return;
        }

        /* Try reading a number. */
        if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') {
            s->value = strtod(s->next, (char**)&s->next);
            s->type = TOK_NUMBER;
            return;
        }

        /* 1. First, check if there's a matching operator or variable in the lookup table.
           We match the longest possible prefix to support custom operators like '==' or '=>'.
        */
        const LouroVariable *best_match = NULL;
        int best_match_len = 0;
        
        if (s->lookup) {
            for (int i = 0; i < s->lookup_len; ++i) {
                const LouroVariable *var = &s->lookup[i];
                int len = strlen(var->name);
                if (strncmp(s->next, var->name, len) == 0) {
                    // Make sure it's a full word match if it's alphanumeric
                    if (isalpha(var->name[0])) {
                        if (isalnum(s->next[len]) || s->next[len] == '_') continue;
                    }
                    
                    if (var->type & LOURO_OPERATOR) {
                        if (s->expecting_operator) {
                            if (!(var->type & (LOURO_FLAG_INFIX | LOURO_FLAG_POSTFIX | LOURO_FLAG_TERNARY))) continue;
                        } else {
                            if (!(var->type & LOURO_FLAG_PREFIX)) continue;
                        }
                    }

                    if (len > best_match_len) {
                        best_match = var;
                        best_match_len = len;
                    }
                }
            }
        }

        if (best_match) {
            s->next += best_match_len;
            if (best_match->type & LOURO_OPERATOR) {
                s->type = TOK_OPERATOR;
                s->function = best_match->address;
                s->op_precedence = (best_match->type >> 16);
                s->op_flags = best_match->type & (LOURO_FLAG_RIGHT_ASSOC | LOURO_FLAG_INFIX | LOURO_FLAG_PREFIX | LOURO_FLAG_POSTFIX | LOURO_FLAG_TERNARY | LOURO_FLAG_LAZY | LOURO_FLAG_QUATERNARY);
                s->op_separator = best_match->separator;
                s->op_separator2 = best_match->separator2;
                s->op_separator3 = best_match->separator3;
                return;
            } else {
                switch(TYPE_MASK(best_match->type)) {
                    case LOURO_VARIABLE:
                        s->type = TOK_VARIABLE;
                        s->bound = (const double*)best_match->address;
                        return;
                    case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3: case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7: case LOURO_CLOSURE8: case LOURO_CLOSURE9: case LOURO_CLOSURE10: case LOURO_CLOSURE11: case LOURO_CLOSURE12: case LOURO_CLOSURE13: case LOURO_CLOSURE14: case LOURO_CLOSURE15:
                        s->context = best_match->context;
                    case LOURO_FUNCTION0: case LOURO_FUNCTION1: case LOURO_FUNCTION2: case LOURO_FUNCTION3: case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7: case LOURO_FUNCTION8: case LOURO_FUNCTION9: case LOURO_FUNCTION10: case LOURO_FUNCTION11: case LOURO_FUNCTION12: case LOURO_FUNCTION13: case LOURO_FUNCTION14: case LOURO_FUNCTION15:
                        s->type = best_match->type;
                        s->function = best_match->address;
                        return;
                }
            }
        }

        /* 1.5. Match custom ternary separators */
        if (!best_match && s->lookup) {
            for (int i = 0; i < s->lookup_len; ++i) {
                const LouroVariable *var = &s->lookup[i];
                if (var->separator) {
                    int len = strlen(var->separator);
                    if (strncmp(s->next, var->separator, len) == 0) {
                        if (isalpha(var->separator[0]) && (isalnum(s->next[len]) || s->next[len] == '_')) continue;
                        s->next += len;
                        s->type = TOK_TERNARY_SEP;
                        s->op_separator = var->separator; // Pass which separator was found
                        return;
                    }
                }
                if (var->separator2) {
                    int len = strlen(var->separator2);
                    if (strncmp(s->next, var->separator2, len) == 0) {
                        if (isalpha(var->separator2[0]) && (isalnum(s->next[len]) || s->next[len] == '_')) continue;
                        s->next += len;
                        s->type = TOK_TERNARY_SEP;
                        s->op_separator = var->separator2; // Pass which separator was found
                        return;
                    }
                }
                if (var->separator3) {
                    int len = strlen(var->separator3);
                    if (strncmp(s->next, var->separator3, len) == 0) {
                        if (isalpha(var->separator3[0]) && (isalnum(s->next[len]) || s->next[len] == '_')) continue;
                        s->next += len;
                        s->type = TOK_TERNARY_SEP;
                        s->op_separator = var->separator3; // Pass which separator was found
                        return;
                    }
                }
            }
        }

        /* 2. Fallback to builtin structural characters if no custom operator matched. */
        int matched = 1;
        switch (s->next[0]) {
            case '(': s->type = TOK_OPEN;  s->next++; break;
            case ')': s->type = TOK_CLOSE; s->next++; break;
            case ',': s->type = TOK_SEP;   s->function = 0; s->op_precedence = 10; s->op_flags = 0; s->next++; break;
            case ' ': case '\t': case '\n': case '\r': s->next++; matched = 0; break;
            default: 
                // If it's an unrecognized alphanumeric, it's an error (e.g. undeclared variable)
                // OR we can dynamically look it up if lookup_cb is provided.
                if (isalpha(s->next[0])) {
                    const char *token_start = s->next;
                    while (isalpha(s->next[0]) || isdigit(s->next[0]) || (s->next[0] == '_')) s->next++;
                    int len = (int)(s->next - token_start);
                    
                    if (s->lookup_cb) {
                        LouroVariable var = s->lookup_cb(s->lookup_ctx, token_start, len);
                        if (var.name != NULL) { // Using name != NULL as a valid flag
                            if (var.type & LOURO_OPERATOR) {
                                s->type = TOK_OPERATOR;
                                s->function = var.address;
                                s->op_precedence = (var.type >> 16);
                                s->op_flags = var.type & (LOURO_FLAG_RIGHT_ASSOC | LOURO_FLAG_INFIX | LOURO_FLAG_PREFIX | LOURO_FLAG_POSTFIX | LOURO_FLAG_TERNARY | LOURO_FLAG_LAZY | LOURO_FLAG_QUATERNARY);
                                s->op_separator = var.separator;
                                s->op_separator2 = var.separator2;
                                s->op_separator3 = var.separator3;
                            } else {
                                switch(TYPE_MASK(var.type)) {
                                    case LOURO_VARIABLE:
                                        s->type = TOK_VARIABLE;
                                        s->bound = (const double*)var.address;
                                        break;
                                    case LOURO_FUNCTION0: case LOURO_FUNCTION1: case LOURO_FUNCTION2: case LOURO_FUNCTION3: case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7: case LOURO_FUNCTION8: case LOURO_FUNCTION9: case LOURO_FUNCTION10: case LOURO_FUNCTION11: case LOURO_FUNCTION12: case LOURO_FUNCTION13: case LOURO_FUNCTION14: case LOURO_FUNCTION15:
                                    case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3: case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7: case LOURO_CLOSURE8: case LOURO_CLOSURE9: case LOURO_CLOSURE10: case LOURO_CLOSURE11: case LOURO_CLOSURE12: case LOURO_CLOSURE13: case LOURO_CLOSURE14: case LOURO_CLOSURE15:
                                        s->type = var.type;
                                        s->function = var.address;
                                        break;
                                    default:
                                        s->type = TOK_ERROR;
                                        break;
                                }
                            }
                            if (s->type != TOK_ERROR) return;
                        }
                    }
                } else {
                    s->next++; 
                }
                s->type = TOK_ERROR; 
                break;
        }
        if (matched && s->type != TOK_NULL) return;
        
    } while (s->type == TOK_NULL);
}

static inline LouroExpression *parse_expr_dynamic(state *s, int precedence);

static inline LouroExpression *base(state *s) {
    LouroExpression *ret;
    int arity;

    switch (TYPE_MASK(s->type)) {
        case TOK_NUMBER:
            ret = new_expr(LOURO_CONSTANT, 0);
            if(!ret) { s->type = TOK_ERROR;  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->value = s->value;
            s->expecting_operator = 1;
            next_token(s);
            break;

        case TOK_VARIABLE:
            ret = new_expr(LOURO_VARIABLE, 0);
            if(!ret) { s->type = TOK_ERROR;  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->bound = s->bound;
            s->expecting_operator = 1;
            next_token(s);
            break;

        case LOURO_FUNCTION0:
        case LOURO_CLOSURE0:
            ret = new_expr(s->type, 0);
            if(!ret) { s->type = TOK_ERROR;  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[0] = s->context;
            
            s->expecting_operator = 1;
            next_token(s);
            if (s->type == TOK_OPEN) {
                s->expecting_operator = 0;
                next_token(s);
                if (s->type != TOK_CLOSE) {
                    s->type = TOK_ERROR;
                } else {
                    s->expecting_operator = 1;
                    next_token(s);
                }
            }
            break;

        case LOURO_FUNCTION1:
        case LOURO_CLOSURE1:
            ret = new_expr(s->type, 0);
            if(!ret) { s->type = TOK_ERROR;  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[1] = s->context;
            
            s->expecting_operator = 0;
            next_token(s);
            
            ret->parameters[0] = parse_expr_dynamic(s, 60);
            if(!ret->parameters[0]) { louro_free(ret);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            break;

        case LOURO_FUNCTION2: case LOURO_FUNCTION3: case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7: case LOURO_FUNCTION8: case LOURO_FUNCTION9: case LOURO_FUNCTION10: case LOURO_FUNCTION11: case LOURO_FUNCTION12: case LOURO_FUNCTION13: case LOURO_FUNCTION14: case LOURO_FUNCTION15:
        case LOURO_CLOSURE2: case LOURO_CLOSURE3: case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7: case LOURO_CLOSURE8: case LOURO_CLOSURE9: case LOURO_CLOSURE10: case LOURO_CLOSURE11: case LOURO_CLOSURE12: case LOURO_CLOSURE13: case LOURO_CLOSURE14: case LOURO_CLOSURE15:
            arity = ARITY(s->type);
            ret = new_expr(s->type, 0);
            if(!ret) { s->type = TOK_ERROR;  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[arity] = s->context;
            
            s->expecting_operator = 0; // The next token must be '(' (not an operator, structural, fetched in primary mode)
            next_token(s);

            if (s->type != TOK_OPEN) {
                s->type = TOK_ERROR;
            } else {
                int i;
                for(i = 0; i < arity; i++) {
                    s->expecting_operator = 0;
                    next_token(s);
                    ret->parameters[i] = parse_expr_dynamic(s, 0); 
                    if(!ret->parameters[i]) { louro_free(ret);  { printf("NULL at %d\n", __LINE__); return NULL; }; }

                    if(s->type != TOK_SEP) {
                        break;
                    }
                }
                if(s->type != TOK_CLOSE || i != arity - 1) {
                    s->type = TOK_ERROR;
                } else {
                    s->expecting_operator = 1;
                    next_token(s);
                }
            }
            break;

        case TOK_OPEN:
            s->expecting_operator = 0;
            next_token(s);
            ret = parse_expr_dynamic(s, 0);
            if(!ret)  { printf("NULL at %d\n", __LINE__); return NULL; };

            if (s->type != TOK_CLOSE) {
                s->type = TOK_ERROR;
            } else {
                s->expecting_operator = 1;
                next_token(s);
            }
            break;

        default:
            ret = new_expr(0, 0);
            if(!ret) { s->type = TOK_ERROR;  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            s->type = TOK_ERROR;
            ret->value = NAN; // using NAN requires math.h but louro evaluate returns NAN anyway. We'll use 0.0/0.0 if NAN isn't available? Wait, louro.h doesn't include math.h at the top, but it uses fmod etc. It's fine.
            break;
    }

    return ret;
}

static inline LouroExpression *parse_prefix(state *s) {
    if (s->type == TOK_OPERATOR && (s->op_flags & LOURO_FLAG_PREFIX)) {
        const void *func = s->function;
        int prec = s->op_precedence;
        const char *sep = s->op_separator;
        const char *sep2 = s->op_separator2;
        const char *sep3 = s->op_separator3;
        int is_ternary = (s->op_flags & LOURO_FLAG_TERNARY);
        int is_quaternary = (s->op_flags & LOURO_FLAG_QUATERNARY);
        int lazy_flag  = (s->op_flags & LOURO_FLAG_LAZY);
        
        s->expecting_operator = 0;
        next_token(s);
        
        LouroExpression *operand = parse_expr_dynamic(s, prec);
        if (!operand)  { printf("NULL at %d\n", __LINE__); return NULL; };
        
        if (is_quaternary) {
            if (s->type != TOK_TERNARY_SEP || s->op_separator != sep) {
                louro_free(operand); { printf("NULL at %d\n", __LINE__); return NULL; };
            }
            s->expecting_operator = 0;
            next_token(s);
            
            LouroExpression *operand2 = parse_expr_dynamic(s, 0);
            if (!operand2) { louro_free(operand); { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            if (s->type != TOK_TERNARY_SEP || s->op_separator != sep2) {
                louro_free(operand); louro_free(operand2); { printf("NULL at %d\n", __LINE__); return NULL; };
            }
            s->expecting_operator = 0;
            next_token(s);
            
            LouroExpression *operand3 = parse_expr_dynamic(s, prec);
            if (!operand3) { louro_free(operand); louro_free(operand2); { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            if (s->type != TOK_TERNARY_SEP || s->op_separator != sep3) {
                louro_free(operand); louro_free(operand2); louro_free(operand3); { printf("NULL at %d\n", __LINE__); return NULL; };
            }
            s->expecting_operator = 1;
            next_token(s);
            
            LouroExpression *ret = new_expr(LOURO_FUNCTION3 | LOURO_FLAG_PURE | lazy_flag, 0);
            if (!ret) { louro_free(operand); louro_free(operand2); louro_free(operand3); { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->function = func;
            ret->parameters[0] = operand;
            ret->parameters[1] = operand2;
            ret->parameters[2] = operand3;
            return ret;
        }

        if (is_ternary) {
            /* Expect the separator (e.g. "else") */
            if (s->type != TOK_TERNARY_SEP || s->op_separator != sep) {
                louro_free(operand); { printf("NULL at %d\n", __LINE__); return NULL; };
            }
            
            s->expecting_operator = 0;
            next_token(s);
            
            LouroExpression *operand2 = parse_expr_dynamic(s, prec);
            if (!operand2) { louro_free(operand); { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            LouroExpression *ret = new_expr(LOURO_FUNCTION2 | LOURO_FLAG_PURE | lazy_flag, 0);
            if (!ret) { louro_free(operand); louro_free(operand2); { printf("NULL at %d\n", __LINE__); return NULL; }; }
            ret->function = func;
            ret->parameters[0] = operand;
            ret->parameters[1] = operand2;
            return ret;
        }
        
        LouroExpression *ret = new_expr(LOURO_FUNCTION1 | LOURO_FLAG_PURE | lazy_flag, 0);
        if (!ret) { louro_free(operand);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
        ret->function = func;
        ret->parameters[0] = operand;
        return ret;
    }
    return base(s);
}

static inline LouroExpression *parse_expr_dynamic(state *s, int current_precedence) {
    LouroExpression *left = parse_prefix(s);
    if (!left)  { printf("NULL at %d\n", __LINE__); return NULL; };

    while (s->type == TOK_OPERATOR) {
        if (s->op_flags & LOURO_FLAG_POSTFIX) {
            if (s->op_precedence < current_precedence) break;
            const void *func = s->function;
            
            s->expecting_operator = 1;
            next_token(s);
            
            LouroExpression *new_left = new_expr(LOURO_FUNCTION1 | LOURO_FLAG_PURE, 0);
            if (!new_left) { louro_free(left);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            new_left->function = func;
            new_left->parameters[0] = left;
            left = new_left;
            continue;
        }

        if (s->op_flags & LOURO_FLAG_TERNARY) {
            if (s->op_precedence < current_precedence) break;
            
            int op_prec = s->op_precedence;
            int right_assoc = (s->op_flags & LOURO_FLAG_RIGHT_ASSOC);
            const void *func = s->function;
            const char *expected_separator = s->op_separator;
            int lazy_flag = (s->op_flags & LOURO_FLAG_LAZY);
            
            s->expecting_operator = 0;
            next_token(s);
            
            LouroExpression *middle = parse_expr_dynamic(s, 0);
            if (!middle) { louro_free(left);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            if (s->type != TOK_TERNARY_SEP || s->op_separator != expected_separator) {
                louro_free(left); louro_free(middle);  { printf("NULL at %d\n", __LINE__); return NULL; };
            }
            
            s->expecting_operator = 0;
            next_token(s);
            
            int next_prec = right_assoc ? op_prec : (op_prec + 1);
            LouroExpression *right = parse_expr_dynamic(s, next_prec);
            if (!right) { louro_free(left); louro_free(middle);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            LouroExpression *new_left = new_expr(LOURO_FUNCTION3 | LOURO_FLAG_PURE | lazy_flag, 0);
            if (!new_left) { louro_free(left); louro_free(middle); louro_free(right);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            new_left->function = func;
            new_left->parameters[0] = left;
            new_left->parameters[1] = middle;
            new_left->parameters[2] = right;
            left = new_left;
            continue;
        }

        if (s->op_flags & LOURO_FLAG_INFIX) {
            if (s->op_precedence < current_precedence) break;
            
            int op_prec = s->op_precedence;
            int right_assoc = (s->op_flags & LOURO_FLAG_RIGHT_ASSOC);
            const void *func = s->function;
            int lazy_flag = (s->op_flags & LOURO_FLAG_LAZY);
            
            s->expecting_operator = 0;
            next_token(s);
            
            int next_prec = right_assoc ? op_prec : (op_prec + 1);
            LouroExpression *right = parse_expr_dynamic(s, next_prec);
            if (!right) { louro_free(left);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            LouroExpression *new_left = new_expr(LOURO_FUNCTION2 | LOURO_FLAG_PURE | lazy_flag, 0);
            if (!new_left) { louro_free(left); louro_free(right);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            new_left->function = func;
            new_left->parameters[0] = left;
            new_left->parameters[1] = right;
            left = new_left;
            continue;
        }
        
        break;
    }
    return left;
}

#define LR_FUN(...) ((double(*)(__VA_ARGS__))n->function)
#define M(e) louro_evaluate((LouroExpression*)n->parameters[e])


static inline double louro_evaluate(const LouroExpression *n) {
    if (!n) return NAN;

    switch(TYPE_MASK(n->type)) {
        case LOURO_CONSTANT: return n->value;
        case LOURO_VARIABLE: return *n->bound;

        case LOURO_FUNCTION0: case LOURO_FUNCTION1: case LOURO_FUNCTION2: case LOURO_FUNCTION3:
        case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7:
        case LOURO_FUNCTION8: case LOURO_FUNCTION9: case LOURO_FUNCTION10: case LOURO_FUNCTION11:
        case LOURO_FUNCTION12: case LOURO_FUNCTION13: case LOURO_FUNCTION14: case LOURO_FUNCTION15:
            if (IS_LAZY(n->type)) {
                switch(ARITY(n->type)) {
                    case 0: return LR_FUN(void)();
                    case 1: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; return ((double(*)(double))n->function)((double)(uintptr_t)&l0); }
                    case 2: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; return ((double(*)(double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1); }
                    case 3: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; return ((double(*)(double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2); }
                    case 4: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; return ((double(*)(double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3); }
                    case 5: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; return ((double(*)(double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4); }
                    case 6: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; return ((double(*)(double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5); }
                    case 7: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; return ((double(*)(double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6); }
                    case 8: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; return ((double(*)(double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7); }
                    case 9: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; return ((double(*)(double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8); }
                    case 10: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; LouroLazy l9={louro_thunk_wrapper,n->parameters[9]}; return ((double(*)(double,double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8,(double)(uintptr_t)&l9); }
                    case 11: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; LouroLazy l9={louro_thunk_wrapper,n->parameters[9]}; LouroLazy l10={louro_thunk_wrapper,n->parameters[10]}; return ((double(*)(double,double,double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8,(double)(uintptr_t)&l9,(double)(uintptr_t)&l10); }
                    case 12: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; LouroLazy l9={louro_thunk_wrapper,n->parameters[9]}; LouroLazy l10={louro_thunk_wrapper,n->parameters[10]}; LouroLazy l11={louro_thunk_wrapper,n->parameters[11]}; return ((double(*)(double,double,double,double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8,(double)(uintptr_t)&l9,(double)(uintptr_t)&l10,(double)(uintptr_t)&l11); }
                    case 13: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; LouroLazy l9={louro_thunk_wrapper,n->parameters[9]}; LouroLazy l10={louro_thunk_wrapper,n->parameters[10]}; LouroLazy l11={louro_thunk_wrapper,n->parameters[11]}; LouroLazy l12={louro_thunk_wrapper,n->parameters[12]}; return ((double(*)(double,double,double,double,double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8,(double)(uintptr_t)&l9,(double)(uintptr_t)&l10,(double)(uintptr_t)&l11,(double)(uintptr_t)&l12); }
                    case 14: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; LouroLazy l9={louro_thunk_wrapper,n->parameters[9]}; LouroLazy l10={louro_thunk_wrapper,n->parameters[10]}; LouroLazy l11={louro_thunk_wrapper,n->parameters[11]}; LouroLazy l12={louro_thunk_wrapper,n->parameters[12]}; LouroLazy l13={louro_thunk_wrapper,n->parameters[13]}; return ((double(*)(double,double,double,double,double,double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8,(double)(uintptr_t)&l9,(double)(uintptr_t)&l10,(double)(uintptr_t)&l11,(double)(uintptr_t)&l12,(double)(uintptr_t)&l13); }
                    case 15: { LouroLazy l0={louro_thunk_wrapper,n->parameters[0]}; LouroLazy l1={louro_thunk_wrapper,n->parameters[1]}; LouroLazy l2={louro_thunk_wrapper,n->parameters[2]}; LouroLazy l3={louro_thunk_wrapper,n->parameters[3]}; LouroLazy l4={louro_thunk_wrapper,n->parameters[4]}; LouroLazy l5={louro_thunk_wrapper,n->parameters[5]}; LouroLazy l6={louro_thunk_wrapper,n->parameters[6]}; LouroLazy l7={louro_thunk_wrapper,n->parameters[7]}; LouroLazy l8={louro_thunk_wrapper,n->parameters[8]}; LouroLazy l9={louro_thunk_wrapper,n->parameters[9]}; LouroLazy l10={louro_thunk_wrapper,n->parameters[10]}; LouroLazy l11={louro_thunk_wrapper,n->parameters[11]}; LouroLazy l12={louro_thunk_wrapper,n->parameters[12]}; LouroLazy l13={louro_thunk_wrapper,n->parameters[13]}; LouroLazy l14={louro_thunk_wrapper,n->parameters[14]}; return ((double(*)(double,double,double,double,double,double,double,double,double,double,double,double,double,double,double))n->function)((double)(uintptr_t)&l0,(double)(uintptr_t)&l1,(double)(uintptr_t)&l2,(double)(uintptr_t)&l3,(double)(uintptr_t)&l4,(double)(uintptr_t)&l5,(double)(uintptr_t)&l6,(double)(uintptr_t)&l7,(double)(uintptr_t)&l8,(double)(uintptr_t)&l9,(double)(uintptr_t)&l10,(double)(uintptr_t)&l11,(double)(uintptr_t)&l12,(double)(uintptr_t)&l13,(double)(uintptr_t)&l14); }

                    default: return NAN;
                }
            }
            switch(ARITY(n->type)) {
                case 0: return LR_FUN(void)();
                case 1: {
                    double m0 = M(0);
                    return LR_FUN(double)(m0);
                }
                case 2: {
                    double m0 = M(0); double m1 = M(1);
                    return LR_FUN(double, double)(m0, m1);
                }
                case 3: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2);
                    return LR_FUN(double, double, double)(m0, m1, m2);
                }
                case 4: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3);
                    return LR_FUN(double, double, double, double)(m0, m1, m2, m3);
                }
                case 5: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4);
                    return LR_FUN(double, double, double, double, double)(m0, m1, m2, m3, m4);
                }
                case 6: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5);
                    return LR_FUN(double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5);
                }
                case 7: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6);
                    return LR_FUN(double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6);
                }
                case 8: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7);
                    return LR_FUN(double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7);
                }
                case 9: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8);
                    return LR_FUN(double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8);
                }
                case 10: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9);
                    return LR_FUN(double, double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9);
                }
                case 11: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10);
                    return LR_FUN(double, double, double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10);
                }
                case 12: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11);
                    return LR_FUN(double, double, double, double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
                }
                case 13: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11); double m12 = M(12);
                    return LR_FUN(double, double, double, double, double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
                }
                case 14: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11); double m12 = M(12); double m13 = M(13);
                    return LR_FUN(double, double, double, double, double, double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13);
                }
                case 15: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11); double m12 = M(12); double m13 = M(13); double m14 = M(14);
                    return LR_FUN(double, double, double, double, double, double, double, double, double, double, double, double, double, double, double)(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14);
                }
                default: return NAN;
            }

        case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3:
        case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
        case LOURO_CLOSURE8: case LOURO_CLOSURE9: case LOURO_CLOSURE10: case LOURO_CLOSURE11:
        case LOURO_CLOSURE12: case LOURO_CLOSURE13: case LOURO_CLOSURE14: case LOURO_CLOSURE15:
            switch(ARITY(n->type)) {
                case 0: return LR_FUN(void*)(n->parameters[0]);
                case 1: {
                    double m0 = M(0);
                    return LR_FUN(void*, double)(n->parameters[1], m0);
                }
                case 2: {
                    double m0 = M(0); double m1 = M(1);
                    return LR_FUN(void*, double, double)(n->parameters[2], m0, m1);
                }
                case 3: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2);
                    return LR_FUN(void*, double, double, double)(n->parameters[3], m0, m1, m2);
                }
                case 4: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3);
                    return LR_FUN(void*, double, double, double, double)(n->parameters[4], m0, m1, m2, m3);
                }
                case 5: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4);
                    return LR_FUN(void*, double, double, double, double, double)(n->parameters[5], m0, m1, m2, m3, m4);
                }
                case 6: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5);
                    return LR_FUN(void*, double, double, double, double, double, double)(n->parameters[6], m0, m1, m2, m3, m4, m5);
                }
                case 7: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6);
                    return LR_FUN(void*, double, double, double, double, double, double, double)(n->parameters[7], m0, m1, m2, m3, m4, m5, m6);
                }
                case 8: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double)(n->parameters[8], m0, m1, m2, m3, m4, m5, m6, m7);
                }
                case 9: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double)(n->parameters[9], m0, m1, m2, m3, m4, m5, m6, m7, m8);
                }
                case 10: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double, double)(n->parameters[10], m0, m1, m2, m3, m4, m5, m6, m7, m8, m9);
                }
                case 11: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double, double, double)(n->parameters[11], m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10);
                }
                case 12: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double, double, double, double)(n->parameters[12], m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
                }
                case 13: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11); double m12 = M(12);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double, double, double, double, double)(n->parameters[13], m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
                }
                case 14: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11); double m12 = M(12); double m13 = M(13);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double)(n->parameters[14], m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13);
                }
                case 15: {
                    double m0 = M(0); double m1 = M(1); double m2 = M(2); double m3 = M(3); double m4 = M(4); double m5 = M(5); double m6 = M(6); double m7 = M(7); double m8 = M(8); double m9 = M(9); double m10 = M(10); double m11 = M(11); double m12 = M(12); double m13 = M(13); double m14 = M(14);
                    return LR_FUN(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double)(n->parameters[15], m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14);
                }
                default: return NAN;
            }

        default: return NAN;
    }

}

#undef LR_FUN
#undef M

static inline void optimize(LouroExpression *n) {
    /* Evaluates as much as possible. */
    if (n->type == LOURO_CONSTANT) return;
    if (n->type == LOURO_VARIABLE) return;

    /* Only optimize out functions flagged as pure. */
    if (IS_PURE(n->type)) {
        const int arity = ARITY(n->type);
        int known = 1;
        int i;
        for (i = 0; i < arity; ++i) {
            optimize((LouroExpression*)n->parameters[i]);
            if (((LouroExpression*)(n->parameters[i]))->type != LOURO_CONSTANT) {
                known = 0;
            }
        }
        if (known) {
            const double value = louro_evaluate(n);
            louro_free_parameters(n);
            n->type = LOURO_CONSTANT;
            n->value = value;
        }
    }
}


static inline LouroExpression *louro_compile_ex(const char *expression, const LouroVariable *variables, int var_count, LouroLookupCallback lookup_cb, void *lookup_ctx, int *error) {
    state s = { 0 };
    s.start = s.next = expression;
    s.context = 0;
    s.lookup = variables;
    s.lookup_len = var_count;
    s.lookup_cb = lookup_cb;
    s.lookup_ctx = lookup_ctx;
    s.expecting_operator = 0;

    next_token(&s);
    LouroExpression *root = parse_expr_dynamic(&s, 0);
    if (root == NULL) {
        if (error) *error = -1;
         { printf("NULL at %d\n", __LINE__); return NULL; };
    }

    if (s.type != TOK_END) {
        louro_free(root);
        if (error) {
            *error = (s.next - s.start);
            if (*error == 0) *error = 1;
        }
        return 0;
    } else {
        optimize(root);
        if (error) *error = 0;
        return root;
    }
}

#ifdef __cplusplus
}
#endif

#endif /*LOURO_H*/
