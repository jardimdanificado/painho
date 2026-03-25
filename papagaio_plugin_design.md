# Sistema de Plugins do Papagaio

## Visão Geral

O modelo é simples: `$import{./lua.so}` carrega uma `.so` que se auto-registra
no contexto do Papagaio, podendo adicionar novos comandos (`$lua{...}`,
`$javascript{...}`, etc), novos modificadores de variável (`$n$mylang`) e
novos handlers de bloco pré-processados.

Lua deixa de ser obrigatória — vira apenas mais um plugin.

---

## Ciclo de vida de um plugin

```
$import{./lua.so}
    │
    ▼
dlopen("./lua.so")
    │
    ▼
papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx)
    │
    ├─ plugin->register_command("lua", lua_handler, state)
    ├─ plugin->register_modifier("luamod", lua_mod_handler, state)
    └─ plugin->register_finalizer(lua_cleanup, state)
```

Cada `.so` expõe **uma única função pública**: `papagaio_plugin_init`.
Tudo mais é interno ao plugin.

---

## A interface C do plugin (`papagaio_plugin.h`)

```c
/* papagaio_plugin.h — interface pública para autores de plugins */
#ifndef PAPAGAIO_PLUGIN_H
#define PAPAGAIO_PLUGIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle passado pelo core ao plugin */
typedef struct Papagaio Papagaio;

/* -----------------------------------------------------------------------
 * PapCommandHandler
 *
 * Chamado quando $nome{conteudo} aparece no texto.
 * Recebe:
 *   ctx     — contexto Papagaio (pode chamar papagaio_process_text de volta)
 *   content — string dentro das chaves (sem delimitadores)
 *   clen    — comprimento de content
 *   userdata— ponteiro registrado junto com o handler
 *
 * Retorna malloc'd string que substitui $nome{conteudo} no output.
 * Retornar NULL equivale a remover o bloco do output.
 * ----------------------------------------------------------------------- */
typedef char *(*PapCommandHandler)(Papagaio   *ctx,
                                   const char *content,
                                   size_t      clen,
                                   void       *userdata);

/* -----------------------------------------------------------------------
 * PapModifierHandler
 *
 * Chamado quando $variavel$nomemod aparece num pattern e captura um match.
 * Recebe a string capturada; retorna malloc'd transformação.
 * ----------------------------------------------------------------------- */
typedef char *(*PapModifierHandler)(const char *match,
                                    size_t      match_len,
                                    const char *modifier_arg, /* conteúdo de {arg} se houver */
                                    size_t      arg_len,
                                    void       *userdata);

/* -----------------------------------------------------------------------
 * PapFinalizer — chamado quando o contexto é fechado
 * ----------------------------------------------------------------------- */
typedef void (*PapFinalizer)(void *userdata);

/* -----------------------------------------------------------------------
 * PapPlugin — estrutura passada para papagaio_plugin_init().
 * O plugin preenche seus campos; o core lê e integra.
 * ----------------------------------------------------------------------- */
typedef struct PapPlugin {
    /* Metadados (opcionais mas recomendados) */
    const char *name;       /* "lua", "javascript", etc */
    const char *version;    /* "1.0.0" */

    /* Registro de comandos: $nome{...} */
    int  (*register_command)(struct PapPlugin *self,
                             const char        *name,
                             PapCommandHandler  handler,
                             void              *userdata);

    /* Registro de modificadores de variável: $x$nome ou $x$nome{arg} */
    int  (*register_modifier)(struct PapPlugin *self,
                              const char         *name,
                              PapModifierHandler  handler,
                              void               *userdata);

    /* Registro de finalizer */
    void (*register_finalizer)(struct PapPlugin *self,
                               PapFinalizer      fn,
                               void             *userdata);

    /* Ponteiro interno do core — não toque */
    void *_core;
} PapPlugin;

/* -----------------------------------------------------------------------
 * Ponto de entrada que TODO plugin DEVE exportar.
 *
 * Retorna 0 em sucesso, != 0 em falha (o $import{} emite erro e continua).
 * ----------------------------------------------------------------------- */
typedef int (*papagaio_plugin_init_fn)(PapPlugin *plugin, Papagaio *ctx);

/* Nome do símbolo exportado pela .so */
#define PAPAGAIO_PLUGIN_INIT "papagaio_plugin_init"

#ifdef __cplusplus
}
#endif
#endif /* PAPAGAIO_PLUGIN_H */
```

---

## O que muda em `papagaio.h` (adições mínimas)

```c
/* Carrega um plugin de um caminho de .so */
int   papagaio_load_plugin(Papagaio *ctx, const char *path);

/* Consulta se um comando está registrado (útil para testes) */
int   papagaio_has_command(Papagaio *ctx, const char *name);
```

---

## O que muda em `papagaio.c`

### 1 — Estrutura interna do contexto

```c
/* Antes */
struct Papagaio { lua_State *L; int owned; };

/* Depois */
#define PAP_MAX_PLUGINS 64

typedef struct {
    const char        *name;
    PapCommandHandler  handler;
    void              *userdata;
} RegisteredCommand;

typedef struct {
    const char         *name;
    PapModifierHandler  handler;
    void               *userdata;
} RegisteredModifier;

typedef struct {
    PapFinalizer fn;
    void        *userdata;
} RegisteredFinalizer;

struct Papagaio {
    lua_State *L;
    int        owned;

    /* plugin registry */
    void              **dl_handles;          /* dlopen handles p/ dlclose */
    int                 dl_count;

    RegisteredCommand  *commands;
    int                 cmd_count, cmd_cap;

    RegisteredModifier *modifiers;
    int                 mod_count, mod_cap;

    RegisteredFinalizer *finalizers;
    int                  fin_count, fin_cap;
};
```

### 2 — `$import{path}` no pré-processador

Em `papagaio_process_text()`, antes da etapa de patterns, adicionar uma
passagem para resolver `$import{...}`:

```c
/* Extrair e executar todos os $import{} antes de qualquer coisa */
static char *resolve_imports(Papagaio *ctx, const char *src,
                              const Symbols *sym)
{
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);
    size_t sl = strlen(sym->sigil);
    const char *keyword = "import";
    size_t kl = 6;
    StrView so = { sym->open,  strlen(sym->open)  };
    StrView sc = { sym->close, strlen(sym->close) };

    while (i < len) {
        if (memcmp(src + i, sym->sigil, sl) == 0 &&
            i + sl + kl <= len &&
            memcmp(src + i + sl, keyword, kl) == 0 &&
            /* não é prefixo de outra palavra */
            !isalnum((unsigned char)src[i + sl + kl]))
        {
            size_t j = i + sl + kl;
            while (j < len && isspace((unsigned char)src[j])) j++;
            if (sv_pfx(src + j, so)) {
                StrView path_sv;
                int next = extract_block(src, (int)j, so, sc, &path_sv);
                StrView path_t = trim_sv(path_sv);

                char path[512];
                size_t plen = path_t.len < 511 ? path_t.len : 511;
                memcpy(path, path_t.ptr, plen); path[plen] = '\0';

                if (papagaio_load_plugin(ctx, path) != 0)
                    fprintf(stderr, "papagaio: $import failed: %s\n", path);

                i = (size_t)next;
                continue;
            }
        }
        sb_append_char(&out, src[i++]);
    }
    return out.data; /* $import{} é removido do stream */
}
```

Inserir no início de `papagaio_process_text()`:

```c
char *after_imports = resolve_imports(ctx, prepared, &sym);
free(prepared);
prepared = after_imports;
```

### 3 — `papagaio_load_plugin()`

```c
#include <dlfcn.h>  /* POSIX; Windows: LoadLibrary */

int papagaio_load_plugin(Papagaio *ctx, const char *path)
{
    if (!ctx || !path) return -1;

    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "papagaio: dlopen(%s): %s\n", path, dlerror());
        return -1;
    }

    papagaio_plugin_init_fn init_fn =
        (papagaio_plugin_init_fn)dlsym(handle, PAPAGAIO_PLUGIN_INIT);
    if (!init_fn) {
        fprintf(stderr, "papagaio: %s has no '%s'\n", path, PAPAGAIO_PLUGIN_INIT);
        dlclose(handle);
        return -1;
    }

    /* Prepara o PapPlugin com as funções de registro */
    PapPlugin plugin;
    memset(&plugin, 0, sizeof(plugin));
    plugin._core            = ctx;
    plugin.register_command  = pap_reg_command;   /* funções estáticas abaixo */
    plugin.register_modifier = pap_reg_modifier;
    plugin.register_finalizer= pap_reg_finalizer;

    int r = init_fn(&plugin, ctx);
    if (r != 0) { dlclose(handle); return r; }

    /* Guarda handle para dlclose() no papagaio_close() */
    ctx->dl_handles = realloc(ctx->dl_handles,
                              sizeof(void*) * (ctx->dl_count + 1));
    ctx->dl_handles[ctx->dl_count++] = handle;
    return 0;
}
```

### 4 — Despacho de comandos em `papagaio_process_text()`

Após a etapa de `$import`, antes dos patterns, passa-se pelo texto e qualquer
`$nome{...}` cujo `nome` esteja no registry de comandos é despachado:

```c
static char *dispatch_commands(Papagaio *ctx, const char *src, const Symbols *sym)
{
    if (!ctx || ctx->cmd_count == 0) {
        /* retorna cópia simples */
        char *r = malloc(strlen(src)+1); strcpy(r, src); return r;
    }
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);
    size_t sl = strlen(sym->sigil);
    StrView so = { sym->open,  strlen(sym->open)  };
    StrView sc = { sym->close, strlen(sym->close) };

    while (i < len) {
        if (memcmp(src + i, sym->sigil, sl) == 0) {
            size_t j = i + sl;
            size_t ks = j;
            while (j < len && (isalnum((unsigned char)src[j]) || src[j]=='_'))
                j++;
            size_t klen = j - ks;

            /* procura no registry */
            int found = -1;
            for (int ci = 0; ci < ctx->cmd_count && found < 0; ci++) {
                if (strlen(ctx->commands[ci].name) == klen &&
                    memcmp(ctx->commands[ci].name, src + ks, klen) == 0)
                    found = ci;
            }

            if (found >= 0) {
                /* pula espaços e tenta ler bloco {…} */
                while (j < len && isspace((unsigned char)src[j])) j++;
                const char *content = ""; size_t clen = 0; int next = (int)j;
                if (j < len && sv_pfx(src + j, so)) {
                    StrView blk;
                    next = extract_block(src, (int)j, so, sc, &blk);
                    content = blk.ptr; clen = blk.len;
                }
                RegisteredCommand *cmd = &ctx->commands[found];
                char *res = cmd->handler(ctx, content, clen, cmd->userdata);
                if (res) { sb_append_n(&out, res, strlen(res)); free(res); }
                i = (size_t)next; continue;
            }
        }
        sb_append_char(&out, src[i++]);
    }
    return out.data;
}
```

Ordem de execução em `papagaio_process_text()`:

```
1. handle_changequotes()
2. pap_prepare()          ← escapa \$
3. resolve_imports()      ← carrega .so, remove $import{}
4. dispatch_commands()    ← $lua{} $javascript{} etc
5. extract_nested() / apply_patterns()  ← $pattern como antes
6. extract_evals() / apply_evals()      ← $eval como antes (se Lua presente)
7. pap_restore()
```

---

## Exemplo: plugin Lua (`lua_plugin.c`)

```c
/* lua_plugin.c — compila com:
 *   cc -shared -fPIC -o lua.so lua_plugin.c -llua5.4
 */
#include "papagaio_plugin.h"
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdlib.h>
#include <string.h>

typedef struct { lua_State *L; } LuaState;

static char *lua_command_handler(Papagaio *ctx, const char *content,
                                  size_t clen, void *ud)
{
    (void)ctx;
    LuaState *s = (LuaState *)ud;
    if (luaL_loadbuffer(s->L, content, clen, "papagaio:lua") != LUA_OK ||
        lua_pcall(s->L, 0, 1, 0) != LUA_OK) {
        const char *err = lua_tostring(s->L, -1);
        char *r = malloc(strlen(err) + 20);
        sprintf(r, "[lua error: %s]", err);
        lua_pop(s->L, 1);
        return r;
    }
    const char *res = lua_tostring(s->L, -1);
    char *out = res ? strdup(res) : strdup("");
    lua_pop(s->L, 1);
    return out;
}

static void lua_finalizer(void *ud) {
    LuaState *s = (LuaState *)ud;
    lua_close(s->L);
    free(s);
}

int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    (void)ctx;
    plugin->name    = "lua";
    plugin->version = "1.0.0";

    LuaState *s = malloc(sizeof(LuaState));
    s->L = luaL_newstate();
    luaL_openlibs(s->L);

    plugin->register_command(plugin,  "lua",       lua_command_handler, s);
    plugin->register_finalizer(plugin, lua_finalizer, s);
    return 0;
}
```

Uso em texto Papagaio:

```
$import{./lua.so}
$lua{return tostring(2 + 2)}
```

Saída: `4`

---

## Exemplo: plugin JavaScript via QuickJS (`js_plugin.c`)

```c
/* js_plugin.c — compila com:
 *   cc -shared -fPIC -o js.so js_plugin.c -lquickjs
 */
#include "papagaio_plugin.h"
#include <quickjs/quickjs.h>
#include <stdlib.h>
#include <string.h>

typedef struct { JSRuntime *rt; JSContext *jctx; } JsState;

static char *js_command_handler(Papagaio *ctx, const char *content,
                                  size_t clen, void *ud)
{
    (void)ctx;
    JsState *s = (JsState *)ud;
    JSValue v = JS_Eval(s->jctx, content, clen, "<papagaio>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(v)) {
        JSValue exc = JS_GetException(s->jctx);
        const char *msg = JS_ToCString(s->jctx, exc);
        char *r = malloc(strlen(msg) + 20);
        sprintf(r, "[js error: %s]", msg);
        JS_FreeCString(s->jctx, msg);
        JS_FreeValue(s->jctx, exc);
        return r;
    }
    const char *str = JS_ToCString(s->jctx, v);
    char *out = str ? strdup(str) : strdup("");
    JS_FreeCString(s->jctx, str);
    JS_FreeValue(s->jctx, v);
    return out;
}

static void js_finalizer(void *ud) {
    JsState *s = (JsState *)ud;
    JS_FreeContext(s->jctx);
    JS_FreeRuntime(s->rt);
    free(s);
}

int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    (void)ctx;
    plugin->name    = "javascript";
    plugin->version = "1.0.0";

    JsState *s = malloc(sizeof(JsState));
    s->rt   = JS_NewRuntime();
    s->jctx = JS_NewContext(s->rt);

    plugin->register_command(plugin,   "javascript", js_command_handler, s);
    plugin->register_command(plugin,   "js",         js_command_handler, s);  /* alias */
    plugin->register_finalizer(plugin,  js_finalizer, s);
    return 0;
}
```

Uso:

```
$import{./js.so}
$js{(2 + 2).toString()}
$javascript{Math.PI.toFixed(4)}
```

---

## Exemplo: plugin Python via Embedded Python (`python_plugin.c`)

```c
#include "papagaio_plugin.h"
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>
#include <string.h>

static char *py_command_handler(Papagaio *ctx, const char *content,
                                  size_t clen, void *ud)
{
    (void)ctx; (void)ud; (void)clen;
    /* Executa code, captura sys.stdout */
    PyObject *io      = PyImport_ImportModule("io");
    PyObject *StringIO= PyObject_GetAttrString(io, "StringIO");
    PyObject *buf     = PyObject_CallObject(StringIO, NULL);
    PySys_SetObject("stdout", buf);

    PyRun_SimpleString(content);

    PyObject *getval  = PyObject_GetAttrString(buf, "getvalue");
    PyObject *result  = PyObject_CallObject(getval, NULL);
    const char *cstr  = PyUnicode_AsUTF8(result);
    char *out = cstr ? strdup(cstr) : strdup("");

    Py_XDECREF(result); Py_XDECREF(getval);
    Py_XDECREF(buf); Py_XDECREF(StringIO); Py_XDECREF(io);
    return out;
}

int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    (void)ctx;
    plugin->name = "python";
    Py_Initialize();
    plugin->register_command(plugin, "python", py_command_handler, NULL);
    plugin->register_command(plugin, "py",     py_command_handler, NULL);
    return 0;
}
```

---

## Exemplo: plugin que adiciona modificador de variável

Um plugin pode registrar `$x$rot13` como modificador:

```c
static char *rot13_modifier(const char *match, size_t mlen,
                              const char *arg, size_t alen, void *ud)
{
    (void)arg; (void)alen; (void)ud;
    char *out = malloc(mlen + 1);
    for (size_t i = 0; i < mlen; i++) {
        char c = match[i];
        if      (c >= 'a' && c <= 'z') out[i] = 'a' + (c - 'a' + 13) % 26;
        else if (c >= 'A' && c <= 'Z') out[i] = 'A' + (c - 'A' + 13) % 26;
        else                           out[i] = c;
    }
    out[mlen] = '\0';
    return out;
}

int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    (void)ctx;
    plugin->name = "rot13";
    plugin->register_modifier(plugin, "rot13", rot13_modifier, NULL);
    return 0;
}
```

Uso:

```
$import{./rot13.so}
$pattern {$w$rot13} {[$w]}
hello
```

Saída: `[uryyb]`

---

## Compatibilidade com WASM (papagaio.js)

Para o build WASM, `dlopen` não existe. A solução é ter uma camada de
**plugin estático** compilado junto ao wasm:

```c
/* papagaio_plugins_static.c — incluído no build wasm */
#include "papagaio_plugin.h"

/* Declarações dos init functions dos plugins compilados estaticamente */
extern int papagaio_plugin_init_lua(PapPlugin *, Papagaio *);
extern int papagaio_plugin_init_js(PapPlugin *, Papagaio *);

static const struct {
    const char *name;
    papagaio_plugin_init_fn init;
} STATIC_PLUGINS[] = {
    { "lua", papagaio_plugin_init_lua },
    { "js",  papagaio_plugin_init_js  },
};

/* Chamado por papagaio_open() no build wasm */
void papagaio_load_static_plugins(Papagaio *ctx) {
    for (size_t i = 0; i < sizeof(STATIC_PLUGINS)/sizeof(*STATIC_PLUGINS); i++)
        papagaio_load_plugin_static(ctx, STATIC_PLUGINS[i].init);
}
```

E `$import{lua}` (sem `.so`, sem `/`) resolve via tabela estática ao invés
de `dlopen`, mantendo a mesma sintaxe para o usuário.

---

## Resumo das mudanças no código

| Arquivo | Mudança |
|---|---|
| `papagaio_plugin.h` | **Novo** — interface pública do plugin |
| `papagaio.h` | +2 funções: `papagaio_load_plugin`, `papagaio_has_command` |
| `papagaio.c` | `struct Papagaio` ganha registry; `process_text` ganha fases `resolve_imports` + `dispatch_commands`; `papagaio_close` faz `dlclose` de todos os handles |
| `papagaio_plugins_static.c` | **Novo** — stub p/ build wasm |
| `lua_plugin.c` | **Novo** — Lua como plugin separado |
| `js_plugin.c` | **Novo** — QuickJS como plugin separado |

O núcleo do Papagaio (parsing de patterns, blocks, modifiers, tokens) **não
muda**. Só `process_text` ganha duas passagens a mais no início do pipeline.
