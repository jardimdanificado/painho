#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/papagaio.h"

int main(void) {
    Papagaio *ctx = papagaio_open();
    char *o;

    printf("=== Testing $NAME$from syntax ===\n");

    /* Test 1: Basic Assignment */
    const char *in1 = "$versao$from{0.31.2} Versao: $versao";
    o = papagaio_process_text(ctx, in1, strlen(in1));
    /* $versao$from should be removed, and $versao replaced.
       Expected: " Versao: 0.31.2" */
    printf("Test 1 [Basic Assignment] - '%s'\n", o);
    free(o);

    /* Test 2: Assignment with side effects */
    const char *in2 = "$autor$from{$pattern{A}{Jardel} A} Autor: $autor";
    o = papagaio_process_text(ctx, in2, strlen(in2));
    /* Inside $from, A becomes Jardel. So $autor is "Jardel".
       The A->Jardel rule also persists.
       Expected: " Autor: Jardel" */
    printf("Test 2 [Side effects] - '%s'\n", o);
    free(o);

    /* Test 3: Multiple assignments */
    const char *in3 = "$A$from{1} $B$from{2} Sum: $A + $B";
    o = papagaio_process_text(ctx, in3, strlen(in3));
    /* Expected: "   Sum: 1 + 2" */
    printf("Test 3 [Multiple] - '%s'\n", o);
    free(o);

    /* Test 4: Nested and chained */
    const char *in4 = "$X$from{val} $Y$from{$X$X} $X $Y";
    o = papagaio_process_text(ctx, in4, strlen(in4));
    /* X=val, Y=valval. Output: "  val valval" */
    printf("Test 4 [Chained] - '%s'\n", o);
    free(o);

    papagaio_close(ctx);
    return 0;
}
