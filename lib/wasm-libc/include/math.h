#ifndef _MATH_H
#define _MATH_H

/* ── constantes ──────────────────────────────────────────────────────────── */

#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.43429448190325182765
#define M_LN2       0.69314718055994530942
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.78539816339744830962
#define M_1_PI      0.31830988618379067154
#define M_2_PI      0.63661977236758134308
#define M_SQRT2     1.41421356237309504880
#define M_SQRT1_2   0.70710678118654752440

#define HUGE_VAL    __builtin_huge_val()
#define INFINITY    __builtin_inff()
#define NAN         __builtin_nanf("")

/* ── classificação ───────────────────────────────────────────────────────── */
int isinf (double x);
int isnan (double x);
int isfinite(double x);
int fpclassify(double x);   /* retorna: 0=normal 1=subnormal 2=zero 3=inf 4=nan */

/* ── trigonometria ───────────────────────────────────────────────────────── */
double sin  (double x);
double cos  (double x);
double tan  (double x);
double asin (double x);
double acos (double x);
double atan (double x);
double atan2(double y, double x);

/* ── hiperbólicas ────────────────────────────────────────────────────────── */
double sinh (double x);
double cosh (double x);
double tanh (double x);

/* ── exponencial e logaritmo ─────────────────────────────────────────────── */
double exp  (double x);
double exp2 (double x);
double log  (double x);
double log2 (double x);
double log10(double x);
double pow  (double base, double exp);

/* ── raiz e potência ─────────────────────────────────────────────────────── */
double sqrt (double x);
double cbrt (double x);
double hypot(double x, double y);

/* ── arredondamento ──────────────────────────────────────────────────────── */
double floor(double x);
double ceil (double x);
double round(double x);
double trunc(double x);
double fmod (double x, double y);

/* ── valor absoluto e sinal ──────────────────────────────────────────────── */
double fabs (double x);
double copysign(double x, double y);

/* ── float versions (delegam para double) ────────────────────────────────── */
float  sinf  (float x);
float  cosf  (float x);
float  tanf  (float x);
float  sqrtf (float x);
float  fabsf (float x);
float  floorf(float x);
float  ceilf (float x);
float  roundf(float x);
float  powf  (float base, float exp);
float  logf  (float x);
float  expf  (float x);

#endif /* _MATH_H */
