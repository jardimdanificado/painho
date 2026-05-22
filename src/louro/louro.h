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

    LOURO_FUNCTION0 = 8, LOURO_FUNCTION1, LOURO_FUNCTION2, LOURO_FUNCTION3,
    LOURO_FUNCTION4, LOURO_FUNCTION5, LOURO_FUNCTION6, LOURO_FUNCTION7,

    LOURO_CLOSURE0 = 16, LOURO_CLOSURE1, LOURO_CLOSURE2, LOURO_CLOSURE3,
    LOURO_CLOSURE4, LOURO_CLOSURE5, LOURO_CLOSURE6, LOURO_CLOSURE7,

    LOURO_FLAG_PURE = 32,
    LOURO_OPERATOR = 64,
    LOURO_FLAG_RIGHT_ASSOC = 128,
    
    LOURO_FLAG_INFIX = 256,
    LOURO_FLAG_PREFIX = 512,
    LOURO_FLAG_POSTFIX = 1024
};

/* Builtin function descriptor (used internally; no variable binding). */
typedef struct LouroVariable {
    const char *name;
    const void *address;
    int type;
    void *context;
} LouroVariable;
#define LOURO_VAR(name, ptr) {name, (const void*)(ptr), LOURO_VARIABLE, 0}

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    /* Helper macro to automatically detect function arity (C11 only) */
    #define LOURO_ARITY(func) _Generic((func), \
        double (*)(void): LOURO_FUNCTION0, \
        double (*)(double): LOURO_FUNCTION1, \
        double (*)(double, double): LOURO_FUNCTION2, \
        double (*)(double, double, double): LOURO_FUNCTION3, \
        double (*)(double, double, double, double): LOURO_FUNCTION4, \
        double (*)(double, double, double, double, double): LOURO_FUNCTION5, \
        double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, \
        double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 \
    )

    #define LOURO_PURE(name, func)   {name, (const void*)(func), LOURO_ARITY(func) | LOURO_FLAG_PURE, 0}
    #define LOURO_IMPURE(name, func) {name, (const void*)(func), LOURO_ARITY(func), 0}
#endif

#define LOURO_OP(name, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((prec) << 12), 0}
#define LOURO_OP_RIGHT(name, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FLAG_RIGHT_ASSOC | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((prec) << 12), 0}
#define LOURO_OP_PREFIX(name, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_PREFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | ((prec) << 12), 0}
#define LOURO_OP_POSTFIX(name, func, prec) {name, (const void*)(func), LOURO_OPERATOR | LOURO_FLAG_POSTFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | ((prec) << 12), 0}

/* Parses the input expression. */
/* Returns NULL on error. */
static inline LouroExpression *louro_compile(const char *expression, const LouroVariable *variables, int var_count, int *error);

/* Evaluates the expression. */
static inline double louro_evaluate(const LouroExpression *n);

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
    TOK_NULL = LOURO_CLOSURE7+1, TOK_ERROR, TOK_END, TOK_SEP,
    TOK_OPEN, TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_OPERATOR
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
    
    int expecting_operator;
    int op_precedence;
    int op_flags;
} state;


#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

#define IS_PURE(TYPE) (((TYPE) & LOURO_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE) & LOURO_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE) (((TYPE) & LOURO_CLOSURE0) != 0)
#define ARITY(TYPE) ( ((TYPE) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((TYPE) & 0x00000007) : 0 )
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
                            if (!(var->type & (LOURO_FLAG_INFIX | LOURO_FLAG_POSTFIX))) continue;
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
                s->op_precedence = (best_match->type >> 12);
                s->op_flags = best_match->type & (LOURO_FLAG_RIGHT_ASSOC | LOURO_FLAG_INFIX | LOURO_FLAG_PREFIX | LOURO_FLAG_POSTFIX);
                return;
            } else {
                switch(TYPE_MASK(best_match->type)) {
                    case LOURO_VARIABLE:
                        s->type = TOK_VARIABLE;
                        s->bound = (const double*)best_match->address;
                        return;
                    case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3:
                    case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
                        s->context = best_match->context;
                    case LOURO_FUNCTION0: case LOURO_FUNCTION1: case LOURO_FUNCTION2: case LOURO_FUNCTION3:
                    case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7:
                        s->type = best_match->type;
                        s->function = best_match->address;
                        return;
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
                if (isalpha(s->next[0])) {
                    while (isalpha(s->next[0]) || isdigit(s->next[0]) || (s->next[0] == '_')) s->next++;
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

        case LOURO_FUNCTION2: case LOURO_FUNCTION3: case LOURO_FUNCTION4:
        case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7:
        case LOURO_CLOSURE2: case LOURO_CLOSURE3: case LOURO_CLOSURE4:
        case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
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
        
        s->expecting_operator = 0;
        next_token(s);
        
        LouroExpression *operand = parse_expr_dynamic(s, prec);
        if (!operand)  { printf("NULL at %d\n", __LINE__); return NULL; };
        
        LouroExpression *ret = new_expr(LOURO_FUNCTION1 | LOURO_FLAG_PURE, 0);
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

        if (s->op_flags & LOURO_FLAG_INFIX) {
            if (s->op_precedence < current_precedence) break;
            
            int op_prec = s->op_precedence;
            int right_assoc = (s->op_flags & LOURO_FLAG_RIGHT_ASSOC);
            const void *func = s->function;
            
            s->expecting_operator = 0;
            next_token(s);
            
            int next_prec = right_assoc ? op_prec : (op_prec + 1);
            LouroExpression *right = parse_expr_dynamic(s, next_prec);
            if (!right) { louro_free(left);  { printf("NULL at %d\n", __LINE__); return NULL; }; }
            
            LouroExpression *new_left = new_expr(LOURO_FUNCTION2 | LOURO_FLAG_PURE, 0);
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
            switch(ARITY(n->type)) {
                case 0: return LR_FUN(void)();
                case 1: return LR_FUN(double)(M(0));
                case 2: return LR_FUN(double, double)(M(0), M(1));
                case 3: return LR_FUN(double, double, double)(M(0), M(1), M(2));
                case 4: return LR_FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
                case 5: return LR_FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
                case 6: return LR_FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return LR_FUN(double, double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3:
        case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
            switch(ARITY(n->type)) {
                case 0: return LR_FUN(void*)(n->parameters[0]);
                case 1: return LR_FUN(void*, double)(n->parameters[1], M(0));
                case 2: return LR_FUN(void*, double, double)(n->parameters[2], M(0), M(1));
                case 3: return LR_FUN(void*, double, double, double)(n->parameters[3], M(0), M(1), M(2));
                case 4: return LR_FUN(void*, double, double, double, double)(n->parameters[4], M(0), M(1), M(2), M(3));
                case 5: return LR_FUN(void*, double, double, double, double, double)(n->parameters[5], M(0), M(1), M(2), M(3), M(4));
                case 6: return LR_FUN(void*, double, double, double, double, double, double)(n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return LR_FUN(void*, double, double, double, double, double, double, double)(n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
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


static inline LouroExpression *louro_compile(const char *expression, const LouroVariable *variables, int var_count, int *error) {
    state s = { 0 };
    s.start = s.next = expression;
    s.context = 0;
    s.lookup = variables;
    s.lookup_len = var_count;
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
