#include "../../src/papagaio_plugin.h"
#include "../../src/papagaio.h"
#include "mquickjs/mquickjs.h"
#include "mquickjs/mquickjs_priv.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
    JSContext *ctx;
    void *mem_buf;
    PapPlugin plugin; /* Store by value to avoid dangling pointer */
} MjsState;

static char *mjs_command_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud);
static void mjs_finalizer(void *ud);

/* Prototypes for functions used in mjs_stdlib.h that aren't in mquickjs_priv.h */
static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_pap_register(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

/* Standard library and atoms table */
#include "mjs_stdlib.h"

/* print bridge */
static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    for (int i = 0; i < argc; i++) {
        JSCStringBuf buf;
        const char *s = JS_ToCString(ctx, argv[i], &buf);
        fprintf(stderr, "%s%s", s ? s : "undefined", (i < argc - 1) ? "\t" : "");
    }
    fprintf(stderr, "\n");
    return JS_UNDEFINED;
}

/* papagaio.register bridge */
static JSValue js_pap_register(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    MjsState *s = (MjsState *)JS_GetContextOpaque(ctx);
    if (!s) return JS_UNDEFINED;
    
    JSCStringBuf buf;
    const char *name = JS_ToCString(ctx, argv[0], &buf);
    
    /* Store the function in a global object _PAP_COMMANDS */
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue commands = JS_GetPropertyStr(ctx, global, "_PAP_COMMANDS");
    if (JS_IsUndefined(commands)) {
        commands = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, global, "_PAP_COMMANDS", commands);
    }
    JS_SetPropertyStr(ctx, commands, name, argv[1]);
    
    s->plugin.register_command(&s->plugin, name, mjs_command_handler, s);
    return JS_UNDEFINED;
}

static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JS_GC(ctx);
    return JS_UNDEFINED;
}

static int64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}

static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt64(ctx, get_time_ms());
}

static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    return JS_NewInt64(ctx, get_time_ms());
}

static char *mjs_command_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud) {
    MjsState *s = (MjsState *)ud;
    JSContext *jctx = s->ctx;
    
    if (argc == 0 && strcmp(cmd_name, "mqjs") == 0) return strdup("");

    /* Populate 'params' object */
    JSValue params = JS_NewArray(jctx, argc);
    for (int i = 0; i < argc; i++) {
        JS_SetPropertyUint32(jctx, params, i, JS_NewStringLen(jctx, argv[i], argl[i]));
    }
    JSValue global = JS_GetGlobalObject(jctx);
    JS_SetPropertyStr(jctx, global, "params", params);

    JSValue res;
    if (strcmp(cmd_name, "mqjs") == 0) {
        char *code = (char *)malloc(argl[0] + 1);
        memcpy(code, argv[0], argl[0]);
        code[argl[0]] = '\0';
        res = JS_Eval(jctx, code, argl[0], "papagaio:mqjs", JS_EVAL_RETVAL);
        free(code);
    } else {
        JSValue commands = JS_GetPropertyStr(jctx, global, "_PAP_COMMANDS");
        if (JS_IsUndefined(commands)) return strdup("");
        
        JSValue func = JS_GetPropertyStr(jctx, commands, cmd_name);
        if (!JS_IsFunction(jctx, func)) return strdup("");
        
        if (JS_StackCheck(jctx, 4)) {
            return strdup("[mqjs error: stack overflow]");
        }
        
        /* Create 'args' array to pass as single argument */
        JSValue js_args = JS_NewArray(jctx, argc);
        for (int i = 0; i < argc; i++) {
            JS_SetPropertyUint32(jctx, js_args, i, JS_NewStringLen(jctx, argv[i], argl[i]));
        }
        
        JS_PushArg(jctx, js_args);
        JS_PushArg(jctx, func);
        JS_PushArg(jctx, JS_NULL); /* this */
        res = JS_Call(jctx, 1);
    }

    if (JS_IsException(res)) {
        JSValue exc = JS_GetException(jctx);
        JSCStringBuf buf;
        const char *msg = JS_ToCString(jctx, exc, &buf);
        char *r = (char *)malloc(strlen(msg) + 32);
        sprintf(r, "[mqjs error: %s]", msg ? msg : "unknown");
        return r;
    }

    if (JS_IsUndefined(res) || JS_IsNull(res)) return strdup("");

    JSCStringBuf buf;
    const char *str = JS_ToCString(jctx, res, &buf);
    char *out = str ? strdup(str) : strdup("");
    return out;
}

static void mjs_finalizer(void *ud) {
    MjsState *s = (MjsState *)ud;
    JS_FreeContext(s->ctx);
    free(s->mem_buf);
    free(s);
}

__attribute__((visibility("default")))
int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    size_t mem_size = 2 * 1024 * 1024; /* 2MB for JS */
    void *mem_buf = malloc(mem_size);
    if (!mem_buf) return -1;
    
    MjsState *s = (MjsState *)malloc(sizeof(MjsState));
    s->mem_buf = mem_buf;
    s->plugin = *plugin;
    s->ctx = JS_NewContext(mem_buf, mem_size, &mjs_stdlib);
    JS_SetContextOpaque(s->ctx, s);

    s->plugin.name = "mqjs";
    s->plugin.version = "1.0.0";

    /* Populate 'args' array */
    int pargc = 0; char **pargv = NULL;
    s->plugin.get_args(&s->plugin, &pargc, &pargv);
    JSValue args = JS_NewArray(s->ctx, pargc);
    for (int i = 0; i < pargc; i++) {
        JS_SetPropertyUint32(s->ctx, args, i, JS_NewString(s->ctx, pargv[i]));
    }
    JSValue global = JS_GetGlobalObject(s->ctx);
    JS_SetPropertyStr(s->ctx, global, "args", args);

    s->plugin.register_command(&s->plugin, "mqjs", mjs_command_handler, s);
    s->plugin.register_finalizer(&s->plugin, mjs_finalizer, s);
    
    return 0;
}
