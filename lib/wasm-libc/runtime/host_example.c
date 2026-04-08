/*
 * host_example.c — exemplo de host wasm3 que carrega um .wasm
 *                  compilado com a wasm-libc
 *
 * Compile com:
 *   gcc host_example.c wasm3_host.c \
 *       -I /path/to/wasm3/source \
 *       -L /path/to/wasm3/build -lwasm3 -lm \
 *       -o host
 *
 * Rode com:
 *   ./host ../build/test.wasm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm3.h"
#include "m3_env.h"
#include "wasm3_host.h"

#define STACK_SIZE_BYTES (64 * 1024)

/* lê arquivo inteiro em memória — retorna ponteiro e tamanho via *out_size */
static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }

    fseek(f, 0, SEEK_END);
    size_t size = (size_t)ftell(f);
    rewind(f);

    uint8_t *buf = (uint8_t *)malloc(size);
    if (!buf) { fclose(f); return NULL; }

    if (fread(buf, 1, size, f) != size) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = size;
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "uso: %s <arquivo.wasm> [args...]\n", argv[0]);
        return 1;
    }

    /* ── carrega o .wasm ── */
    size_t   wasm_size;
    uint8_t *wasm_bytes = read_file(argv[1], &wasm_size);
    if (!wasm_bytes) return 1;

    /* ── inicializa wasm3 ── */
    M3Result   result;
    IM3Environment env     = m3_NewEnvironment();
    IM3Runtime     runtime = m3_NewRuntime(env, STACK_SIZE_BYTES, NULL);

    /* memória importada: deixa wasm3 alocar e gerenciar */
    IM3Module module;
    result = m3_ParseModule(env, &module, wasm_bytes, (uint32_t)wasm_size);
    if (result) { fprintf(stderr, "ParseModule: %s\n", result); return 1; }

    result = m3_LoadModule(runtime, module);
    if (result) { fprintf(stderr, "LoadModule: %s\n", result); return 1; }

    /* ── registra imports da wasm-libc ── */
    result = wasm_libc_link(runtime, module);
    if (result) { fprintf(stderr, "wasm_libc_link: %s\n", result); return 1; }

    /* ── busca e chama main() ── */
    IM3Function fn_main;
    result = m3_FindFunction(&fn_main, runtime, "wasm_main");
    if (result) { fprintf(stderr, "FindFunction(main): %s\n", result); return 1; }

    /* passa argc/argv se o módulo aceitar — ignora erro se assinatura diferir */
    result = m3_CallV(fn_main);
    if (result) {
        M3ErrorInfo info;
        m3_GetErrorInfo(runtime, &info);
        fprintf(stderr, "Erro em runtime: %s\n  em %s:%d\n",
                result, info.file ? info.file : "?", info.line);
        return 1;
    }

    /* obtém valor de retorno */
    int32_t ret = 0;
    m3_GetResultsV(fn_main, &ret);
    printf("\n[host] main() retornou: %d\n", ret);

    /* ── cleanup ── */
    free(wasm_bytes);
    /* wasm3 gerencia a memória de module/runtime internamente */

    return (int)ret;
}
