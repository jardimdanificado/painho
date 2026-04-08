#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

/* ── ABI de I/O com o host (wasm3) ──────────────────────────────────────── */
/*
 * Estas funções são importadas do ambiente host.
 * No wasm3 você as registra com m3_LinkRawFunction.
 *
 * Convenção de módulo: "env"
 */
extern void __host_write(const char *buf, int len);  /* fd stdout implícito */
extern void __host_write_err(const char *buf, int len); /* stderr           */

/* ── printf família ──────────────────────────────────────────────────────── */
int printf (const char *fmt, ...);
int fprintf_stdout(const char *fmt, ...);   /* stdout explícito */
int fprintf_stderr(const char *fmt, ...);   /* stderr           */

int sprintf (char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t size, const char *fmt, ...);

int vprintf (const char *fmt, va_list ap);
int vsprintf (char *buf, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

/* ── putchar / puts ──────────────────────────────────────────────────────── */
int putchar(int c);
int puts   (const char *s);


/* ── scanf família ───────────────────────────────────────────────────────── */
int scanf  (const char *fmt, ...);
int sscanf (const char *buf, const char *fmt, ...);
int vscanf (const char *fmt, va_list ap);
int vsscanf(const char *buf, const char *fmt, va_list ap);

#endif /* _STDIO_H */
