#include <stdio.h>
#include <stdlib.h>
#include "../src/papagaio.h"


/* ============================================================
 * Demo com testes
 * ============================================================ */
int main(void)
{
    char *o;

    o = papagaio_process("((()))", "$inner$block{(}{)}", "", NULL);
    printf("Test 1 - inner='%s'\n", o);
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

    o = papagaio_process("start end", "start $blk$blockseq{(}{)}? end", "[$blk]", NULL);
    printf("Test 5 - %s (esperado: [])\n", o);
    free(o);

    o = papagaio_process("start xyz end", "start $opt? end", "[$opt]", NULL);
    printf("Test 6 - %s (esperado: [xyz])\n", o);
    free(o);

    o = papagaio_process("a c", "$x? $y? $z?", "x=$x y=$y z=$z", NULL);
    printf("Test 7 - %s (esperado: x=a y=c z=)\n", o);
    free(o);

    o = papagaio_process("hello world", "$a $b", "$a=$b", NULL);
    printf("Test 8 - %s (esperado: hello=world)\n", o);
    free(o);

    o = papagaio_process("name:John age:30", "name:$name age:$age", "$name is $age", NULL);
    printf("Test 9 - %s (esperado: John is 30)\n", o);
    free(o);

    o = papagaio_process("prefix (content)", "$p $${(}{)}b", "$p+$b", NULL);
    printf("Test 10 - %s (esperado: prefix+content)\n", o);
    free(o);

    o = papagaio_process("one two three", "$a $b $c", "[$a][$b][$c]", NULL);
    printf("Test 11 - %s (esperado: [one][two][three])\n", o);
    free(o);

    o = papagaio_process("a c.x", " $x.", "[$x]", NULL);
    printf("Test 12 - %s (esperado: a[c]x)\n", o);
    free(o);

    o = papagaio_process("abc.def", "$x.", "[$x]", NULL);
    printf("Test 13 - %s (esperado: [abc]def)\n", o);
    free(o);

    o = papagaio_process("a  b  c", "$x $y $z", "[$x][$y][$z]", NULL);
    printf("Test 14 - %s (esperado: [a][b][c])\n", o);
    free(o);

    o = papagaio_process("hello world", " $x", "[$x]", NULL);
    printf("Test 15 - %s (esperado: hello[world])\n", o);
    free(o);

    o = papagaio_process("abc", " $x", "[$x]", NULL);
    printf("Test 16 - %s (esperado: abc - sem match)\n", o);
    free(o);

    printf("\n=== TESTE DETALHADO ===\n");
    o = papagaio_process("a c.c", " $x.", "x=$x", NULL);
    printf("Input:    \"a c.c\"\n");
    printf("Pattern:  \" $x.\"\n");
    printf("Replace:  \"x=$x\"\n");
    printf("Resultado: \"%s\"\n", o);
    printf("Esperado:  \"ax=cc\"\n");
    free(o);

    printf("\n=== TESTE REGRESSÃO ===\n");
    o = papagaio_process("a c", "$x? $y? $z?", "x=$x y=$y z=$z", NULL);
    printf("Input:    \"a c\"\n");
    printf("Pattern:  \"$x? $y? $z?\"\n");
    printf("Resultado: \"%s\"\n", o);
    printf("Esperado:  \"x=a y=c z=\"\n");
    free(o);

    o = papagaio_process("42 days", "$num$regex {[0-9]+}", "Number: $num", NULL);
    printf("Test 17 - %s (esperado: Number: 42 days)\n", o);
    free(o);

    o = papagaio_process_text(NULL, "$changequote{@}{<}{>}@eval<return 1+1>", 38);
    printf("Test 19 - %s (esperado: 2)\n", o);
    free(o);

    /* Test that $eval{} can call the internal papagaio parser via papagaio.process(). */
    o = papagaio_process("x", "$x", "$eval{return papagaio.process('$pattern {x} {X}\\nx')}", NULL);
    printf("Test 18 - %s (esperado: X)\n", o);
    free(o);

    return 0;
}