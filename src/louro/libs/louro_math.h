#ifndef LOURO_MATH_H
#define LOURO_MATH_H

#include "../louro.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline double lr_pi(void) { return 3.14159265358979323846; }

#define LOURO_MATH \
    LOURO_PURE("sqrt", (double(*)(double))sqrt), \
    LOURO_PURE("sin", (double(*)(double))sin), \
    LOURO_PURE("cos", (double(*)(double))cos), \
    LOURO_PURE("tan", (double(*)(double))tan), \
    LOURO_PURE("asin", (double(*)(double))asin), \
    LOURO_PURE("acos", (double(*)(double))acos), \
    LOURO_PURE("atan", (double(*)(double))atan), \
    LOURO_PURE("atan2", (double(*)(double, double))atan2), \
    LOURO_PURE("abs", (double(*)(double))fabs), \
    LOURO_PURE("fabs", (double(*)(double))fabs), \
    LOURO_PURE("log", (double(*)(double))log), \
    LOURO_PURE("log10", (double(*)(double))log10), \
    LOURO_PURE("exp", (double(*)(double))exp), \
    LOURO_PURE("ceil", (double(*)(double))ceil), \
    LOURO_PURE("floor", (double(*)(double))floor), \
    LOURO_PURE("pi", (double(*)(void))lr_pi)

#ifdef __cplusplus
}
#endif

#endif // LOURO_MATH_H
