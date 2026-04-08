#ifndef _CTYPE_H
#define _CTYPE_H

/*
 * Implementação via lookup table de 256 entradas.
 * Cada entrada é uma bitmask das categorias do caractere.
 * Todas as funções são inline para zero overhead em hot paths.
 */

/* bits de categoria */
#define _CT_UP   0x01   /* maiúscula A-Z         */
#define _CT_LO   0x02   /* minúscula a-z         */
#define _CT_DI   0x04   /* dígito decimal 0-9    */
#define _CT_SP   0x08   /* espaço em branco      */
#define _CT_PU   0x10   /* pontuação/símbolo     */
#define _CT_CN   0x20   /* controle              */
#define _CT_HX   0x40   /* dígito hex a-f A-F    */
#define _CT_BL   0x80   /* espaço (0x20)         */

extern const unsigned char _ctype_table[256];

static inline int isalpha (int c) { return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO); }
static inline int isupper (int c) { return _ctype_table[(unsigned char)c] & _CT_UP; }
static inline int islower (int c) { return _ctype_table[(unsigned char)c] & _CT_LO; }
static inline int isdigit (int c) { return _ctype_table[(unsigned char)c] & _CT_DI; }
static inline int isalnum (int c) { return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO|_CT_DI); }
static inline int isxdigit(int c) { return _ctype_table[(unsigned char)c] & _CT_HX; }
static inline int isspace (int c) { return _ctype_table[(unsigned char)c] & _CT_SP; }
static inline int isblank (int c) { return (c == ' ' || c == '\t'); }
static inline int ispunct (int c) { return _ctype_table[(unsigned char)c] & _CT_PU; }
static inline int iscntrl (int c) { return _ctype_table[(unsigned char)c] & _CT_CN; }
static inline int isprint (int c) {
    return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO|_CT_DI|_CT_PU|_CT_BL);
}
static inline int isgraph (int c) {
    return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO|_CT_DI|_CT_PU);
}

static inline int toupper(int c) {
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}
static inline int tolower(int c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

#endif /* _CTYPE_H */
