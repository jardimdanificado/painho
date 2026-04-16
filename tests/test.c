#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/papagaio.h"

int main(void)
{
    char *o;
    Papagaio *ctx = papagaio_open();
    if (!ctx) { fprintf(stderr, "Failed to open papagaio\n"); return 1; }

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

    o = papagaio_process("foo", "$id$word", "${id}x", NULL);
    printf("Test 8 [Braced] - %s (esperado: foox)\n", o);
    free(o);

    o = papagaio_process("42 days", "$num$regex {[0-9]+}", "Number: $num", NULL);
    printf("Test 17 - %s (esperado: Number: 42 days)\n", o);
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

    printf("\n=== Compiling Bare Metal Wasm Test Plugin ===\n");
    int rc = system("./papagaio tests/test_wasm.c.pap > tests/test_wasm.c && "
                    "clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export-all -o tests/test_plugin.wasm tests/test_wasm.c 2>/dev/null");
    
    if (rc == 0) {
        printf("--- Wasm Plugin Compiled. Testing Execution...\n");
        const char *in22 = "$wasmfile{tests/test_plugin.wasm} $test_mul{7}{6}";
        o = papagaio_process_text(ctx, in22, strlen(in22));
        printf("Test 22 [Wasm] - %s (esperado: 42)\n", o);
        free(o);
        
        const char *in23 = "$test_mul{-5}{4}";
        o = papagaio_process_text(ctx, in23, strlen(in23));
        printf("Test 23 [Wasm Memory Isolation] - %s (esperado: -20)\n", o);
        free(o);

        /* Test 24: Wasm via Base64 command */
        system("base64 -w 0 tests/test_plugin.wasm > tests/test_plugin.b64");
        FILE *f = fopen("tests/test_plugin.b64", "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            char *b64 = malloc(fsize + 1);
            fread(b64, 1, fsize, f);
            b64[fsize] = '\0';
            fclose(f);

            char *in24 = malloc(fsize + 128);
            sprintf(in24, "$wasm{%s} $test_mul{10}{20}", b64);
            o = papagaio_process_text(ctx, in24, strlen(in24));
            printf("Test 24 [Wasm Base64] - %s (esperado: 200)\n", o);
            
            free(o);
            free(b64);
            free(in24);
        }
        
        system("rm -f tests/test_wasm.c tests/test_plugin.wasm tests/test_plugin.b64"); // Cleanup
    } else {
        printf("Test 22 [Wasm] - SKIPPED (clang wasm32 backend not available)\n");
    }

    /* Test CLI Args Expansion */
    printf("\n=== Testing CLI Args Expansion ===\n");
    char *test_argv[] = {"test_bin", "test_script.c", "arg1", "arg2", "foo=bar", "target=wasm"};
    papagaio_set_args(ctx, 6, test_argv);

    const char *in25 = "Script: $args$0, First: $args$1, Count: $args$count, All: $args$all";
    o = papagaio_process_text(ctx, in25, strlen(in25));
    printf("Test 25 [CLI Args Positional] - %s (esperado: Script: test_script.c, First: arg1, Count: 5, All: arg1 arg2 foo=bar target=wasm)\n", o);
    free(o);

    const char *in26 = "Foo: $args$foo, Target: $args$target, Unknown: $args$unknown";
    o = papagaio_process_text(ctx, in26, strlen(in26));
    printf("Test 26 [CLI Args Named] - %s (esperado: Foo: bar, Target: wasm, Unknown: $args$unknown)\n", o);
    free(o);

    const char *in27 = "Direct Foo: $foo, Direct Target: $target, No Direct: $unknown";
    o = papagaio_process_text(ctx, in27, strlen(in27));
    printf("Test 27 [CLI Args Direct] - %s (esperado: Direct Foo: bar, Direct Target: wasm, No Direct: $unknown)\n", o);
    free(o);

    printf("\n=== All C Tests Finished ===\n");
    papagaio_close(ctx);
    return 0;
}