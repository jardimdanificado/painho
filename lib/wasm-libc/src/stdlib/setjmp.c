/*
 * setjmp.c — longjmp bare-metal para wasm3
 *
 * Ver setjmp.h para explicação completa das limitações.
 *
 * __longjmp_impl apenas marca o env como "jumped" com o valor.
 * O fluxo de controle real depende do padrão de uso do chamador.
 *
 * Para tratamento de erros estilo "panic", o padrão recomendado é:
 *
 *   typedef struct { jmp_buf buf; } Context;
 *
 *   void safe_op(Context *ctx) {
 *       // operação que pode falhar
 *       longjmp(ctx->buf, ERR_CODE);
 *   }
 *
 *   int run(Context *ctx) {
 *       int err = setjmp(ctx->buf);
 *       if (err) return err;
 *       safe_op(ctx);
 *       return 0;
 *   }
 *
 * Nota: em wasm3, o suporte a exceções WASM (proposal/exceptions) permite
 * implementação completa. Compile com -fwasm-exceptions e use a versão
 * Emscripten do setjmp quando disponível.
 */

#include <setjmp.h>

void __longjmp_impl(jmp_buf env, int val) {
    env[0].value  = (val == 0) ? 1 : val;
    env[0].jumped = 1;
    env[0].active = 0;
    /*
     * Em WASM sem suporte a exceções, não podemos desempilar frames.
     * A implementação completa requereria __builtin_wasm_throw ou
     * a instrução `throw` do WASM exceptions proposal.
     *
     * Por ora, o valor está salvo em env para ser lido pelo chamador
     * que usa o padrão de polling.
     */
}
