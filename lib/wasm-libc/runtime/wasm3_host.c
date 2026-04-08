#include <time.h>
/*
 * wasm3_host.c — binding do host para wasm-libc rodando em wasm3
 *
 * Compila junto com o seu programa host (não vai para o .wasm).
 *
 * Uso:
 *   #include "wasm3_host.h"
 *   ...
 *   IM3Module module = carregar_modulo(...);
 *   wasm_libc_link(runtime, module);
 *   ...
 *
 * Dependências: wasm3 headers (wasm3.h, m3_env.h)
 */

#include "wasm3_host.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── implementações das funções importadas ───────────────────────────────── */

/*
 * __host_write(buf: *u8, len: i32) -> void
 *
 * Escreve `len` bytes de `buf` (na memória WASM) para stdout.
 * m3_GetMemory garante que o ponteiro é válido dentro do linear memory.
 */
m3ApiRawFunction(m3_host_write_stdout) {
    m3ApiGetArgMem(const char *, buf);
    m3ApiGetArg(int32_t, len);

    if (buf && len > 0)
        fwrite(buf, 1, (size_t)len, stdout);

    m3ApiSuccess();
}

/*
 * __host_write_err(buf: *u8, len: i32) -> void
 */
m3ApiRawFunction(m3_host_write_stderr) {
    m3ApiGetArgMem(const char *, buf);
    m3ApiGetArg(int32_t, len);

    if (buf && len > 0)
        fwrite(buf, 1, (size_t)len, stderr);

    m3ApiSuccess();
}

/*
 * __host_abort() -> void
 *
 * Chamado por abort(). Pode ser customizado para logging, panic, etc.
 */
m3ApiRawFunction(m3_host_abort) {
    fprintf(stderr, "[wasm3-host] abort() chamado pelo módulo WASM\n");
    fflush(stderr);
    exit(1);
    m3ApiSuccess();
}

m3ApiRawFunction(m3_host_readline) {
    m3ApiReturnType(int32_t);
    m3ApiGetArgMem(char *, buf);
    m3ApiGetArg(int32_t, maxlen);

    if (!buf || maxlen <= 0) {
        m3ApiReturn(0);
    }

    if (fgets(buf, maxlen, stdin)) {
        size_t len = strlen(buf);
        m3ApiReturn((int32_t)len);
    }

    m3ApiReturn(0);
}

/* ── registro no runtime ─────────────────────────────────────────────────── */

/*
 * Assinaturas no formato wasm3:
 *   v = void, i = i32, I = i64, f = f32, F = f64
 *   * = ponteiro (i32 validado contra o linear memory)
 *
 *   "v(*i)"  → retorna void, recebe (ptr, i32)
 *   "v()"    → retorna void, sem argumentos
 */

m3ApiRawFunction(m3_host_time) {
    m3ApiReturnType(uint32_t);
    time_t t;
    time(&t);
    m3ApiReturn((uint32_t)t);
}

m3ApiRawFunction(m3_host_clock_ms) {
    m3ApiReturnType(uint32_t);
    struct timespec ts;
    /* clock_gettime não disponível em todos os hosts — usa clock() como fallback */
    m3ApiReturn((uint32_t)(clock() * 1000 / CLOCKS_PER_SEC));
}

M3Result wasm_libc_link(IM3Runtime runtime, IM3Module module) {
    M3Result r;

#define LINK(mod, name, sig, fn) \
    r = m3_LinkRawFunction(module, mod, name, sig, fn); \
    if (r && r != m3Err_functionLookupFailed) return r;

    LINK("env", "__host_write",     "v(*i)", m3_host_write_stdout);
    LINK("env", "__host_write_err", "v(*i)", m3_host_write_stderr);
    LINK("env", "__host_abort",     "v()",   m3_host_abort);
    LINK("env", "__host_time",      "i()",   m3_host_time);
    LINK("env", "__host_clock_ms",  "i()",   m3_host_clock_ms);
    LINK("env", "__host_readline",  "i(*i)", m3_host_readline);

#undef LINK

    (void)runtime;
    return m3Err_none;
}
