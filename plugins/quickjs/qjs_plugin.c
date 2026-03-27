#include "../../src/papagaio_plugin.h"
#include "../../src/papagaio.h"
#include "quickjs/quickjs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    JSRuntime *rt;
    JSContext *ctx;
    PapPlugin plugin;
} QjsState;

static char *qjs_command_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud);
static void qjs_finalizer(void *ud);

/* print bridge */
static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    for (int i = 0; i < argc; i++) {
        const char *s = JS_ToCString(ctx, argv[i]);
        fprintf(stderr, "%s%s", s ? s : "undefined", (i < argc - 1) ? "\t" : "");
        JS_FreeCString(ctx, s);
    }
    fprintf(stderr, "\n");
    return JS_UNDEFINED;
}

/* papagaio.register bridge */
static JSValue js_pap_register(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    QjsState *s = (QjsState *)JS_GetContextOpaque(ctx);
    if (!s) return JS_UNDEFINED;
    
    const char *name = JS_ToCString(ctx, argv[0]);
    
    /* Store the function in a global object _PAP_COMMANDS */
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue commands = JS_GetPropertyStr(ctx, global, "_PAP_COMMANDS");
    if (JS_IsUndefined(commands) || JS_IsException(commands)) {
        commands = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, global, "_PAP_COMMANDS", JS_DupValue(ctx, commands));
    }
    JS_SetPropertyStr(ctx, commands, name, JS_DupValue(ctx, argv[1]));
    JS_FreeValue(ctx, commands);
    JS_FreeValue(ctx, global);
    
    s->plugin.register_command(&s->plugin, name, qjs_command_handler, s);
    JS_FreeCString(ctx, name);
    return JS_UNDEFINED;
}

static char *qjs_command_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud) {
    QjsState *s = (QjsState *)ud;
    JSContext *jctx = s->ctx;
    
    if (argc == 0 && strcmp(cmd_name, "qjs") == 0) return strdup("");

    /* Populate 'params' object */
    JSValue params = JS_NewArray(jctx);
    for (int i = 0; i < argc; i++) {
        JS_SetPropertyUint32(jctx, params, i, JS_NewStringLen(jctx, argv[i], argl[i]));
    }
    JSValue global = JS_GetGlobalObject(jctx);
    JS_SetPropertyStr(jctx, global, "params", params);

    JSValue res;
    if (strcmp(cmd_name, "qjs") == 0) {
        char *code = (char *)malloc(argl[0] + 1);
        memcpy(code, argv[0], argl[0]);
        code[argl[0]] = '\0';
        res = JS_Eval(jctx, code, argl[0], "papagaio:qjs", JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_STRICT);
        free(code);
    } else {
        JSValue commands = JS_GetPropertyStr(jctx, global, "_PAP_COMMANDS");
        if (JS_IsUndefined(commands) || JS_IsException(commands)) {
            JS_FreeValue(jctx, global);
            return strdup("");
        }
        
        JSValue func = JS_GetPropertyStr(jctx, commands, cmd_name);
        JS_FreeValue(jctx, commands);
        if (!JS_IsFunction(jctx, func)) {
            JS_FreeValue(jctx, func);
            JS_FreeValue(jctx, global);
            return strdup("");
        }
        
        /* Create 'args' array to pass as single argument */
        JSValue js_args = JS_NewArray(jctx);
        for (int i = 0; i < argc; i++) {
            JS_SetPropertyUint32(jctx, js_args, i, JS_NewStringLen(jctx, argv[i], argl[i]));
        }
        
        res = JS_Call(jctx, func, JS_UNDEFINED, 1, (JSValueConst *)&js_args);
        JS_FreeValue(jctx, js_args);
        JS_FreeValue(jctx, func);
    }
    JS_FreeValue(jctx, global);

    if (JS_IsException(res)) {
        JSValue exc = JS_GetException(jctx);
        const char *msg = JS_ToCString(jctx, exc);
        char *r = (char *)malloc(strlen(msg) + 32);
        sprintf(r, "[qjs error: %s]", msg ? msg : "unknown");
        JS_FreeCString(jctx, msg);
        JS_FreeValue(jctx, exc);
        return r;
    }

    if (JS_IsUndefined(res) || JS_IsNull(res)) {
        JS_FreeValue(jctx, res);
        return strdup("");
    }

    const char *str = JS_ToCString(jctx, res);
    char *out = str ? strdup(str) : strdup("");
    JS_FreeCString(jctx, str);
    JS_FreeValue(jctx, res);
    return out;
}

static void qjs_finalizer(void *ud) {
    QjsState *s = (QjsState *)ud;
    JS_FreeContext(s->ctx);
    JS_FreeRuntime(s->rt);
    free(s);
}

__attribute__((visibility("default")))
int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    QjsState *s = (QjsState *)malloc(sizeof(QjsState));
    s->rt = JS_NewRuntime();
    s->ctx = JS_NewContext(s->rt);
    s->plugin = *plugin;
    JS_SetContextOpaque(s->ctx, s);

    s->plugin.name = "qjs";
    s->plugin.version = "1.0.0";

    /* Global objects */
    JSValue global = JS_GetGlobalObject(s->ctx);
    
    /* Bridge functions */
    JS_SetPropertyStr(s->ctx, global, "print", JS_NewCFunction(s->ctx, js_print, "print", 1));
    
    JSValue papagaio = JS_NewObject(s->ctx);
    JS_SetPropertyStr(s->ctx, papagaio, "register", JS_NewCFunction(s->ctx, js_pap_register, "register", 2));
    JS_SetPropertyStr(s->ctx, global, "papagaio", papagaio);

    /* Populate 'args' array */
    int pargc = 0; char **pargv = NULL;
    s->plugin.get_args(&s->plugin, &pargc, &pargv);
    JSValue args = JS_NewArray(s->ctx);
    for (int i = 0; i < pargc; i++) {
        JS_SetPropertyUint32(s->ctx, args, i, JS_NewString(s->ctx, pargv[i]));
    }
    JS_SetPropertyStr(s->ctx, global, "args", args);
    
    JS_FreeValue(s->ctx, global);

    s->plugin.register_command(&s->plugin, "qjs", qjs_command_handler, s);
    s->plugin.register_finalizer(&s->plugin, qjs_finalizer, s);
    
    return 0;
}
