#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/papagaio.h"

int main(void) {
    Papagaio *ctx = papagaio_open();
    char *o;

    printf("=== Testing $document rework ===\n");

    /* Test 1: $document (current) */
    const char *in1 = "Hello $document world";
    /* In resolve_preprocessor, $document becomes "Hello $document world"
       Wait, if it becomes the WHOLE buffer, it will include itself!
       Actually, that's how it worked before. */
    o = papagaio_process_text(ctx, in1, strlen(in1));
    /* Expected: "Hello Hello $document world world" 
       Actually, if it's the current buffer, it should be the whole string. */
    printf("Test 1 [$document] - '%s'\n", o);
    free(o);

    /* Test 2: $document$original */
    /* Let's define a pattern and then use $document$original */
    const char *in2 = "$pattern{A}{OK} A - $document$original";
    o = papagaio_process_text(ctx, in2, strlen(in2));
    /* The original is "$pattern{A}{OK} A - $document$original"
       The current should be " OK - ..." 
       Wait, $document$original should be exactly the input. */
    printf("Test 2 [$document$original] - '%s'\n", o);
    if (strstr(o, "$pattern{A}{OK}")) {
        printf("  SUCCESS: Found original pattern definition in output.\n");
    } else {
        printf("  FAILURE: Original document not found correctly.\n");
    }
    free(o);

    /* Test 3: $document$current */
    const char *in3 = "A $document$current";
    o = papagaio_process_text(ctx, in3, strlen(in3));
    printf("Test 3 [$document$current] - '%s'\n", o);
    free(o);

    papagaio_close(ctx);
    return 0;
}
