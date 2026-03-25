/* papagaio_plugin.h — interface pública para autores de plugins */
#ifndef PAPAGAIO_PLUGIN_H
#define PAPAGAIO_PLUGIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle passado pelo core ao plugin */
typedef struct Papagaio Papagaio;

/* -----------------------------------------------------------------------
 * PapCommandHandler
 *
 * Chamado quando $nome{conteudo} aparece no texto.
 * Recebe:
 *   ctx     — contexto Papagaio (pode chamar papagaio_process_text de volta)
 *   content — string dentro das chaves (sem delimitadores)
 *   clen    — comprimento de content
 *   userdata— ponteiro registrado junto com o handler
 *
 * Retorna malloc'd string que substitui $nome{conteudo} no output.
 * Retornar NULL equivale a remover o bloco do output.
 * ----------------------------------------------------------------------- */
typedef char *(*PapCommandHandler)(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata);

typedef struct {
    char              *name;
    PapCommandHandler  handler;
    void              *userdata;
} RegisteredCommand;

/* -----------------------------------------------------------------------
 * PapModifierHandler
 *
 * Chamado quando $variavel$nomemod aparece num pattern e captura um match.
 * Recebe a string capturada; retorna malloc'd transformação.
 * ----------------------------------------------------------------------- */
typedef char *(*PapModifierHandler)(const char *match,
                                    size_t      match_len,
                                    const char *modifier_arg, /* conteúdo de {arg} se houver */
                                    size_t      arg_len,
                                    void       *userdata);

typedef struct {
    char               *name;
    PapModifierHandler  handler;
    void               *userdata;
} RegisteredModifier;

/* -----------------------------------------------------------------------
 * PapFinalizer — chamado quando o contexto é fechado
 * ----------------------------------------------------------------------- */
typedef void (*PapFinalizer)(void *userdata);

/* -----------------------------------------------------------------------
 * PapPlugin — estrutura passada para papagaio_plugin_init().
 * O plugin preenche seus campos; o core lê e integra.
 * ----------------------------------------------------------------------- */
typedef struct PapPlugin {
    /* Metadados (opcionais mas recomendados) */
    const char *name;       /* "lua", "javascript", etc */
    const char *version;    /* "1.0.0" */

    /* Registro de comandos: $nome{...} */
    int  (*register_command)(struct PapPlugin *self,
                             const char        *name,
                             PapCommandHandler  handler,
                             void              *userdata);

    /* Registro de modificadores de variável: $x$nome ou $x$nome{arg} */
    int  (*register_modifier)(struct PapPlugin *self,
                              const char         *name,
                              PapModifierHandler  handler,
                              void               *userdata);

    /* Registro de finalizer */
    void (*register_finalizer)(struct PapPlugin *self,
                               PapFinalizer      fn,
                               void             *userdata);

    /* Recuperar argumentos do host */
    void (*get_args)(struct PapPlugin *self, int *argc, char ***argv);

    /* Ponteiro interno do core — não toque */
    void *_core;
} PapPlugin;

/* -----------------------------------------------------------------------
 * Ponto de entrada que TODO plugin DEVE exportar.
 *
 * Retorna 0 em sucesso, != 0 em falha (o $import{} emite erro e continua).
 * ----------------------------------------------------------------------- */
typedef int (*papagaio_plugin_init_fn)(PapPlugin *plugin, Papagaio *ctx);

/* Nome do símbolo exportado pela .so */
#define PAPAGAIO_PLUGIN_INIT "papagaio_plugin_init"

#ifdef __cplusplus
}
#endif
#endif /* PAPAGAIO_PLUGIN_H */
