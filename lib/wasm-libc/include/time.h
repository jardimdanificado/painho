#ifndef _TIME_H
#define _TIME_H

#include <stddef.h>
#include <stdint.h>

/* ── tipos ───────────────────────────────────────────────────────────────── */

typedef uint32_t  time_t;       /* segundos desde epoch (32 bits em wasm32) */
typedef uint32_t  clock_t;      /* ticks de CPU */
typedef int32_t   suseconds_t;  /* microssegundos */

#define CLOCKS_PER_SEC  1000u   /* clock() retorna milissegundos */

struct tm {
    int tm_sec;    /* segundos      [0, 60]  */
    int tm_min;    /* minutos       [0, 59]  */
    int tm_hour;   /* horas         [0, 23]  */
    int tm_mday;   /* dia do mês    [1, 31]  */
    int tm_mon;    /* mês           [0, 11]  */
    int tm_year;   /* anos desde 1900        */
    int tm_wday;   /* dia da semana [0, 6]   */
    int tm_yday;   /* dia do ano    [0, 365] */
    int tm_isdst;  /* horário de verão: >0 sim, 0 não, <0 desconhecido */
};

struct timespec {
    time_t  tv_sec;   /* segundos           */
    int32_t tv_nsec;  /* nanossegundos [0, 999999999] */
};

/* ── imports do host ─────────────────────────────────────────────────────── */

/*
 * Estas funções devem ser registradas pelo host via m3_LinkRawFunction.
 * Ver runtime/wasm3_host.c para o binding.
 *
 * __host_time()         → segundos desde Unix epoch (UTC)
 * __host_clock_ms()     → milissegundos desde início do processo
 */
__attribute__((import_module("env"), import_name("__host_time")))
extern uint32_t __host_time(void);

__attribute__((import_module("env"), import_name("__host_clock_ms")))
extern uint32_t __host_clock_ms(void);

/* ── funções ─────────────────────────────────────────────────────────────── */

time_t  time   (time_t *tloc);
clock_t clock  (void);
double  difftime(time_t t1, time_t t0);

/* conversão entre time_t e struct tm (calendário gregoriano, sem timezone) */
struct tm *gmtime   (const time_t *timep);
struct tm *localtime(const time_t *timep);   /* sem TZ: igual a gmtime */
time_t     mktime   (struct tm *tm);

/* formatação */
size_t strftime(char *buf, size_t max, const char *fmt, const struct tm *tm);

#endif /* _TIME_H */
