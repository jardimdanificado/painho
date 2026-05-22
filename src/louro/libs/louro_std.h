#ifndef LOURO_STD_H
#define LOURO_STD_H

#include "../louro.h"
#include <math.h>

/* Basic Math Wrappers */
static inline double lr_add(double a, double b) {return a + b;}
static inline double lr_sub(double a, double b) {return a - b;}
static inline double lr_mul(double a, double b) {return a * b;}
static inline double lr_divide(double a, double b) {return a / b;}
static inline double lr_negate(double a) {return -a;}
static inline double lr_comma(double a, double b) {(void)a; return b;}

/* Comparison Wrappers — return 1.0 for true, 0.0 for false */
static inline double lr_cmp_lt(double a, double b)  {return a <  b ? 1.0 : 0.0;}
static inline double lr_cmp_gt(double a, double b)  {return a >  b ? 1.0 : 0.0;}
static inline double lr_cmp_le(double a, double b)  {return a <= b ? 1.0 : 0.0;}
static inline double lr_cmp_ge(double a, double b)  {return a >= b ? 1.0 : 0.0;}
static inline double lr_cmp_eq(double a, double b)  {return a == b ? 1.0 : 0.0;}
static inline double lr_cmp_ne(double a, double b)  {return a != b ? 1.0 : 0.0;}

/* 
 * Standard Operator Library
 * You can inject this directly into your context.
 */
#define LOURO_STD \
    LOURO_OP("+", lr_add, 30), \
    LOURO_OP("-", lr_sub, 30), \
    LOURO_OP_PREFIX("-", lr_negate, 60), \
    LOURO_OP_PREFIX("+", lr_add, 60), \
    LOURO_OP("*", lr_mul, 40), \
    LOURO_OP("/", lr_divide, 40), \
    LOURO_OP("%", fmod, 40), \
    LOURO_OP_RIGHT("^", pow, 50), \
    LOURO_OP("<", lr_cmp_lt, 20), \
    LOURO_OP(">", lr_cmp_gt, 20), \
    LOURO_OP("<=", lr_cmp_le, 20), \
    LOURO_OP(">=", lr_cmp_ge, 20), \
    LOURO_OP("==", lr_cmp_eq, 20), \
    LOURO_OP("!=", lr_cmp_ne, 20)

#endif // LOURO_STD_H
