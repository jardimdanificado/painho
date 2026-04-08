#ifndef _ERRNO_H
#define _ERRNO_H

/*
 * Variável de erro global.
 * Em bare-metal sem threads usamos uma variável estática simples.
 * Se quiser multi-thread no futuro, substitua por um macro que
 * acessa armazenamento por-thread via um import do host.
 */
extern int errno;

/* ── códigos de erro POSIX essenciais ───────────────────────────────────── */

#define EPERM        1   /* operação não permitida           */
#define ENOENT       2   /* arquivo ou diretório inexistente */
#define ESRCH        3   /* processo inexistente             */
#define EINTR        4   /* chamada de sistema interrompida  */
#define EIO          5   /* erro de I/O                      */
#define EBADF        9   /* descritor de arquivo inválido    */
#define ENOMEM      12   /* sem memória disponível           */
#define EACCES      13   /* permissão negada                 */
#define EFAULT      14   /* endereço inválido                */
#define EBUSY       16   /* dispositivo ou recurso ocupado   */
#define EEXIST      17   /* arquivo já existe                */
#define ENODEV      19   /* dispositivo inexistente          */
#define EINVAL      22   /* argumento inválido               */
#define ENOSPC      28   /* sem espaço no dispositivo        */
#define ERANGE      34   /* resultado fora do intervalo      */
#define EOVERFLOW   75   /* valor muito grande para o tipo   */
#define EILSEQ      84   /* sequência de bytes ilegal        */
#define ENOSYS      38   /* função não implementada          */

#endif /* _ERRNO_H */
