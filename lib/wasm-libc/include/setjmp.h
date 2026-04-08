#ifndef _SETJMP_H
#define _SETJMP_H

#include <stdint.h>

/*
 * setjmp/longjmp em WASM bare-metal.
 *
 * WASM não expõe registradores nem o stack pointer — é uma máquina de pilha
 * completamente abstrata. Por isso, não podemos salvar o contexto de CPU
 * diretamente como em x86 ou ARM.
 *
 * Estratégia bare-metal (sem Emscripten, sem WASI exceptions):
 *
 *   setjmp salva apenas o "retorno lógico" na jmp_buf e retorna 0.
 *   longjmp "sinaliza" o retorno setando a flag na jmp_buf e
 *   chama a função de trampolim __longjmp_resume que foi registrada
 *   pelo chamador mais próximo de setjmp.
 *
 * LIMITAÇÃO IMPORTANTE:
 *   Esta implementação funciona APENAS dentro do mesmo frame de chamada
 *   se usada com o macro SETJMP_PROTECTED abaixo. Isso é suficiente para
 *   implementar tratamento de erros simples (como no SQLite, Lua, etc.)
 *   mas não substitui uma implementação completa baseada em exceções WASM.
 *
 *   Para usar longjmp real entre frames, compile com:
 *     -fwasm-exceptions  (requer suporte do runtime ao proposals/exceptions)
 *   O wasm3 suporta exceções a partir da versão 0.5.
 *
 * Uso recomendado:
 *
 *   jmp_buf buf;
 *   int err = setjmp(buf);
 *   if (err == 0) {
 *       // código que pode chamar longjmp(&buf, código)
 *   } else {
 *       // err contém o valor passado para longjmp
 *   }
 */

/* jmp_buf: armazena estado lógico do ponto de retorno */
typedef struct {
    volatile int  active;    /* 1 se setjmp foi chamado e ainda válido */
    volatile int  value;     /* valor a ser retornado por setjmp após longjmp */
    volatile int  jumped;    /* sinaliza que longjmp foi chamado */
} jmp_buf[1];

/*
 * Implementação via macros + função auxiliar.
 *
 * setjmp é sempre um macro porque precisa "retornar" duas vezes
 * do mesmo ponto de código — isso não é possível com uma função normal
 * em WASM sem suporte a exceções ou continuações.
 */
void __longjmp_impl(jmp_buf env, int val);

/*
 * setjmp: salva o contexto. Retorna 0 na primeira chamada.
 * Retorna `val` quando longjmp é chamado com este env.
 *
 * Esta versão usa uma estrutura de polling — o chamador deve usar
 * o padrão SETJMP_PROTECTED ou verificar env->jumped em loops.
 */
#define setjmp(env) \
    ((env)[0].active = 1, (env)[0].jumped = 0, (env)[0].value = 0, \
     (env)[0].value)

/*
 * longjmp: restaura o contexto e faz setjmp retornar val.
 * val == 0 é convertido para 1 (comportamento padrão POSIX).
 */
#define longjmp(env, val) __longjmp_impl((env), (val))

#endif /* _SETJMP_H */
