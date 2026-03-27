#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/papagaio.h"

int main(void)
{
    char *o;
    Papagaio *ctx = papagaio_open();
    if (!ctx) { fprintf(stderr, "Failed to open papagaio\n"); return 1; }
    papagaio_load_plugin(ctx, "plugins/lua/lua.so");

    printf("=== Starting Papagaio C Tests ===\n");

    o = papagaio_process_text(ctx, "((()))", 6);
    printf("Test 1 - '%s'\n", o);
    free(o);

    o = papagaio_process("foo (a) (b) (c) bar",
                         "foo $xs$blockseq{(}{)} bar",
                         "OK [$xs]", NULL);
    printf("Test 2 - %s\n", o);
    free(o);

    o = papagaio_process("hello:world", "$a:", "[$a]:", NULL);
    printf("Test 3 - %s\n", o);
    free(o);

    o = papagaio_process("start end", "start $opt? end", "[$opt]", NULL);
    printf("Test 4 - %s (esperado: [])\n", o);
    free(o);

    o = papagaio_process("a c", "$x? $y? $z?", "x=$x y=$y z=$z", NULL);
    printf("Test 7 - %s (esperado: x=a y=c z=)\n", o);
    free(o);

    o = papagaio_process("42 days", "$num$regex {[0-9]+}", "Number: $num", NULL);
    printf("Test 17 - %s (esperado: Number: 42 days)\n", o);
    free(o);

    /* Test plugins via $import in variadic API (uses lazy ctx) */
    o = papagaio_process("$import{plugins/lua/lua.so}$changequote{@}{<}{>}@lua<return 1+1>", NULL);
    printf("Test 19 - %s (esperado: 2)\n", o);
    free(o);

    o = papagaio_process("$import{plugins/mquickjs/mjs.so}$mqjs{3+3}", NULL);
    printf("Test MQJS - %s (esperado: 6)\n", o);
    free(o);

    o = papagaio_process("$import{plugins/quickjs/qjs.so}$qjs{4+4}", NULL);
    printf("Test QJS - %s (esperado: 8)\n", o);
    free(o);

    /* Test that $lua{} works from local ctx */
    o = papagaio_process_text(ctx, "$lua{return 'X'}", 16);
    printf("Test 18 - %s (esperado: X)\n", o);
    free(o);

    /* Test multi-char optional marker */
    const char *in20 = "$changesymbols{$}{[}{]}{MAYBE} $pattern{[fooMAYBE]}{[MATCH]}\nfoo";
    o = papagaio_process_text(ctx, in20, strlen(in20));
    printf("Test 20 - %s (esperado:  MATCH)\n", o);
    free(o);

    /* Test group with named capture */
    const char *in21 = "$pattern {{$meta$group{ID: $id$int}?}} {ID=$id}\nID: 42";
    o = papagaio_process_text(ctx, in21, strlen(in21));
    printf("Test 21 - %s (esperado: ID=42)\n", o);
    free(o);

    printf("=== All C Tests Finished ===\n");
    papagaio_close(ctx);
    return 0;
}