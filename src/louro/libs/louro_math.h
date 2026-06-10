#ifndef LOURO_MATH_H
#define LOURO_MATH_H

#include "../louro.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline double lr_pi(void) { return 3.14159265358979323846; }

#define LOURO_MATH \
    LOURO_PURE("sqrt", sqrt, 1), \
    LOURO_PURE("sin", sin, 1), \
    LOURO_PURE("cos", cos, 1), \
    LOURO_PURE("tan", tan, 1), \
    LOURO_PURE("asin", asin, 1), \
    LOURO_PURE("acos", acos, 1), \
    LOURO_PURE("atan", atan, 1), \
    LOURO_PURE("atan2", atan2, 2), \
    LOURO_PURE("abs", fabs, 1), \
    LOURO_PURE("fabs", fabs, 1), \
    LOURO_PURE("log", log, 1), \
    LOURO_PURE("log10", log10, 1), \
    LOURO_PURE("exp", exp, 1), \
    LOURO_PURE("ceil", ceil, 1), \
    LOURO_PURE("floor", floor, 1), \
    LOURO_PURE("pi", lr_pi, 0)

#ifdef __cplusplus
}
#endif

#endif // LOURO_MATH_H
