/*
 * strerror.c — converte código errno em string descritiva
 *
 * Adicionado em string/ porque complementa as funções de string,
 * mas depende de errno.h.
 */

#include <string.h>
#include <errno.h>

/*
 * strerror: retorna ponteiro para string estática.
 * Não é reentrante (padrão POSIX para strerror sem _r).
 */
const char *strerror(int errnum) {
    switch (errnum) {
        case 0:        return "Success";
        case EPERM:    return "Operation not permitted";
        case ENOENT:   return "No such file or directory";
        case ESRCH:    return "No such process";
        case EINTR:    return "Interrupted system call";
        case EIO:      return "Input/output error";
        case EBADF:    return "Bad file descriptor";
        case ENOMEM:   return "Cannot allocate memory";
        case EACCES:   return "Permission denied";
        case EFAULT:   return "Bad address";
        case EBUSY:    return "Device or resource busy";
        case EEXIST:   return "File exists";
        case ENODEV:   return "No such device";
        case EINVAL:   return "Invalid argument";
        case ENOSPC:   return "No space left on device";
        case ERANGE:   return "Numerical result out of range";
        case EOVERFLOW:return "Value too large for defined data type";
        case EILSEQ:   return "Invalid or incomplete multibyte character";
        case ENOSYS:   return "Function not implemented";
        default:       return "Unknown error";
    }
}

/*
 * perror: imprime "prefix: mensagem\n" em stderr.
 */
void perror(const char *s) {
    extern int errno;
    extern void __host_write_err(const char *buf, int len);
    extern size_t strlen(const char *);

    if (s && *s) {
        __host_write_err(s, (int)strlen(s));
        __host_write_err(": ", 2);
    }
    const char *msg = strerror(errno);
    __host_write_err(msg, (int)strlen(msg));
    __host_write_err("\n", 1);
}
