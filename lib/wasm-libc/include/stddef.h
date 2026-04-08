#ifndef _STDDEF_H
#define _STDDEF_H

/* tipos fundamentais para wasm32 */
typedef unsigned int  size_t;
typedef int           ptrdiff_t;

/* intptr_t e uintptr_t ficam em stdint.h para evitar conflito */

#define NULL ((void*)0)

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
