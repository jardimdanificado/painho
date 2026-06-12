#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/papagaio.h"

int main(void)
{
    char *o;
    Papagaio *ctx = papagaio_open();
    if (!ctx) { fprintf(stderr, "Failed to open papagaio\n"); return 1; }
    papagaio_set_cli_mode(ctx, 1);

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


    /* Test group with named capture */
    const char *in21 = "$pattern {{$meta$group{ID: $id$int}?}} {ID=$id}\nID: 42";
    o = papagaio_process_text(ctx, in21, strlen(in21));
    printf("Test 21 - %s (esperado: ID=42)\n", o);
    free(o);

    printf("\n=== Compiling Bare Metal Wasm Test Plugin ===\n");
    int rc = system("clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export-all -o tests/test_plugin.wasm tests/test_wasm.c 2>/dev/null");
    
    if (rc == 0) {
        printf("--- Wasm Plugin Compiled. Testing Execution...\n");
        const char *in22 = "$import{examples/wasm/papagaio_wasm.so}$wasm{tests/test_plugin.wasm} $test_mul{7}{6}";
        o = papagaio_process_text(ctx, in22, strlen(in22));
        printf("Test 22 [Wasm] - %s (esperado: 42)\n", o);
        free(o);
        
        const char *in23 = "$test_mul{-5}{4}";
        o = papagaio_process_text(ctx, in23, strlen(in23));
        printf("Test 23 [Wasm Memory Isolation] - %s (esperado: -20)\n", o);
        free(o);

        system("rm -f tests/test_plugin.wasm"); // Cleanup
    } else {
        printf("Test 22 [Wasm] - SKIPPED (clang wasm32 backend not available)\n");
    }

    /* Test $include command */
    printf("\n=== Testing $include command ===\n");
    system("echo 'FILE_CONTENT' > tests/test_file.txt");
    const char *in28 = "$include{tests/test_file.txt}";
    o = papagaio_process_text(ctx, in28, strlen(in28));
    /* Trim newline if any from echo */
    if (o && strlen(o) > 0 && o[strlen(o)-1] == '\n') o[strlen(o)-1] = '\0';
    printf("Test 28 [$include] - %s (esperado: FILE_CONTENT)\n", o);
    free(o);
    system("rm -f tests/test_file.txt");

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

    /* Reset args before further tests */
    papagaio_set_args(ctx, 0, NULL);

    /* Test trailing-sigil whitespace consumption */
    printf("\n=== Testing Trailing Sigil Whitespace Consumption ===\n");

    o = papagaio_process("hello   world",
                         "hello$ world", "MATCH", NULL);
    printf("Test 30 [Trailing sigil literal] - '%s' (esperado: 'MATCH')\n", o);
    free(o);

    o = papagaio_process("foo   bar",
                         "$a$ $b", "$a/$b", NULL);
    printf("Test 31 [Trailing sigil var] - '%s' (esperado: 'foo/bar')\n", o);
    free(o);

    /* Test multiple braced vars in one replacement */
    printf("\n=== Testing Braced Variables ===\n");

    o = papagaio_process("John Doe",
                         "$first $last", "${last}, ${first}", NULL);
    printf("Test 32 [Braced multi-var] - '%s' (esperado: 'Doe, John')\n", o);
    free(o);

    o = papagaio_process("foo",
                         "$x$word", "${x}suffix${unknown}", NULL);
    printf("Test 33 [Braced unknown kept] - '%s' (esperado: 'foosuffix${unknown}')\n", o);
    free(o);

    /* $args$0 tests */
    printf("\n=== Testing $args$0 (script name) ===\n");
    char *argv0[] = {"binary_name", "script.pap", "extra_arg", "key=val"};
    papagaio_set_args(ctx, 4, argv0);
    const char *in_a0 = "Script: $args$0, Extra: $args$1, Count: $args$count";
    o = papagaio_process_text(ctx, in_a0, strlen(in_a0));
    printf("Test 34 [$args$0] - %s (esperado: Script: script.pap, Extra: extra_arg, Count: 3)\n", o);
    free(o);
    const char *in_named = "Named: $args$key, All: $args$all";
    o = papagaio_process_text(ctx, in_named, strlen(in_named));
    printf("Test 35 [$args$ named+all] - %s (esperado: Named: val, All: extra_arg key=val)\n", o);
    free(o);

    /* Unresolved $args$ stays literal */
    const char *in_miss = "Miss: $args$missing";
    o = papagaio_process_text(ctx, in_miss, strlen(in_miss));
    printf("Test 36 [$args$ unresolved literal] - %s (esperado: Miss: $args$missing)\n", o);
    free(o);

    printf("\n=== All C Tests Finished ===\n");
    papagaio_close(ctx);
    return 0;
}