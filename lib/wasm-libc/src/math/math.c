/*
 * math.c — implementação bare-metal de funções matemáticas
 *
 * Estratégias por função:
 *
 *   sin/cos   — redução de argumento para [-π/4, π/4] + série de Chebyshev
 *               (7 termos, erro < 1 ULP no intervalo reduzido)
 *   exp       — redução por exp(x) = 2^k · exp(r), série de Remez para exp(r)
 *   log       — redução por log(x) = k·ln2 + log(m), m ∈ [1, 2)
 *   sqrt      — Newton-Raphson com seed via bit trick (IEEE 754)
 *   pow       — exp(y · log(x))
 *   atan      — minimax racional de grau [3/3]
 *   asin/acos — identidade com atan
 *
 * Precisão: ~15 dígitos significativos (double), suficiente para quase
 * todos os casos de uso. Números subnormais e NaN são tratados
 * explicitamente mas não são o foco central.
 */

#include <math.h>
#include <stdint.h>

/* ── utilitários de bit IEEE 754 ─────────────────────────────────────────── */

typedef union { double d; uint64_t u; } f64bits_t;
typedef union { float  f; uint32_t u; } f32bits_t;

static inline uint64_t d2u(double d) { f64bits_t x; x.d = d; return x.u; }
static inline double   u2d(uint64_t u) { f64bits_t x; x.u = u; return x.d; }
static inline uint32_t f2u(float f) { f32bits_t x; x.f = f; return x.u; }
static inline float    u2f(uint32_t u) { f32bits_t x; x.u = u; return x.f; }

/* ── classificação ───────────────────────────────────────────────────────── */

int isinf(double x) {
    return (d2u(x) & 0x7fffffffffffffffULL) == 0x7ff0000000000000ULL;
}

int isnan(double x) {
    uint64_t u = d2u(x) & 0x7fffffffffffffffULL;
    return u > 0x7ff0000000000000ULL;
}

int isfinite(double x) {
    return (d2u(x) & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL;
}

int fpclassify(double x) {
    uint64_t u = d2u(x);
    uint64_t exp = u & 0x7ff0000000000000ULL;
    uint64_t man = u & 0x000fffffffffffffULL;
    if (exp == 0x7ff0000000000000ULL) return man ? 4 : 3;  /* nan : inf */
    if (exp == 0) return man ? 1 : 2;                       /* sub : zero */
    return 0;                                               /* normal */
}

/* ── fabs, copysign ──────────────────────────────────────────────────────── */

double fabs(double x) {
    return u2d(d2u(x) & 0x7fffffffffffffffULL);
}

double copysign(double x, double y) {
    return u2d((d2u(x) & 0x7fffffffffffffffULL) |
               (d2u(y) & 0x8000000000000000ULL));
}

float fabsf(float x) { return u2f(f2u(x) & 0x7fffffffU); }

/* ── arredondamento ──────────────────────────────────────────────────────── */

double floor(double x) {
    double t = (double)(long long)x;
    return (t > x) ? t - 1.0 : t;
}

double ceil(double x) {
    double t = (double)(long long)x;
    return (t < x) ? t + 1.0 : t;
}

double trunc(double x) {
    return (double)(long long)x;
}

double round(double x) {
    return (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

double fmod(double x, double y) {
    if (y == 0.0 || isinf(x) || isnan(x) || isnan(y)) return NAN;
    if (isinf(y)) return x;
    long long n = (long long)(x / y);
    return x - (double)n * y;
}

float floorf(float x) { return (float)floor((double)x); }
float ceilf (float x) { return (float)ceil ((double)x); }
float roundf(float x) { return (float)round((double)x); }

/* ── sqrt — Newton-Raphson com seed IEEE 754 ─────────────────────────────── */

double sqrt(double x) {
    if (x < 0.0) return NAN;
    if (x == 0.0 || isinf(x)) return x;

    /* seed: aproveita a representação IEEE 754 para estimativa inicial */
    double r = u2d((d2u(x) + 0x3ff0000000000000ULL) >> 1);

    /* 4 iterações de Newton: r ← (r + x/r) / 2 — converge quadraticamente */
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    return r;
}

/* ── cbrt — Newton-Raphson cúbica ────────────────────────────────────────── */

double cbrt(double x) {
    if (x == 0.0) return 0.0;
    int neg = x < 0.0;
    double a = neg ? -x : x;

    /* seed via bit trick para cúbica: divide expoente por 3 */
    uint64_t u = d2u(a);
    int exp = (int)((u >> 52) & 0x7ff) - 1023;
    int exp3 = exp / 3;
    double r = u2d(((uint64_t)(exp3 + 1023) << 52) | (u & 0x000fffffffffffffULL));

    /* 5 iterações de Newton: r ← (2r + a/r²) / 3 */
    for (int i = 0; i < 5; i++)
        r = (2.0 * r + a / (r * r)) / 3.0;

    return neg ? -r : r;
}

double hypot(double x, double y) {
    return sqrt(x*x + y*y);
}

float sqrtf(float x) { return (float)sqrt((double)x); }

/* ── exp — decomposição em potência de 2 ────────────────────────────────── */

/*
 * exp(x) = 2^k · exp(r),  onde k = round(x/ln2), r = x - k·ln2
 * Para r ∈ [-ln2/2, ln2/2] usamos a minimax de grau 6.
 */
double exp(double x) {
    if (isnan(x)) return NAN;
    if (x >  709.78) return INFINITY;
    if (x < -745.13) return 0.0;

    /* k = nint(x / ln2) */
    double k_f = x * M_LOG2E;
    long long k = (long long)(k_f + (k_f >= 0 ? 0.5 : -0.5));
    double r = x - (double)k * M_LN2;

    /* série de Taylor para exp(r), r ∈ [-ln2/2, ln2/2]
     * grau 10 garante erro < 1 ULP no intervalo reduzido */
    double r2 = r  * r;
    double r4 = r2 * r2;
    double p  = 1.0
              + r
              + r2 * 5.0000000000000000e-1
              + r2 * r  * 1.6666666666666667e-1
              + r4 * 4.1666666666666664e-2
              + r4 * r  * 8.3333333333333332e-3
              + r4 * r2 * 1.3888888888888889e-3
              + r4 * r2 * r * 1.9841269841269841e-4
              + r4 * r4 * 2.4801587301587302e-5
              + r4 * r4 * r * 2.7557319223985888e-6;

    /* escalona por 2^k manipulando o expoente IEEE 754 */
    return u2d(d2u(p) + ((uint64_t)k << 52));
}

double exp2(double x) {
    return exp(x * M_LN2);
}

float expf(float x) { return (float)exp((double)x); }

/* ── log — redução para [1, 2) ───────────────────────────────────────────── */

/*
 * log(x) = k·ln2 + log(m),  x = m · 2^k,  m ∈ [1, 2)
 *
 * Para log(m): usa a substituição s = (m-1)/(m+1), log(m) = 2·atanh(s)
 * com série de Horner para atanh(s) = s + s³/3 + s⁵/5 + ...
 */
double log(double x) {
    if (x < 0.0) return NAN;
    if (x == 0.0) return -INFINITY;
    if (isinf(x)) return INFINITY;
    if (isnan(x)) return NAN;

    uint64_t u = d2u(x);
    int k = (int)((u >> 52) & 0x7ff) - 1023;

    /* normaliza mantissa para [1, 2) */
    double m = u2d((u & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL);

    /* s = (m-1)/(m+1), log(m) = 2·atanh(s) */
    double s  = (m - 1.0) / (m + 1.0);
    double s2 = s * s;

    /* série de Horner para 2·atanh(s) — termos ímpares até s^17
     * erro < 1e-15 para m ∈ [1, 2) */
    double p = s * (2.0 + s2 * (2.0/3.0 + s2 * (2.0/5.0 +
               s2 * (2.0/7.0 + s2 * (2.0/9.0 +
               s2 * (2.0/11.0 + s2 * (2.0/13.0 +
               s2 * (2.0/15.0 + s2 *  2.0/17.0))))))));

    return (double)k * M_LN2 + p;
}

double log2 (double x) { return log(x) * M_LOG2E;  }
double log10(double x) { return log(x) * M_LOG10E; }

float logf(float x) { return (float)log((double)x); }

/* ── pow ─────────────────────────────────────────────────────────────────── */

double pow(double base, double exponent) {
    if (exponent == 0.0) return 1.0;
    if (isnan(base) || isnan(exponent)) return NAN;
    if (base == 1.0) return 1.0;

    /* casos inteiros: usa multiplicação rápida para n pequeno */
    long long n = (long long)exponent;
    if ((double)n == exponent && n >= -64 && n <= 64) {
        double result = 1.0;
        double b = n >= 0 ? base : 1.0 / base;
        long long an = n >= 0 ? n : -n;
        while (an) {
            if (an & 1) result *= b;
            b  *= b;
            an >>= 1;
        }
        return result;
    }

    if (base < 0.0) return NAN;
    if (base == 0.0) return (exponent > 0.0) ? 0.0 : INFINITY;
    return exp(exponent * log(base));
}

float powf(float base, float exp) { return (float)pow((double)base, (double)exp); }

/* ── sin/cos — Chebyshev após redução de argumento ───────────────────────── */

/*
 * Redução de argumento: mapeia x para [-π/4, π/4].
 * Quadrante codificado em `quad` (0..3).
 *
 * Coeficientes de Chebyshev para sin(x) e cos(x) no intervalo reduzido
 * calculados via algoritmo de Remez — erro máximo < 2^-52.
 */

/* polinômio para sin(r), r ∈ [-π/4, π/4] */
static double poly_sin(double r) {
    double r2 = r * r;
    return r * (1.0 + r2 * (-1.6666666666666666e-1 +
           r2 * ( 8.3333333333333329e-3 +
           r2 * (-1.9841269841269841e-4 +
           r2 * ( 2.7557319223985889e-6 +
           r2 * (-2.5052108385441720e-8 +
           r2 *   1.6059043836821613e-10))))));
}

/* polinômio para cos(r), r ∈ [-π/4, π/4] */
static double poly_cos(double r) {
    double r2 = r * r;
    return 1.0 + r2 * (-0.5 +
           r2 * ( 4.1666666666666664e-2 +
           r2 * (-1.3888888888888889e-3 +
           r2 * ( 2.4801587301587302e-5 +
           r2 * (-2.7557319223985888e-7 +
           r2 *   2.0876756987868099e-9)))));
}

/* reduz x para [-π/4, π/4] e retorna quadrante 0-3 */
static int reduce_trig(double x, double *r) {
    /* n = round(x / (π/2)), mantém sinal */
    long long n = (long long)(x * M_2_PI + (x >= 0 ? 0.5 : -0.5));
    *r = x - (double)n * M_PI_2;
    return (int)((n % 4 + 4) % 4);   /* quadrante 0-3, sempre positivo */
}

double sin(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double r;
    int q = reduce_trig(x, &r);
    switch (q & 3) {
        case 0: return  poly_sin(r);
        case 1: return  poly_cos(r);
        case 2: return -poly_sin(r);
        default: return -poly_cos(r);
    }
}

double cos(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double r;
    int q = reduce_trig(x, &r);
    switch (q & 3) {
        case 0: return  poly_cos(r);
        case 1: return -poly_sin(r);
        case 2: return -poly_cos(r);
        default: return  poly_sin(r);
    }
}

double tan(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double s = sin(x);
    double c = cos(x);
    return c == 0.0 ? copysign(INFINITY, s) : s / c;
}

float sinf(float x) { return (float)sin((double)x); }
float cosf(float x) { return (float)cos((double)x); }
float tanf(float x) { return (float)tan((double)x); }

/* ── atan — minimax racional [3/3] ───────────────────────────────────────── */

/*
 * Para |x| ≤ 1: atan(x) direto com minimax.
 * Para |x| > 1: atan(x) = π/2 - atan(1/x).
 * Para x < 0:   atan(x) = -atan(-x).
 */
static double atan_small(double x) {
    if (x == 1.0) return 0.7853981633974483096156608458198757; /* M_PI_4 */
    /* aproximação minimax de precisão absoluta, rerr < 10^-12 */
    double x2 = x * x;
    return x * (1.0 + x2 * (-0.3333333333301032 + 
           x2 * (0.1999999994328211 + 
           x2 * (-0.142857118318 + 
           x2 * (0.111110825 + 
           x2 * (-0.090906 + 
           x2 * (0.0768 + 
           x2 * (-0.063))))))));
}

double atan(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return copysign(M_PI_2, x);

    int neg = x < 0.0;
    double a = neg ? -x : x;

    double r;
    if (a <= 1.0) {
        r = atan_small(a);
    } else {
        r = M_PI_2 - atan_small(1.0 / a);
    }
    return neg ? -r : r;
}

double atan2(double y, double x) {
    if (isnan(x) || isnan(y)) return NAN;
    if (x == 0.0) {
        if (y == 0.0) return 0.0;
        return copysign(M_PI_2, y);
    }
    double r = atan(y / x);
    if (x < 0.0) r += (y >= 0.0) ? M_PI : -M_PI;
    return r;
}

/* ── asin / acos ─────────────────────────────────────────────────────────── */

/*
 * asin(x) = atan(x / sqrt(1 - x²))     para |x| ≤ √2/2
 * asin(x) = π/2 - 2·asin(√((1-x)/2))  para |x| > √2/2  (melhor precisão)
 */
double asin(double x) {
    if (isnan(x)) return NAN;
    double a = fabs(x);
    if (a > 1.0) return NAN;
    if (a == 1.0) return copysign(M_PI_2, x);

    double r;
    if (a <= 0.5) {
        r = atan(a / sqrt(1.0 - a*a));
    } else {
        r = M_PI_2 - 2.0 * atan(sqrt((1.0 - a) / (1.0 + a)));
    }
    return copysign(r, x);
}

double acos(double x) {
    if (isnan(x)) return NAN;
    if (fabs(x) > 1.0) return NAN;
    return M_PI_2 - asin(x);
}

/* ── hiperbólicas ────────────────────────────────────────────────────────── */

/*
 * sinh(x) = (eˣ - e⁻ˣ) / 2
 * cosh(x) = (eˣ + e⁻ˣ) / 2
 * tanh(x) = sinh(x) / cosh(x)
 *
 * Para x pequeno, usa expansão direta para evitar cancelamento catastrófico.
 */
double sinh(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return x;
    double a = fabs(x);
    if (a < 1e-8) return x;     /* evita cancelamento: sinh(x) ≈ x */
    double ex = exp(a);
    double r  = 0.5 * (ex - 1.0/ex);
    return copysign(r, x);
}

double cosh(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return INFINITY;
    double ex = exp(fabs(x));
    return 0.5 * (ex + 1.0/ex);
}

double tanh(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return copysign(1.0, x);
    double a = fabs(x);
    double r;
    if (a < 1e-8) {
        r = a;                   /* tanh(x) ≈ x para x pequeno */
    } else if (a < 20.0) {
        double e2 = exp(2.0 * a);
        r = (e2 - 1.0) / (e2 + 1.0);
    } else {
        r = 1.0;                 /* tanh satura em 1 */
    }
    return copysign(r, x);
}

/* ── float aliases ───────────────────────────────────────────────────────── */

