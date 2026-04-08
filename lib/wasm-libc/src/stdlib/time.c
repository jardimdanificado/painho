/*
 * time.c — funções de tempo bare-metal para wasm3
 *
 * time() e clock() delegam para imports do host.
 * gmtime/mktime são implementados em C puro (calendário gregoriano).
 * strftime implementa os especificadores mais comuns.
 *
 * Não há suporte a timezones — todas as funções operam em UTC.
 */

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── time / clock ────────────────────────────────────────────────────────── */

time_t time(time_t *tloc) {
    time_t t = (time_t)__host_time();
    if (tloc) *tloc = t;
    return t;
}

clock_t clock(void) {
    return (clock_t)__host_clock_ms();
}

double difftime(time_t t1, time_t t0) {
    return (double)t1 - (double)t0;
}

/* ── calendário gregoriano ───────────────────────────────────────────────── */

/*
 * Tabela de dias acumulados por mês (ano não-bissexto e bissexto).
 * Índice 0 = janeiro, 11 = dezembro.
 */
static const short days_before[2][13] = {
    /* não-bissexto */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* bissexto     */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 },
};

static int is_leap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int days_in_year(int year) {
    return is_leap(year) ? 366 : 365;
}

/*
 * gmtime_r: converte time_t (segundos UTC desde 1970-01-01) para struct tm.
 * Algoritmo: divisão euclidiana de 400 anos (ciclo gregoriano completo).
 */
static struct tm _tm_buf;  /* buffer estático (não reentrante — ok para bare-metal) */

struct tm *gmtime(const time_t *timep) {
    int32_t secs = (int32_t)*timep;
    struct tm *t = &_tm_buf;

    /* horas, minutos, segundos */
    t->tm_sec  = secs % 60; secs /= 60;
    t->tm_min  = secs % 60; secs /= 60;
    t->tm_hour = secs % 24;
    int32_t days = secs / 24;   /* dias desde 1970-01-01 */

    /* dia da semana: 1970-01-01 foi quinta-feira (4) */
    t->tm_wday = (int)((days + 4) % 7);

    /* ano: começa em 1970 */
    int year = 1970;
    while (1) {
        int dy = days_in_year(year);
        if (days < dy) break;
        days -= dy;
        year++;
    }
    t->tm_year  = year - 1900;
    t->tm_yday  = (int)days;

    /* mês e dia */
    int leap = is_leap(year);
    int mon  = 0;
    while (mon < 11 && days >= days_before[leap][mon + 1])
        mon++;
    t->tm_mon  = mon;
    t->tm_mday = (int)(days - days_before[leap][mon]) + 1;
    t->tm_isdst = 0;

    return t;
}

struct tm *localtime(const time_t *timep) {
    /* sem suporte a timezone — delega para gmtime */
    return gmtime(timep);
}

/*
 * mktime: converte struct tm para time_t.
 * Normaliza campos fora do intervalo (ex: dia 32 → próximo mês).
 */
time_t mktime(struct tm *tm) {
    /* normaliza segundos/minutos/horas */
    int carry;

    carry = tm->tm_sec / 60; tm->tm_sec %= 60;
    if (tm->tm_sec < 0) { tm->tm_sec += 60; carry--; }
    tm->tm_min += carry;

    carry = tm->tm_min / 60; tm->tm_min %= 60;
    if (tm->tm_min < 0) { tm->tm_min += 60; carry--; }
    tm->tm_hour += carry;

    carry = tm->tm_hour / 24; tm->tm_hour %= 24;
    if (tm->tm_hour < 0) { tm->tm_hour += 24; carry--; }

    /* dias extras */
    int extra_days = carry;

    /* normaliza mês (pode overflow) */
    carry = tm->tm_mon / 12; tm->tm_mon %= 12;
    if (tm->tm_mon < 0) { tm->tm_mon += 12; carry--; }
    tm->tm_year += carry;

    /* normaliza dia dentro do mês */
    int year = tm->tm_year + 1900;
    int leap = is_leap(year);
    int dim  = days_before[leap][tm->tm_mon + 1] - days_before[leap][tm->tm_mon];

    while (tm->tm_mday > dim) {
        tm->tm_mday -= dim;
        tm->tm_mon++;
        if (tm->tm_mon == 12) { tm->tm_mon = 0; tm->tm_year++; }
        year = tm->tm_year + 1900;
        leap = is_leap(year);
        dim  = days_before[leap][tm->tm_mon + 1] - days_before[leap][tm->tm_mon];
    }
    while (tm->tm_mday < 1) {
        tm->tm_mon--;
        if (tm->tm_mon < 0) { tm->tm_mon = 11; tm->tm_year--; }
        year = tm->tm_year + 1900;
        leap = is_leap(year);
        dim  = days_before[leap][tm->tm_mon + 1] - days_before[leap][tm->tm_mon];
        tm->tm_mday += dim;
    }

    /* conta dias desde epoch */
    year = tm->tm_year + 1900;
    int32_t days = 0;
    for (int y = 1970; y < year; y++) days += days_in_year(y);
    days += days_before[is_leap(year)][tm->tm_mon];
    days += tm->tm_mday - 1;
    days += extra_days;

    /* atualiza campos derivados */
    tm->tm_wday = (int)((days + 4) % 7);
    tm->tm_yday = days_before[is_leap(year)][tm->tm_mon] + tm->tm_mday - 1;
    tm->tm_isdst = 0;

    return (time_t)(days * 86400 +
                    tm->tm_hour * 3600 +
                    tm->tm_min  * 60   +
                    tm->tm_sec);
}

/* ── strftime ────────────────────────────────────────────────────────────── */

static const char *wday_abbr[] = {
    "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static const char *wday_full[] = {
    "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};
static const char *mon_abbr[] = {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};
static const char *mon_full[] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
};

/*
 * strftime: formata data/hora em string.
 *
 * Especificadores suportados:
 *   %Y %y %m %d %H %M %S %j %u %w %A %a %B %b %p %X %x %c %Z %% %e %n %t
 */
size_t strftime(char *buf, size_t max, const char *fmt, const struct tm *tm) {
    size_t pos = 0;

#define PUTC(c) do { if (pos < max - 1) buf[pos] = (c); pos++; } while (0)
#define PUTS(s) do { \
    const char *_s = (s); \
    while (*_s) { PUTC(*_s++); } \
} while (0)
#define PUTD2(v) do { \
    int _v = (v); \
    PUTC('0' + (_v / 10) % 10); PUTC('0' + _v % 10); \
} while (0)
#define PUTD4(v) do { \
    int _v = (v); \
    PUTC('0' + (_v / 1000) % 10); PUTC('0' + (_v / 100) % 10); \
    PUTC('0' + (_v / 10)  % 10); PUTC('0' +  _v % 10); \
} while (0)

    while (*fmt && pos < max - 1) {
        if (*fmt != '%') { PUTC(*fmt++); continue; }
        fmt++;
        if (*fmt == '\0') break;

        switch (*fmt++) {
        case 'Y': PUTD4(tm->tm_year + 1900); break;
        case 'y': PUTD2((tm->tm_year + 1900) % 100); break;
        case 'm': PUTD2(tm->tm_mon + 1); break;
        case 'd': PUTD2(tm->tm_mday); break;
        case 'e': /* dia sem zero-pad */
            if (tm->tm_mday < 10) PUTC(' ');
            PUTC('0' + (tm->tm_mday % 10));
            break;
        case 'H': PUTD2(tm->tm_hour); break;
        case 'I': PUTD2(tm->tm_hour % 12 ? tm->tm_hour % 12 : 12); break;
        case 'M': PUTD2(tm->tm_min); break;
        case 'S': PUTD2(tm->tm_sec); break;
        case 'j': /* dia do ano 001-366 */ {
            int d = tm->tm_yday + 1;
            PUTC('0' + d/100); PUTC('0' + (d/10)%10); PUTC('0' + d%10);
            break;
        }
        case 'u': PUTC('0' + (tm->tm_wday == 0 ? 7 : tm->tm_wday)); break;
        case 'w': PUTC('0' + tm->tm_wday); break;
        case 'A': PUTS(wday_full[tm->tm_wday]); break;
        case 'a': PUTS(wday_abbr[tm->tm_wday]); break;
        case 'B': PUTS(mon_full[tm->tm_mon]);  break;
        case 'b': case 'h': PUTS(mon_abbr[tm->tm_mon]); break;
        case 'p': PUTS(tm->tm_hour < 12 ? "AM" : "PM"); break;
        case 'P': PUTS(tm->tm_hour < 12 ? "am" : "pm"); break;
        case 'X': /* HH:MM:SS */
            PUTD2(tm->tm_hour); PUTC(':');
            PUTD2(tm->tm_min);  PUTC(':');
            PUTD2(tm->tm_sec);  break;
        case 'x': /* DD/MM/YYYY */
            PUTD2(tm->tm_mday); PUTC('/');
            PUTD2(tm->tm_mon+1); PUTC('/');
            PUTD4(tm->tm_year+1900); break;
        case 'c': /* data e hora completas */
            PUTS(wday_abbr[tm->tm_wday]); PUTC(' ');
            PUTS(mon_abbr[tm->tm_mon]);   PUTC(' ');
            PUTD2(tm->tm_mday); PUTC(' ');
            PUTD2(tm->tm_hour); PUTC(':');
            PUTD2(tm->tm_min);  PUTC(':');
            PUTD2(tm->tm_sec);  PUTC(' ');
            PUTD4(tm->tm_year + 1900); break;
        case 'Z': PUTS("UTC"); break;
        case 'n': PUTC('\n'); break;
        case 't': PUTC('\t'); break;
        case '%': PUTC('%'); break;
        default:  PUTC('%'); PUTC(fmt[-1]); break;
        }
    }

    if (max > 0) buf[pos < max ? pos : max - 1] = '\0';
    return pos;

#undef PUTC
#undef PUTS
#undef PUTD2
#undef PUTD4
}
