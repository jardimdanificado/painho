#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/papagaio.h"

int main(void)
{
    char *o;

    printf("=== Testing Priority System ===\n");

    /* Test 1: Simple reordering of side effects */
    {
        Papagaio *ctx = papagaio_open();
        const char *in = "$priority$1{Result: A} $priority$0{$pattern{A}{OK}}";
        char *o = papagaio_process_text(ctx, in, strlen(in));
        printf("Test 1 [Basic Priority] - '%s' (esperado: 'Result: OK ')\n", o);
        free(o);
        papagaio_close(ctx);
    }

    /* Test 2: Recursive priorities */
    {
        Papagaio *ctx = papagaio_open();
        const char *in = "$priority$0{A $priority$1{B}} $priority$1{C}";
        char *o = papagaio_process_text(ctx, in, strlen(in));
        printf("Test 2 [Recursive Priority] - '%s' (esperado: 'A B C')\n", o);
        free(o);
        papagaio_close(ctx);
    }

    /* Test 3: Side effects order (Inner vs Outer) */
    {
        Papagaio *ctx = papagaio_open();
        const char *in = "$priority$0{$pattern{X}{1} $priority$1{$pattern{X}{2}}} $priority$1{$pattern{X}{3}} X";
        char *o = papagaio_process_text(ctx, in, strlen(in));
        /* Order: Inner P1 (X=2) then P0 (X=1) then Outer P1 (X=3). First rule wins. Result: 2. */
        printf("Test 3 [Side Effects Order] - '%s' (esperado: '   2')\n", o);
        free(o);
        papagaio_close(ctx);
    }

    /* Test 4: Priority vs No Priority */
    {
        Papagaio *ctx = papagaio_open();
        const char *in = "X $priority$0{$pattern{X}{PRIO}}";
        char *o = papagaio_process_text(ctx, in, strlen(in));
        /* P0 runs first. Then X is processed. Result: PRIO. */
        printf("Test 4 [Priority vs No Priority] - '%s' (esperado: 'PRIO ')\n", o);
        free(o);
        papagaio_close(ctx);
    }

    /* Test 5: Negative priority side effects */
    {
        Papagaio *ctx = papagaio_open();
        const char *in = "$priority$0{Result: A} $priority$-1{$pattern{A}{OK}}";
        char *o = papagaio_process_text(ctx, in, strlen(in));
        /* P-1 runs before P0. Result: OK. */
        printf("Test 5 [Negative Priority] - '%s' (esperado: 'Result: OK ')\n", o);
        free(o);
        papagaio_close(ctx);
    }

    /* Test 6: Priority max alias */
    {
        Papagaio *ctx = papagaio_open();
        const char *in = "$priority$0{Result: A} $priority$max{$pattern{A}{MAX}}";
        char *o = papagaio_process_text(ctx, in, strlen(in));
        /* max runs before 0. Result: MAX. */
        printf("Test 6 [Priority Max] - '%s' (esperado: 'Result: MAX ')\n", o);
        free(o);
        papagaio_close(ctx);
    }
    return 0;
}
