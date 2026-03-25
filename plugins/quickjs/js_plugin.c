/* plugins/quickjs/js_plugin.c */
#include "../../src/papagaio_plugin.h"
#include <quickjs/quickjs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
        char *r = (char *)malloc(strlen(msg) + 20);
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
    plugin->name    = "javascript";
    plugin->version = "1.0.0";

    JsState *s = (JsState *)malloc(sizeof(JsState));
    s->rt   = JS_NewRuntime();
    s->jctx = JS_NewContext(s->rt);

    plugin->register_command(plugin, "javascript", js_command_handler, s);
    plugin->register_command(plugin, "js",         js_command_handler, s);  /* alias */
    plugin->register_finalizer(plugin,  js_finalizer, s);
    return 0;
}
