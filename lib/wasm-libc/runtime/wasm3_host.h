#ifndef _WASM3_HOST_H
#define _WASM3_HOST_H

/*
 * wasm3_host.h — interface do binding de host para wasm-libc
 *
 * Inclua este header no seu programa host (não no código WASM).
 * Requer os headers do wasm3 no include path.
 */

#include "wasm3.h"
#include "m3_env.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * wasm_libc_link: registra todos os imports da wasm-libc no módulo.
 *
 * Chame após carregar o módulo e antes de instanciar ou chamar funções.
 *
 * Retorna m3Err_none em sucesso, ou um M3Result de erro.
 * Imports não encontrados no módulo são ignorados silenciosamente
 * (m3Err_functionLookupFailed é tratado como OK).
 */
M3Result wasm_libc_link(IM3Runtime runtime, IM3Module module);

#ifdef __cplusplus
}
#endif

#endif /* _WASM3_HOST_H */
