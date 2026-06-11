#include "../../src/papagaio.h"
#include "wasm3/source/wasm3.h"
#include "wasm3/source/m3_env.h"
#include "wasm3/source/m3_function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    IM3Runtime runtime;
    IM3Function function;
} WasmCommandData;

typedef struct WasmContext {
    Papagaio *papagaio_ctx;
    IM3Environment env;
    IM3Runtime runtime;
    WasmCommandData **cmd_data_list;
    int cmd_data_count;
    int cmd_data_cap;
    struct WasmContext *next;
} WasmContext;

static WasmContext *g_wasm_contexts = NULL;
static void cleanup_wasm_context(void *userdata);

static WasmContext *get_or_create_wasm_context(Papagaio *ctx) {
    WasmContext *curr = g_wasm_contexts;
    while (curr) {
        if (curr->papagaio_ctx == ctx) return curr;
        curr = curr->next;
    }
    WasmContext *new_ctx = malloc(sizeof(WasmContext));
    if (!new_ctx) return NULL;
    new_ctx->papagaio_ctx = ctx;
    new_ctx->env = m3_NewEnvironment();
    if (!new_ctx->env) {
        free(new_ctx);
        return NULL;
    }
    new_ctx->runtime = m3_NewRuntime(new_ctx->env, 1024 * 1024, NULL);
    if (!new_ctx->runtime) {
        m3_FreeEnvironment(new_ctx->env);
        free(new_ctx);
        return NULL;
    }
    new_ctx->cmd_data_list = NULL;
    new_ctx->cmd_data_count = 0;
    new_ctx->cmd_data_cap = 0;
    
    new_ctx->next = g_wasm_contexts;
    g_wasm_contexts = new_ctx;
    
    papagaio_add_finalizer(ctx, cleanup_wasm_context, ctx);
    return new_ctx;
}

static void cleanup_wasm_context(void *userdata) {
    Papagaio *ctx = (Papagaio *)userdata;
    WasmContext **prev = &g_wasm_contexts;
    WasmContext *curr = g_wasm_contexts;
    while (curr) {
        if (curr->papagaio_ctx == ctx) {
            *prev = curr->next;
            
            for (int i = 0; i < curr->cmd_data_count; i++) {
                free(curr->cmd_data_list[i]);
            }
            free(curr->cmd_data_list);
            
            if (curr->runtime) m3_FreeRuntime(curr->runtime);
            if (curr->env) m3_FreeEnvironment(curr->env);
            free(curr);
            break;
        }
        prev = &curr->next;
        curr = curr->next;
    }
}

m3ApiRawFunction(host_write) {
    m3ApiGetArgMem(const char *, buf);
    m3ApiGetArg(int32_t, len);
    if (buf) fwrite(buf, 1, (size_t)len, stdout);
    m3ApiSuccess();
}

m3ApiRawFunction(host_write_err) {
    m3ApiGetArgMem(const char *, buf);
    m3ApiGetArg(int32_t, len);
    if (buf) fwrite(buf, 1, (size_t)len, stderr);
    m3ApiSuccess();
}

m3ApiRawFunction(host_abort) {
    m3ApiGetArgMem(const char *, msg);
    fprintf(stderr, "[WASM ABORT] %s\n", msg ? msg : "unknown error");
    abort();
}

static void link_host_functions(WasmContext *wctx, IM3Module module) {
    m3_LinkRawFunction(module, "env", "__host_write",     "v(*i)", host_write);
    m3_LinkRawFunction(module, "env", "__host_write_err", "v(*i)", host_write_err);
    m3_LinkRawFunction(module, "env", "__host_abort",     "v(*)",  host_abort);
}

static char *wasm_command_bridge(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)name;
    WasmCommandData *data = (WasmCommandData *)ud;
    IM3Function f = data->function;
    IM3Runtime runtime = data->runtime;
    if (!f) return strdup("");

    const uint32_t ARGS_BASE = 4096;
    uint32_t mem_size = 0;
    uint8_t *mem = m3_GetMemory(runtime, &mem_size, 0);
    if (!mem) return strdup("");

    uint32_t table_size = (uint32_t)argc * 4;
    uint32_t str_base = ARGS_BASE + table_size;
    uint32_t cur = str_base;

    for (int i = 0; i < argc; i++) {
        size_t slen = argl[i];
        if (cur + slen + 1 > mem_size) return strdup("");
        memcpy(mem + cur, argv[i], slen);
        mem[cur + slen] = '\0';
        uint32_t off = cur;
        if (ARGS_BASE + i * 4 + 4 > mem_size) return strdup("");
        memcpy(mem + ARGS_BASE + i * 4, &off, 4);
        cur += (uint32_t)(slen + 1 + 7) & ~7u;
    }

    M3Result res = m3_CallV(f, (uint32_t)argc, (uint32_t)ARGS_BASE);
    if (res) {
        fprintf(stderr, "[WASM] CallV Error: %s\n", res);
        return strdup("");
    }

    uint32_t wasm_ptr = 0;
    m3_GetResultsV(f, &wasm_ptr);
    if (wasm_ptr == 0) return strdup("");

    mem = m3_GetMemory(runtime, &mem_size, 0);
    if (!mem || wasm_ptr >= mem_size) return strdup("");

    return strdup((const char *)(mem + wasm_ptr));
}

static void papagaio_load_wasm_bytes(WasmContext *wctx, uint8_t *bytes, size_t size) {
    IM3Module module;
    M3Result result = m3_ParseModule(wctx->env, &module, (bytes_t)bytes, (uint32_t)size);
    if (result) {
        fprintf(stderr, "[WASM] Parse Error: %s\n", result);
        return;
    }
    
    result = m3_LoadModule(wctx->runtime, module);
    if (result) {
        fprintf(stderr, "[WASM] Load Error: %s\n", result);
        return;
    }
    
    link_host_functions(wctx, module);
    m3_CompileModule(module);
    
    for (uint32_t i = 0; i < module->numFunctions; i++) {
        M3Function *f_info = &module->functions[i];
        if (f_info->export_name && strncmp(f_info->export_name, "papagaio_", 9) == 0) {
            const char *cmd_name = f_info->export_name + 9;
            
            IM3Function f_ready = NULL;
            M3Result fres = m3_FindFunction(&f_ready, wctx->runtime, f_info->export_name);
            if (fres || !f_ready) {
                fprintf(stderr, "[WASM] Could not find compiled '%s': %s\n", f_info->export_name, fres ? fres : "null");
                continue;
            }
            
            WasmCommandData *cmd_data = malloc(sizeof(WasmCommandData));
            cmd_data->runtime = wctx->runtime;
            cmd_data->function = f_ready;
            
            if (wctx->cmd_data_count >= wctx->cmd_data_cap) {
                wctx->cmd_data_cap = wctx->cmd_data_cap ? wctx->cmd_data_cap * 2 : 4;
                wctx->cmd_data_list = realloc(wctx->cmd_data_list, wctx->cmd_data_cap * sizeof(WasmCommandData*));
            }
            wctx->cmd_data_list[wctx->cmd_data_count++] = cmd_data;
            
            papagaio_register_command(wctx->papagaio_ctx, cmd_name, wasm_command_bridge, cmd_data);
        }
    }
}

static void papagaio_load_wasm_file(WasmContext *wctx, const char *path) {
    char trim_path[256]; size_t pl = strlen(path);
    size_t start = 0; while(start < pl && isspace((unsigned char)path[start])) start++;
    size_t end = pl; while(end > start && isspace((unsigned char)path[end-1])) end--;
    size_t len = end - start; if (len >= 255) len = 255;
    memcpy(trim_path, path + start, len); trim_path[len] = '\0';
    
    FILE *f = fopen(trim_path, "rb");
    if (!f) {
        fprintf(stderr, "[ERROR] Could not open Wasm file: '%s'\n", trim_path);
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *bytes = malloc(size);
    if (bytes) { 
        if (fread(bytes, 1, size, f) == size) {
            papagaio_load_wasm_bytes(wctx, bytes, size);
        } else {
            fprintf(stderr, "[ERROR] Could not read Wasm file: '%s'\n", trim_path);
        }
        free(bytes); 
    }
    fclose(f);
}

static char *wasm_eval_command(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata) {
    (void)name;
    (void)userdata;
    (void)argl;
    if (argc < 1) return strdup("");
    
    WasmContext *wctx = get_or_create_wasm_context(ctx);
    if (!wctx) {
        fprintf(stderr, "[WASM] Failed to initialize WasmContext\n");
        return strdup("");
    }
    
    papagaio_load_wasm_file(wctx, argv[0]);
    return strdup("");
}

int papagaio_plugin_init(Papagaio *ctx) {
    papagaio_register_command(ctx, "wasm", wasm_eval_command, NULL);
    return 0;
}
