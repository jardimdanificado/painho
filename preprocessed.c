# 0 "src/papagaio.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3
# 0 "<command-line>" 2
# 1 "src/papagaio.c"

# 1 "src/papagaio.h" 1



# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 160 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3

# 160 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3
typedef long int ptrdiff_t;
# 229 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3
typedef long unsigned int size_t;
# 344 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3
typedef int wchar_t;
# 440 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
# 451 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3
} max_align_t;
# 465 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 3
  typedef __typeof__(nullptr) nullptr_t;
# 5 "src/papagaio.h" 2






# 10 "src/papagaio.h"
typedef struct Papagaio Papagaio;
# 34 "src/papagaio.h"
Papagaio *papagaio_open(void);
void papagaio_close(Papagaio *ctx);
void papagaio_set_args(Papagaio *ctx, int argc, char **argv);
void papagaio_get_args(Papagaio *ctx, int *argc, char ***argv);


typedef char *(*PapCommandHandler)(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata);
typedef char *(*PapModifierHandler)(const char *match, const char *modifier, size_t match_len, size_t mod_len, void *userdata);
typedef void (*PapFinalizer)(void *userdata);

int papagaio_has_command(Papagaio *ctx, const char *name);
int papagaio_register_command(Papagaio *ctx, const char *name, PapCommandHandler handler, void *ud);
int papagaio_register_modifier(Papagaio *ctx, const char *name, PapModifierHandler handler, void *ud);


char *papagaio_process(const char *input, ...);
char *papagaio_process_ex(const char *input,
                          const char *sigil,
                          const char *open,
                          const char *close, ...);
char *papagaio_process_pairs(Papagaio *ctx,
                             const char *input,
                             const char **patterns,
                             const char **repls,
                             int pair_count);
char *papagaio_process_text(Papagaio *ctx,
                            const char *input,
                            size_t len);
# 3 "src/papagaio.c" 2
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdbool.h" 1 3
# 4 "src/papagaio.c" 2
# 1 "/usr/include/ctype.h" 1 3
# 25 "/usr/include/ctype.h" 3
# 1 "/usr/include/features.h" 1 3
# 431 "/usr/include/features.h" 3
# 1 "/usr/include/features-time64.h" 1 3
# 20 "/usr/include/features-time64.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 21 "/usr/include/features-time64.h" 2 3
# 1 "/usr/include/bits/timesize.h" 1 3
# 19 "/usr/include/bits/timesize.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 20 "/usr/include/bits/timesize.h" 2 3
# 22 "/usr/include/features-time64.h" 2 3
# 432 "/usr/include/features.h" 2 3
# 540 "/usr/include/features.h" 3
# 1 "/usr/include/sys/cdefs.h" 1 3
# 730 "/usr/include/sys/cdefs.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 731 "/usr/include/sys/cdefs.h" 2 3
# 1 "/usr/include/bits/long-double.h" 1 3
# 732 "/usr/include/sys/cdefs.h" 2 3
# 541 "/usr/include/features.h" 2 3
# 564 "/usr/include/features.h" 3
# 1 "/usr/include/gnu/stubs.h" 1 3
# 10 "/usr/include/gnu/stubs.h" 3
# 1 "/usr/include/gnu/stubs-64.h" 1 3
# 11 "/usr/include/gnu/stubs.h" 2 3
# 565 "/usr/include/features.h" 2 3
# 26 "/usr/include/ctype.h" 2 3
# 1 "/usr/include/bits/types.h" 1 3
# 27 "/usr/include/bits/types.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 28 "/usr/include/bits/types.h" 2 3
# 1 "/usr/include/bits/timesize.h" 1 3
# 19 "/usr/include/bits/timesize.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 20 "/usr/include/bits/timesize.h" 2 3
# 29 "/usr/include/bits/types.h" 2 3



# 31 "/usr/include/bits/types.h" 3
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;






typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;



typedef long int __quad_t;
typedef unsigned long int __u_quad_t;







typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
# 141 "/usr/include/bits/types.h" 3
# 1 "/usr/include/bits/typesizes.h" 1 3
# 142 "/usr/include/bits/types.h" 2 3
# 1 "/usr/include/bits/time64.h" 1 3
# 143 "/usr/include/bits/types.h" 2 3


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;




typedef int __sig_atomic_t;
# 27 "/usr/include/ctype.h" 2 3


# 39 "/usr/include/ctype.h" 3
# 1 "/usr/include/bits/endian.h" 1 3
# 35 "/usr/include/bits/endian.h" 3
# 1 "/usr/include/bits/endianness.h" 1 3
# 36 "/usr/include/bits/endian.h" 2 3
# 40 "/usr/include/ctype.h" 2 3






enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
# 79 "/usr/include/ctype.h" 3
extern const unsigned short int **__ctype_b_loc (void)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern const __int32_t **__ctype_tolower_loc (void)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern const __int32_t **__ctype_toupper_loc (void)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 108 "/usr/include/ctype.h" 3
extern int isalnum (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isalpha (int) __attribute__ ((__nothrow__ , __leaf__));
extern int iscntrl (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isdigit (int) __attribute__ ((__nothrow__ , __leaf__));
extern int islower (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isgraph (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isprint (int) __attribute__ ((__nothrow__ , __leaf__));
extern int ispunct (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isspace (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isupper (int) __attribute__ ((__nothrow__ , __leaf__));
extern int isxdigit (int) __attribute__ ((__nothrow__ , __leaf__));



extern int tolower (int __c) __attribute__ ((__nothrow__ , __leaf__));


extern int toupper (int __c) __attribute__ ((__nothrow__ , __leaf__));




extern int isblank (int) __attribute__ ((__nothrow__ , __leaf__));
# 142 "/usr/include/ctype.h" 3
extern int isascii (int __c) __attribute__ ((__nothrow__ , __leaf__));



extern int toascii (int __c) __attribute__ ((__nothrow__ , __leaf__));



extern int _toupper (int) __attribute__ ((__nothrow__ , __leaf__));
extern int _tolower (int) __attribute__ ((__nothrow__ , __leaf__));
# 237 "/usr/include/ctype.h" 3
# 1 "/usr/include/bits/types/locale_t.h" 1 3
# 22 "/usr/include/bits/types/locale_t.h" 3
# 1 "/usr/include/bits/types/__locale_t.h" 1 3
# 27 "/usr/include/bits/types/__locale_t.h" 3
struct __locale_struct
{

  struct __locale_data *__locales[13];


  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;


  const char *__names[13];
};

typedef struct __locale_struct *__locale_t;
# 23 "/usr/include/bits/types/locale_t.h" 2 3

typedef __locale_t locale_t;
# 238 "/usr/include/ctype.h" 2 3
# 251 "/usr/include/ctype.h" 3
extern int isalnum_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isalpha_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int iscntrl_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isdigit_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int islower_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isgraph_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isprint_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int ispunct_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isspace_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isupper_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));
extern int isxdigit_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));

extern int isblank_l (int, locale_t) __attribute__ ((__nothrow__ , __leaf__));



extern int __tolower_l (int __c, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));
extern int tolower_l (int __c, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));


extern int __toupper_l (int __c, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));
extern int toupper_l (int __c, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));
# 327 "/usr/include/ctype.h" 3

# 5 "src/papagaio.c" 2
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdarg.h" 1 3
# 40 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdarg.h" 3
typedef __builtin_va_list __gnuc_va_list;
# 104 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdarg.h" 3
typedef __gnuc_va_list va_list;
# 6 "src/papagaio.c" 2
# 1 "/usr/include/stdio.h" 1 3
# 28 "/usr/include/stdio.h" 3
# 1 "/usr/include/bits/libc-header-start.h" 1 3
# 29 "/usr/include/stdio.h" 2 3









# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 39 "/usr/include/stdio.h" 2 3





# 1 "/usr/include/bits/types/__fpos_t.h" 1 3




# 1 "/usr/include/bits/types/__mbstate_t.h" 1 3
# 13 "/usr/include/bits/types/__mbstate_t.h" 3
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 6 "/usr/include/bits/types/__fpos_t.h" 2 3




typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
# 45 "/usr/include/stdio.h" 2 3
# 1 "/usr/include/bits/types/__fpos64_t.h" 1 3
# 10 "/usr/include/bits/types/__fpos64_t.h" 3
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
# 46 "/usr/include/stdio.h" 2 3
# 1 "/usr/include/bits/types/__FILE.h" 1 3



struct _IO_FILE;
typedef struct _IO_FILE __FILE;
# 47 "/usr/include/stdio.h" 2 3
# 1 "/usr/include/bits/types/FILE.h" 1 3



struct _IO_FILE;


typedef struct _IO_FILE FILE;
# 48 "/usr/include/stdio.h" 2 3
# 1 "/usr/include/bits/types/struct_FILE.h" 1 3
# 35 "/usr/include/bits/types/struct_FILE.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 36 "/usr/include/bits/types/struct_FILE.h" 2 3

struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;




typedef void _IO_lock_t;





struct _IO_FILE
{
  int _flags;


  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;


  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2:24;

  char _short_backupbuf[1];
  __off_t _old_offset;


  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;







  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  struct _IO_FILE **_prevchain;
  int _mode;

  int _unused3;

  __uint64_t _total_written;




  char _unused2[12 * sizeof (int) - 5 * sizeof (void *)];
};
# 49 "/usr/include/stdio.h" 2 3


# 1 "/usr/include/bits/types/cookie_io_functions_t.h" 1 3
# 27 "/usr/include/bits/types/cookie_io_functions_t.h" 3
typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf,
                                          size_t __nbytes);







typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);







typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);


typedef int cookie_close_function_t (void *__cookie);






typedef struct _IO_cookie_io_functions_t
{
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
# 52 "/usr/include/stdio.h" 2 3
# 68 "/usr/include/stdio.h" 3
typedef __off_t off_t;
# 82 "/usr/include/stdio.h" 3
typedef __ssize_t ssize_t;






typedef __fpos_t fpos_t;
# 133 "/usr/include/stdio.h" 3
# 1 "/usr/include/bits/stdio_lim.h" 1 3
# 134 "/usr/include/stdio.h" 2 3
# 153 "/usr/include/stdio.h" 3
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;






extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));

extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ , __leaf__));



extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) __attribute__ ((__nothrow__ , __leaf__));
# 191 "/usr/include/stdio.h" 3
extern int fclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 201 "/usr/include/stdio.h" 3
extern FILE *tmpfile (void)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
# 218 "/usr/include/stdio.h" 3
extern char *tmpnam (char[20]) __attribute__ ((__nothrow__ , __leaf__)) ;




extern char *tmpnam_r (char __s[20]) __attribute__ ((__nothrow__ , __leaf__)) ;
# 235 "/usr/include/stdio.h" 3
extern char *tempnam (const char *__dir, const char *__pfx)
   __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (__builtin_free, 1)));






extern int fflush (FILE *__stream);
# 252 "/usr/include/stdio.h" 3
extern int fflush_unlocked (FILE *__stream);
# 271 "/usr/include/stdio.h" 3
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) __attribute__ ((__nonnull__ (3)));
# 306 "/usr/include/stdio.h" 3
extern FILE *fdopen (int __fd, const char *__modes) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;





extern FILE *fopencookie (void *__restrict __magic_cookie,
     const char *__restrict __modes,
     cookie_io_functions_t __io_funcs) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
# 341 "/usr/include/stdio.h" 3
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__nonnull__ (1)));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));




extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));







extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nonnull__ (1)));




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));



extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));





extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
        __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 0))) ;
extern int __asprintf (char **__restrict __ptr,
         const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) ;
extern int asprintf (char **__restrict __ptr,
       const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) ;




extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));







extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));




extern int scanf (const char *__restrict __format, ...) ;

extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__));





# 1 "/usr/include/bits/floatn.h" 1 3
# 131 "/usr/include/bits/floatn.h" 3
# 1 "/usr/include/bits/floatn-common.h" 1 3
# 24 "/usr/include/bits/floatn-common.h" 3
# 1 "/usr/include/bits/long-double.h" 1 3
# 25 "/usr/include/bits/floatn-common.h" 2 3
# 132 "/usr/include/bits/floatn.h" 2 3
# 445 "/usr/include/stdio.h" 2 3




extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc23_fscanf")

                                __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc23_scanf")
                              ;
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc23_sscanf") __attribute__ ((__nothrow__ , __leaf__))

                      ;
# 497 "/usr/include/stdio.h" 3
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__nonnull__ (1)));





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;


extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__format__ (__scanf__, 2, 0)));






extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vfscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vscanf")

     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vsscanf") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__ ((__format__ (__scanf__, 2, 0)));
# 582 "/usr/include/stdio.h" 3
extern int fgetc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getc (FILE *__stream) __attribute__ ((__nonnull__ (1)));





extern int getchar (void);






extern int getc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getchar_unlocked (void);
# 607 "/usr/include/stdio.h" 3
extern int fgetc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));







extern int fputc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));





extern int putchar (int __c);
# 631 "/usr/include/stdio.h" 3
extern int fputc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));







extern int putc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream) __attribute__ ((__nonnull__ (1)));


extern int putw (int __w, FILE *__stream) __attribute__ ((__nonnull__ (2)));







extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__access__ (__write_only__, 1, 2))) __attribute__ ((__nonnull__ (3)));
# 693 "/usr/include/stdio.h" 3
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) __attribute__ ((__nonnull__ (4)));
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) __attribute__ ((__nonnull__ (4)));


extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) __attribute__ ((__nonnull__ (3)));







extern int fputs (const char *__restrict __s, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (2)));





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) __attribute__ ((__nonnull__ (4)));
# 760 "/usr/include/stdio.h" 3
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));







extern int fseek (FILE *__stream, long int __off, int __whence)
  __attribute__ ((__nonnull__ (1)));




extern long int ftell (FILE *__stream) __attribute__ ((__nonnull__ (1)));




extern void rewind (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 797 "/usr/include/stdio.h" 3
extern int fseeko (FILE *__stream, __off_t __off, int __whence)
  __attribute__ ((__nonnull__ (1)));




extern __off_t ftello (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 823 "/usr/include/stdio.h" 3
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos)
  __attribute__ ((__nonnull__ (1)));




extern int fsetpos (FILE *__stream, const fpos_t *__pos) __attribute__ ((__nonnull__ (1)));
# 854 "/usr/include/stdio.h" 3
extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));







extern void perror (const char *__s) __attribute__ ((__cold__));




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 891 "/usr/include/stdio.h" 3
extern int pclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));





extern FILE *popen (const char *__command, const char *__modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (pclose, 1))) ;






extern char *ctermid (char *__s) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__access__ (__write_only__, 1)));
# 935 "/usr/include/stdio.h" 3
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 953 "/usr/include/stdio.h" 3
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);
# 977 "/usr/include/stdio.h" 3

# 7 "src/papagaio.c" 2
# 1 "/usr/include/stdlib.h" 1 3
# 26 "/usr/include/stdlib.h" 3
# 1 "/usr/include/bits/libc-header-start.h" 1 3
# 27 "/usr/include/stdlib.h" 2 3





# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 33 "/usr/include/stdlib.h" 2 3


# 44 "/usr/include/stdlib.h" 3
# 1 "/usr/include/bits/waitflags.h" 1 3
# 45 "/usr/include/stdlib.h" 2 3
# 1 "/usr/include/bits/waitstatus.h" 1 3
# 46 "/usr/include/stdlib.h" 2 3
# 63 "/usr/include/stdlib.h" 3
typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;





__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
# 102 "/usr/include/stdlib.h" 3
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__ , __leaf__)) ;



extern double atof (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern int atoi (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern long int atol (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;



__extension__ extern long long int atoll (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;



extern double strtod (const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern float strtof (const char *__restrict __nptr,
       char **__restrict __endptr) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern long double strtold (const char *__restrict __nptr,
       char **__restrict __endptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 181 "/usr/include/stdlib.h" 3
extern long int strtol (const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern unsigned long int strtoul (const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



__extension__
extern long long int strtoq (const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int strtoll (const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));






extern long int strtol (const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__ ("" "__isoc23_strtol") __attribute__ ((__nothrow__ , __leaf__))


     __attribute__ ((__nonnull__ (1)));
extern unsigned long int strtoul (const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__ ("" "__isoc23_strtoul") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__ ((__nonnull__ (1)));

__extension__
extern long long int strtoq (const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__ ("" "__isoc23_strtoll") __attribute__ ((__nothrow__ , __leaf__))


     __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__ ("" "__isoc23_strtoull") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__ ((__nonnull__ (1)));

__extension__
extern long long int strtoll (const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__ ("" "__isoc23_strtoll") __attribute__ ((__nothrow__ , __leaf__))


     __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr, char **__restrict __endptr, int __base) __asm__ ("" "__isoc23_strtoull") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__ ((__nonnull__ (1)));
# 282 "/usr/include/stdlib.h" 3
extern int strfromd (char *__dest, size_t __size, const char *__format,
       double __f)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3)));

extern int strfromf (char *__dest, size_t __size, const char *__format,
       float __f)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3)));

extern int strfroml (char *__dest, size_t __size, const char *__format,
       long double __f)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3)));
# 509 "/usr/include/stdlib.h" 3
extern char *l64a (long int __n) __attribute__ ((__nothrow__ , __leaf__)) ;


extern long int a64l (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;




# 1 "/usr/include/sys/types.h" 1 3
# 27 "/usr/include/sys/types.h" 3






typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;


typedef __loff_t loff_t;




typedef __ino_t ino_t;
# 59 "/usr/include/sys/types.h" 3
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;
# 97 "/usr/include/sys/types.h" 3
typedef __pid_t pid_t;





typedef __id_t id_t;
# 114 "/usr/include/sys/types.h" 3
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;




# 1 "/usr/include/bits/types/clock_t.h" 1 3






typedef __clock_t clock_t;
# 127 "/usr/include/sys/types.h" 2 3

# 1 "/usr/include/bits/types/clockid_t.h" 1 3






typedef __clockid_t clockid_t;
# 129 "/usr/include/sys/types.h" 2 3
# 1 "/usr/include/bits/types/time_t.h" 1 3
# 10 "/usr/include/bits/types/time_t.h" 3
typedef __time_t time_t;
# 130 "/usr/include/sys/types.h" 2 3
# 1 "/usr/include/bits/types/timer_t.h" 1 3






typedef __timer_t timer_t;
# 131 "/usr/include/sys/types.h" 2 3
# 144 "/usr/include/sys/types.h" 3
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 145 "/usr/include/sys/types.h" 2 3



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;




# 1 "/usr/include/bits/stdint-intn.h" 1 3
# 24 "/usr/include/bits/stdint-intn.h" 3
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
# 156 "/usr/include/sys/types.h" 2 3


typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;


typedef int register_t __attribute__ ((__mode__ (__word__)));
# 176 "/usr/include/sys/types.h" 3
# 1 "/usr/include/endian.h" 1 3
# 35 "/usr/include/endian.h" 3
# 1 "/usr/include/bits/byteswap.h" 1 3
# 33 "/usr/include/bits/byteswap.h" 3
static __inline __uint16_t
__bswap_16 (__uint16_t __bsx)
{

  return __builtin_bswap16 (__bsx);



}






static __inline __uint32_t
__bswap_32 (__uint32_t __bsx)
{

  return __builtin_bswap32 (__bsx);



}
# 69 "/usr/include/bits/byteswap.h" 3
__extension__ static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{

  return __builtin_bswap64 (__bsx);



}
# 36 "/usr/include/endian.h" 2 3
# 1 "/usr/include/bits/uintn-identity.h" 1 3
# 32 "/usr/include/bits/uintn-identity.h" 3
static __inline __uint16_t
__uint16_identity (__uint16_t __x)
{
  return __x;
}

static __inline __uint32_t
__uint32_identity (__uint32_t __x)
{
  return __x;
}

static __inline __uint64_t
__uint64_identity (__uint64_t __x)
{
  return __x;
}
# 37 "/usr/include/endian.h" 2 3
# 177 "/usr/include/sys/types.h" 2 3


# 1 "/usr/include/sys/select.h" 1 3
# 30 "/usr/include/sys/select.h" 3
# 1 "/usr/include/bits/select.h" 1 3
# 31 "/usr/include/sys/select.h" 2 3


# 1 "/usr/include/bits/types/sigset_t.h" 1 3



# 1 "/usr/include/bits/types/__sigset_t.h" 1 3




typedef struct
{
  unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;
# 5 "/usr/include/bits/types/sigset_t.h" 2 3


typedef __sigset_t sigset_t;
# 34 "/usr/include/sys/select.h" 2 3



# 1 "/usr/include/bits/types/struct_timeval.h" 1 3







struct timeval
{




  __time_t tv_sec;
  __suseconds_t tv_usec;

};
# 38 "/usr/include/sys/select.h" 2 3

# 1 "/usr/include/bits/types/struct_timespec.h" 1 3
# 11 "/usr/include/bits/types/struct_timespec.h" 3
struct timespec
{



  __time_t tv_sec;




  __syscall_slong_t tv_nsec;
# 31 "/usr/include/bits/types/struct_timespec.h" 3
};
# 40 "/usr/include/sys/select.h" 2 3



typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 59 "/usr/include/sys/select.h" 3
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 91 "/usr/include/sys/select.h" 3

# 102 "/usr/include/sys/select.h" 3
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 127 "/usr/include/sys/select.h" 3
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
# 153 "/usr/include/sys/select.h" 3

# 180 "/usr/include/sys/types.h" 2 3





typedef __blksize_t blksize_t;






typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 227 "/usr/include/sys/types.h" 3
# 1 "/usr/include/bits/pthreadtypes.h" 1 3
# 23 "/usr/include/bits/pthreadtypes.h" 3
# 1 "/usr/include/bits/thread-shared-types.h" 1 3
# 44 "/usr/include/bits/thread-shared-types.h" 3
# 1 "/usr/include/bits/pthreadtypes-arch.h" 1 3
# 21 "/usr/include/bits/pthreadtypes-arch.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 22 "/usr/include/bits/pthreadtypes-arch.h" 2 3
# 45 "/usr/include/bits/thread-shared-types.h" 2 3

# 1 "/usr/include/bits/atomic_wide_counter.h" 1 3
# 25 "/usr/include/bits/atomic_wide_counter.h" 3
typedef union
{
  __extension__ unsigned long long int __value64;
  struct
  {
    unsigned int __low;
    unsigned int __high;
  } __value32;
} __atomic_wide_counter;
# 47 "/usr/include/bits/thread-shared-types.h" 2 3




typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
# 76 "/usr/include/bits/thread-shared-types.h" 3
# 1 "/usr/include/bits/struct_mutex.h" 1 3
# 22 "/usr/include/bits/struct_mutex.h" 3
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;

  unsigned int __nusers;



  int __kind;

  short __spins;
  short __unused;
  __pthread_list_t __list;
# 52 "/usr/include/bits/struct_mutex.h" 3
};
# 77 "/usr/include/bits/thread-shared-types.h" 2 3
# 89 "/usr/include/bits/thread-shared-types.h" 3
# 1 "/usr/include/bits/struct_rwlock.h" 1 3
# 23 "/usr/include/bits/struct_rwlock.h" 3
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;

  int __cur_writer;
  int __shared;
  unsigned long int __pad1;
  unsigned long int __pad2;


  unsigned int __flags;
# 48 "/usr/include/bits/struct_rwlock.h" 3
};
# 90 "/usr/include/bits/thread-shared-types.h" 2 3




struct __pthread_cond_s
{
  __atomic_wide_counter __wseq;
  __atomic_wide_counter __g1_start;
  unsigned int __g_size[2] ;
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
  unsigned int __unused_initialized_1;
  unsigned int __unused_initialized_2;
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct
{
  int __data ;
} __once_flag;
# 24 "/usr/include/bits/pthreadtypes.h" 2 3



typedef unsigned long int pthread_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;


union pthread_attr_t
{
  char __size[56];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;




typedef union
{
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;


typedef union
{
  struct __pthread_cond_s __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;





typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 228 "/usr/include/sys/types.h" 2 3



# 519 "/usr/include/stdlib.h" 2 3






extern long int random (void) __attribute__ ((__nothrow__ , __leaf__));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__ , __leaf__));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));





extern int rand (void) __attribute__ ((__nothrow__ , __leaf__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__ , __leaf__));



extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__ , __leaf__));







extern double drand48 (void) __attribute__ ((__nothrow__ , __leaf__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__ , __leaf__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__ , __leaf__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__ , __leaf__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    __extension__ unsigned long long int __a;

  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern __uint32_t arc4random (void)
     __attribute__ ((__nothrow__ , __leaf__)) ;


extern void arc4random_buf (void *__buf, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern __uint32_t arc4random_uniform (__uint32_t __upper_bound)
     __attribute__ ((__nothrow__ , __leaf__)) ;




extern void *malloc (size_t __size) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) ;

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (1, 2))) ;






extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__alloc_size__ (2)));


extern void free (void *__ptr) __attribute__ ((__nothrow__ , __leaf__));
# 702 "/usr/include/stdlib.h" 3
extern void free_sized (void *__ptr, size_t __size) __attribute__ ((__nothrow__ , __leaf__));




extern void free_aligned_sized (void *__ptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__));
# 717 "/usr/include/stdlib.h" 3
extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__))
     __attribute__ ((__alloc_size__ (2, 3)))
    __attribute__ ((__malloc__ (__builtin_free, 1)));


extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__ (reallocarray, 1)));



# 1 "/usr/include/alloca.h" 1 3
# 24 "/usr/include/alloca.h" 3
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 25 "/usr/include/alloca.h" 2 3







extern void *alloca (size_t __size) __attribute__ ((__nothrow__ , __leaf__));






# 729 "/usr/include/stdlib.h" 2 3





extern void *valloc (size_t __size) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) ;




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) ;




extern void *aligned_alloc (size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__alloc_align__ (1)))
     __attribute__ ((__alloc_size__ (2))) ;



extern void abort (void) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));







extern int at_quick_exit (void (*__func) (void)) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));






extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));





extern void exit (int __status) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));





extern void quick_exit (int __status) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));





extern void _Exit (int __status) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));




extern char *getenv (const char *__name) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) ;
# 808 "/usr/include/stdlib.h" 3
extern int putenv (char *__string) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (const char *__name, const char *__value, int __replace)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (const char *__name) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) __attribute__ ((__nothrow__ , __leaf__));
# 836 "/usr/include/stdlib.h" 3
extern char *mktemp (char *__template) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 849 "/usr/include/stdlib.h" 3
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
# 871 "/usr/include/stdlib.h" 3
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) ;
# 892 "/usr/include/stdlib.h" 3
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) ;
# 945 "/usr/include/stdlib.h" 3
extern int system (const char *__command) ;
# 962 "/usr/include/stdlib.h" 3
extern char *realpath (const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__ , __leaf__)) ;






typedef int (*__compar_fn_t) (const void *, const void *);
# 982 "/usr/include/stdlib.h" 3
extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;
# 998 "/usr/include/stdlib.h" 3
extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));
# 1008 "/usr/include/stdlib.h" 3
extern int abs (int __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) ;


__extension__ extern long long int llabs (long long int __x)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) ;
# 1026 "/usr/include/stdlib.h" 3
extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) ;


__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) ;
# 1046 "/usr/include/stdlib.h" 3
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3))) ;




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3))) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));





extern int mblen (const char *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__));


extern int mbtowc (wchar_t *__restrict __pwc,
     const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__ , __leaf__));


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__ , __leaf__));



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__read_only__, 2)));

extern size_t wcstombs (char *__restrict __s,
   const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__access__ (__write_only__, 1, 3)))
  __attribute__ ((__access__ (__read_only__, 2)));






extern int rpmatch (const char *__response) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) ;
# 1133 "/usr/include/stdlib.h" 3
extern int getsubopt (char **__restrict __optionp,
        char *const *__restrict __tokens,
        char **__restrict __valuep)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2, 3))) ;
# 1179 "/usr/include/stdlib.h" 3
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 1191 "/usr/include/stdlib.h" 3
# 1 "/usr/include/bits/types/once_flag.h" 1 3
# 24 "/usr/include/bits/types/once_flag.h" 3
typedef __once_flag once_flag;
# 1192 "/usr/include/stdlib.h" 2 3



extern void call_once (once_flag *__flag, void (*__func)(void));



extern size_t memalignment (const void *__p);


# 1 "/usr/include/bits/stdlib-float.h" 1 3
# 1203 "/usr/include/stdlib.h" 2 3
# 1214 "/usr/include/stdlib.h" 3

# 8 "src/papagaio.c" 2
# 1 "/usr/include/string.h" 1 3
# 26 "/usr/include/string.h" 3
# 1 "/usr/include/bits/libc-header-start.h" 1 3
# 27 "/usr/include/string.h" 2 3


# 37 "/usr/include/string.h" 3
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 38 "/usr/include/string.h" 2 3
# 47 "/usr/include/string.h" 3
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
       size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, const void *__src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));





extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
        int __c, size_t __n)
    __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 4)));




extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));




extern void *memset_explicit (void *__s, int __c, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__access__ (__write_only__, 1, 3)));



extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 91 "/usr/include/string.h" 3
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 118 "/usr/include/string.h" 3
extern void *memchr (const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 156 "/usr/include/string.h" 3
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
    __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 1, 3)));






extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));


extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
    locale_t __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 4)))
     __attribute__ ((__access__ (__write_only__, 1, 3)));





extern char *strdup (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (const char *__string, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 261 "/usr/include/string.h" 3
extern char *strchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 292 "/usr/include/string.h" 3
extern char *strrchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 309 "/usr/include/string.h" 3
extern char *strchrnul (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern size_t strcspn (const char *__s, const char *__reject)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 346 "/usr/include/string.h" 3
extern char *strpbrk (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 377 "/usr/include/string.h" 3
extern char *strstr (const char *__haystack, const char *__needle)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 388 "/usr/include/string.h" 3
extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



extern char *__strtok_r (char *__restrict __s,
    const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
         char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
# 412 "/usr/include/string.h" 3
extern char *strcasestr (const char *__haystack, const char *__needle)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));







extern void *memmem (const void *__haystack, size_t __haystacklen,
       const void *__needle, size_t __needlelen)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 3)))
    __attribute__ ((__access__ (__read_only__, 1, 2)))
    __attribute__ ((__access__ (__read_only__, 3, 4)));



extern void *__mempcpy (void *__restrict __dest,
   const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *mempcpy (void *__restrict __dest,
        const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlen (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern size_t strnlen (const char *__string, size_t __maxlen)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern char *strerror (int __errnum) __attribute__ ((__nothrow__ , __leaf__));
# 464 "/usr/include/string.h" 3
extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__ , __leaf__))

                        __attribute__ ((__nonnull__ (2)))
    __attribute__ ((__access__ (__write_only__, 2, 3)));
# 490 "/usr/include/string.h" 3
extern char *strerror_l (int __errnum, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));



# 1 "/usr/include/strings.h" 1 3
# 23 "/usr/include/strings.h" 3
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stddef.h" 1 3
# 24 "/usr/include/strings.h" 2 3










extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bcopy (const void *__src, void *__dest, size_t __n)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 68 "/usr/include/strings.h" 3
extern char *index (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 96 "/usr/include/strings.h" 3
extern char *rindex (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));






extern int ffs (int __i) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));





extern int ffsl (long int __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
__extension__ extern int ffsll (long long int __ll)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern int strcasecmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));






extern int strcasecmp_l (const char *__s1, const char *__s2, locale_t __loc)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));



extern int strncasecmp_l (const char *__s1, const char *__s2,
     size_t __n, locale_t __loc)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 4)));



# 495 "/usr/include/string.h" 2 3



extern void explicit_bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)))
    __attribute__ ((__access__ (__write_only__, 1, 2)));



extern char *strsep (char **__restrict __stringp,
       const char *__restrict __delim)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) __attribute__ ((__nothrow__ , __leaf__));
# 521 "/usr/include/string.h" 3
extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
   const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlcpy (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 3)));



extern size_t strlcat (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__read_write__, 1, 3)));
# 584 "/usr/include/string.h" 3

# 9 "src/papagaio.c" 2
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdint.h" 1 3
# 9 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdint.h" 3
 
# 9 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdint.h" 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
# 1 "/usr/include/stdint.h" 1 3
# 26 "/usr/include/stdint.h" 3
# 1 "/usr/include/bits/libc-header-start.h" 1 3
# 27 "/usr/include/stdint.h" 2 3

# 1 "/usr/include/bits/wchar.h" 1 3
# 29 "/usr/include/stdint.h" 2 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 30 "/usr/include/stdint.h" 2 3
# 41 "/usr/include/stdint.h" 3
# 1 "/usr/include/bits/stdint-uintn.h" 1 3
# 24 "/usr/include/bits/stdint-uintn.h" 3
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
# 42 "/usr/include/stdint.h" 2 3



# 1 "/usr/include/bits/stdint-least.h" 1 3
# 25 "/usr/include/bits/stdint-least.h" 3
typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;


typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;
# 46 "/usr/include/stdint.h" 2 3





typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
# 64 "/usr/include/stdint.h" 3
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
# 80 "/usr/include/stdint.h" 3
typedef long int intptr_t;


typedef unsigned long int uintptr_t;
# 94 "/usr/include/stdint.h" 3
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;
# 12 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/stdint.h" 2 3
#pragma GCC diagnostic pop
# 10 "src/papagaio.c" 2
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/limits.h" 1 3
# 34 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/limits.h" 3
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/syslimits.h" 1 3






#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/limits.h" 1 3
# 210 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/limits.h" 3
# 1 "/usr/include/limits.h" 1 3
# 26 "/usr/include/limits.h" 3
# 1 "/usr/include/bits/libc-header-start.h" 1 3
# 27 "/usr/include/limits.h" 2 3
# 198 "/usr/include/limits.h" 3
# 1 "/usr/include/bits/posix1_lim.h" 1 3
# 27 "/usr/include/bits/posix1_lim.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 28 "/usr/include/bits/posix1_lim.h" 2 3
# 161 "/usr/include/bits/posix1_lim.h" 3
# 1 "/usr/include/bits/local_lim.h" 1 3
# 38 "/usr/include/bits/local_lim.h" 3
# 1 "/usr/include/linux/limits.h" 1 3
# 39 "/usr/include/bits/local_lim.h" 2 3
# 81 "/usr/include/bits/local_lim.h" 3
# 1 "/usr/include/bits/pthread_stack_min-dynamic.h" 1 3
# 29 "/usr/include/bits/pthread_stack_min-dynamic.h" 3
# 1 "/usr/include/bits/pthread_stack_min.h" 1 3
# 30 "/usr/include/bits/pthread_stack_min-dynamic.h" 2 3
# 82 "/usr/include/bits/local_lim.h" 2 3
# 162 "/usr/include/bits/posix1_lim.h" 2 3
# 199 "/usr/include/limits.h" 2 3



# 1 "/usr/include/bits/posix2_lim.h" 1 3
# 203 "/usr/include/limits.h" 2 3
# 211 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/limits.h" 2 3
# 10 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/syslimits.h" 2 3
#pragma GCC diagnostic pop
# 35 "/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/include/limits.h" 2 3
# 11 "src/papagaio.c" 2
# 1 "src/louro/louro.h" 1
# 37 "src/louro/louro.h"
# 1 "/usr/include/math.h" 1 3
# 27 "/usr/include/math.h" 3
# 1 "/usr/include/bits/libc-header-start.h" 1 3
# 28 "/usr/include/math.h" 2 3









# 1 "/usr/include/bits/math-vector.h" 1 3
# 25 "/usr/include/bits/math-vector.h" 3
# 1 "/usr/include/bits/libm-simd-decl-stubs.h" 1 3
# 26 "/usr/include/bits/math-vector.h" 2 3
# 38 "/usr/include/math.h" 2 3
# 157 "/usr/include/math.h" 3
# 1 "/usr/include/bits/flt-eval-method.h" 1 3
# 158 "/usr/include/math.h" 2 3
# 170 "/usr/include/math.h" 3
typedef float float_t;
typedef double double_t;
# 376 "/usr/include/math.h" 3
# 1 "/usr/include/bits/fp-logb.h" 1 3
# 377 "/usr/include/math.h" 2 3
# 419 "/usr/include/math.h" 3
# 1 "/usr/include/bits/fp-fast.h" 1 3
# 420 "/usr/include/math.h" 2 3



enum
  {
    FP_INT_UPWARD =

      0,
    FP_INT_DOWNWARD =

      1,
    FP_INT_TOWARDZERO =

      2,
    FP_INT_TONEARESTFROMZERO =

      3,
    FP_INT_TONEAREST =

      4,
  };


# 1 "/usr/include/bits/mathcalls-macros.h" 1 3
# 444 "/usr/include/math.h" 2 3





# 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3
# 20 "/usr/include/bits/mathcalls-helper-functions.h" 3
extern int __fpclassify (double __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));


extern int __signbit (double __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));



extern int __isinf (double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __finite (double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __isnan (double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __iseqsig (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));


extern int __issignaling (double __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));
# 450 "/usr/include/math.h" 2 3
# 1 "/usr/include/bits/mathcalls.h" 1 3
# 53 "/usr/include/bits/mathcalls.h" 3
 extern double acos (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __acos (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double asin (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __asin (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double atan (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atan (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double atan2 (double __y, double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atan2 (double __y, double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double cos (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __cos (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double sin (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __sin (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double tan (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __tan (double __x) __attribute__ ((__nothrow__ , __leaf__));



extern double acospi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __acospi (double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern double acospi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __acospi (double __x) __attribute__ ((__nothrow__ , __leaf__));

extern double asinpi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __asinpi (double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern double asinpi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __asinpi (double __x) __attribute__ ((__nothrow__ , __leaf__));

extern double atanpi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atanpi (double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern double atanpi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atanpi (double __x) __attribute__ ((__nothrow__ , __leaf__));

extern double atan2pi (double __y, double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atan2pi (double __y, double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern double atan2pi (double __y, double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atan2pi (double __y, double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double cospi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __cospi (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double sinpi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __sinpi (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double tanpi (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __tanpi (double __x) __attribute__ ((__nothrow__ , __leaf__));





 extern double cosh (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __cosh (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double sinh (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __sinh (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double tanh (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __tanh (double __x) __attribute__ ((__nothrow__ , __leaf__));
# 107 "/usr/include/bits/mathcalls.h" 3
 extern double acosh (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __acosh (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double asinh (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __asinh (double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern double atanh (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __atanh (double __x) __attribute__ ((__nothrow__ , __leaf__));





 extern double exp (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __exp (double __x) __attribute__ ((__nothrow__ , __leaf__));


extern double frexp (double __x, int *__exponent) __attribute__ ((__nothrow__ , __leaf__)); extern double __frexp (double __x, int *__exponent) __attribute__ ((__nothrow__ , __leaf__));


extern double ldexp (double __x, int __exponent) __attribute__ ((__nothrow__ , __leaf__)); extern double __ldexp (double __x, int __exponent) __attribute__ ((__nothrow__ , __leaf__));


 extern double log (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __log (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double log10 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __log10 (double __x) __attribute__ ((__nothrow__ , __leaf__));


extern double modf (double __x, double *__iptr) __attribute__ ((__nothrow__ , __leaf__)); extern double __modf (double __x, double *__iptr) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



 extern double exp10 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __exp10 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double exp2m1 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __exp2m1 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double exp10m1 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __exp10m1 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double log2p1 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __log2p1 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double log10p1 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __log10p1 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double logp1 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __logp1 (double __x) __attribute__ ((__nothrow__ , __leaf__));




 extern double expm1 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __expm1 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double log1p (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __log1p (double __x) __attribute__ ((__nothrow__ , __leaf__));


extern double logb (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __logb (double __x) __attribute__ ((__nothrow__ , __leaf__));




 extern double exp2 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __exp2 (double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern double log2 (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __log2 (double __x) __attribute__ ((__nothrow__ , __leaf__));






 extern double pow (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __pow (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));


extern double sqrt (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __sqrt (double __x) __attribute__ ((__nothrow__ , __leaf__));



 extern double hypot (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __hypot (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));




 extern double cbrt (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __cbrt (double __x) __attribute__ ((__nothrow__ , __leaf__));




extern double compoundn (double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __compoundn (double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


extern double pown (double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __pown (double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


extern double powr (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __powr (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));


extern double rootn (double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __rootn (double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


 extern double rsqrt (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __rsqrt (double __x) __attribute__ ((__nothrow__ , __leaf__));






extern double ceil (double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fabs (double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double floor (double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fmod (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __fmod (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));
# 231 "/usr/include/bits/mathcalls.h" 3
extern int isinf (double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));




extern int finite (double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern double drem (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __drem (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));



extern double significand (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __significand (double __x) __attribute__ ((__nothrow__ , __leaf__));






extern double copysign (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern double nan (const char *__tagb) __attribute__ ((__nothrow__ , __leaf__)); extern double __nan (const char *__tagb) __attribute__ ((__nothrow__ , __leaf__));
# 267 "/usr/include/bits/mathcalls.h" 3
extern int isnan (double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));





extern double j0 (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __j0 (double) __attribute__ ((__nothrow__ , __leaf__));
extern double j1 (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __j1 (double) __attribute__ ((__nothrow__ , __leaf__));
extern double jn (int, double) __attribute__ ((__nothrow__ , __leaf__)); extern double __jn (int, double) __attribute__ ((__nothrow__ , __leaf__));
extern double y0 (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __y0 (double) __attribute__ ((__nothrow__ , __leaf__));
extern double y1 (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __y1 (double) __attribute__ ((__nothrow__ , __leaf__));
extern double yn (int, double) __attribute__ ((__nothrow__ , __leaf__)); extern double __yn (int, double) __attribute__ ((__nothrow__ , __leaf__));





 extern double erf (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __erf (double) __attribute__ ((__nothrow__ , __leaf__));
 extern double erfc (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __erfc (double) __attribute__ ((__nothrow__ , __leaf__));
extern double lgamma (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __lgamma (double) __attribute__ ((__nothrow__ , __leaf__));




extern double tgamma (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __tgamma (double) __attribute__ ((__nothrow__ , __leaf__));





extern double gamma (double) __attribute__ ((__nothrow__ , __leaf__)); extern double __gamma (double) __attribute__ ((__nothrow__ , __leaf__));







extern double lgamma_r (double, int *__signgamp) __attribute__ ((__nothrow__ , __leaf__)); extern double __lgamma_r (double, int *__signgamp) __attribute__ ((__nothrow__ , __leaf__));






extern double rint (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __rint (double __x) __attribute__ ((__nothrow__ , __leaf__));


extern double nextafter (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __nextafter (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));

extern double nexttoward (double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __nexttoward (double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));




extern double nextdown (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __nextdown (double __x) __attribute__ ((__nothrow__ , __leaf__));

extern double nextup (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __nextup (double __x) __attribute__ ((__nothrow__ , __leaf__));



extern double remainder (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __remainder (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));



extern double scalbn (double __x, int __n) __attribute__ ((__nothrow__ , __leaf__)); extern double __scalbn (double __x, int __n) __attribute__ ((__nothrow__ , __leaf__));



extern int ilogb (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern int __ilogb (double __x) __attribute__ ((__nothrow__ , __leaf__));




extern long int llogb (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __llogb (double __x) __attribute__ ((__nothrow__ , __leaf__));




extern double scalbln (double __x, long int __n) __attribute__ ((__nothrow__ , __leaf__)); extern double __scalbln (double __x, long int __n) __attribute__ ((__nothrow__ , __leaf__));



extern double nearbyint (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern double __nearbyint (double __x) __attribute__ ((__nothrow__ , __leaf__));



extern double round (double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern double trunc (double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern double remquo (double __x, double __y, int *__quo) __attribute__ ((__nothrow__ , __leaf__)); extern double __remquo (double __x, double __y, int *__quo) __attribute__ ((__nothrow__ , __leaf__));






extern long int lrint (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __lrint (double __x) __attribute__ ((__nothrow__ , __leaf__));
__extension__
extern long long int llrint (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long long int __llrint (double __x) __attribute__ ((__nothrow__ , __leaf__));



extern long int lround (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __lround (double __x) __attribute__ ((__nothrow__ , __leaf__));
__extension__
extern long long int llround (double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long long int __llround (double __x) __attribute__ ((__nothrow__ , __leaf__));



extern double fdim (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)); extern double __fdim (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));



extern double fmax (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fmin (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern double fma (double __x, double __y, double __z) __attribute__ ((__nothrow__ , __leaf__)); extern double __fma (double __x, double __y, double __z) __attribute__ ((__nothrow__ , __leaf__));




extern double roundeven (double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern double fromfp (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern double __fromfp (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));



extern double ufromfp (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern double __ufromfp (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));




extern double fromfpx (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern double __fromfpx (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));




extern double ufromfpx (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern double __ufromfpx (double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));


extern int canonicalize (double *__cx, const double *__x) __attribute__ ((__nothrow__ , __leaf__));
# 435 "/usr/include/bits/mathcalls.h" 3
extern double fmaximum (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fminimum (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fmaximum_num (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fminimum_num (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fmaximum_mag (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fminimum_mag (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fmaximum_mag_num (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern double fminimum_mag_num (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 485 "/usr/include/bits/mathcalls.h" 3
extern double scalb (double __x, double __n) __attribute__ ((__nothrow__ , __leaf__)); extern double __scalb (double __x, double __n) __attribute__ ((__nothrow__ , __leaf__));
# 451 "/usr/include/math.h" 2 3
# 466 "/usr/include/math.h" 3
# 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3
# 20 "/usr/include/bits/mathcalls-helper-functions.h" 3
extern int __fpclassifyf (float __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));


extern int __signbitf (float __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));



extern int __isinff (float __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __finitef (float __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __isnanf (float __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __iseqsigf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));


extern int __issignalingf (float __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));
# 467 "/usr/include/math.h" 2 3
# 1 "/usr/include/bits/mathcalls.h" 1 3
# 53 "/usr/include/bits/mathcalls.h" 3
 extern float acosf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __acosf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float asinf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __asinf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float atanf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atanf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float atan2f (float __y, float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atan2f (float __y, float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float cosf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __cosf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float sinf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __sinf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float tanf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __tanf (float __x) __attribute__ ((__nothrow__ , __leaf__));



extern float acospif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __acospif (float __x) __attribute__ ((__nothrow__ , __leaf__));
 extern float acospif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __acospif (float __x) __attribute__ ((__nothrow__ , __leaf__));

extern float asinpif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __asinpif (float __x) __attribute__ ((__nothrow__ , __leaf__));
 extern float asinpif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __asinpif (float __x) __attribute__ ((__nothrow__ , __leaf__));

extern float atanpif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atanpif (float __x) __attribute__ ((__nothrow__ , __leaf__));
 extern float atanpif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atanpif (float __x) __attribute__ ((__nothrow__ , __leaf__));

extern float atan2pif (float __y, float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atan2pif (float __y, float __x) __attribute__ ((__nothrow__ , __leaf__));
 extern float atan2pif (float __y, float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atan2pif (float __y, float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float cospif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __cospif (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float sinpif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __sinpif (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float tanpif (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __tanpif (float __x) __attribute__ ((__nothrow__ , __leaf__));





 extern float coshf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __coshf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float sinhf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __sinhf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float tanhf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __tanhf (float __x) __attribute__ ((__nothrow__ , __leaf__));
# 107 "/usr/include/bits/mathcalls.h" 3
 extern float acoshf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __acoshf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float asinhf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __asinhf (float __x) __attribute__ ((__nothrow__ , __leaf__));

 extern float atanhf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __atanhf (float __x) __attribute__ ((__nothrow__ , __leaf__));





 extern float expf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __expf (float __x) __attribute__ ((__nothrow__ , __leaf__));


extern float frexpf (float __x, int *__exponent) __attribute__ ((__nothrow__ , __leaf__)); extern float __frexpf (float __x, int *__exponent) __attribute__ ((__nothrow__ , __leaf__));


extern float ldexpf (float __x, int __exponent) __attribute__ ((__nothrow__ , __leaf__)); extern float __ldexpf (float __x, int __exponent) __attribute__ ((__nothrow__ , __leaf__));


 extern float logf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __logf (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float log10f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __log10f (float __x) __attribute__ ((__nothrow__ , __leaf__));


extern float modff (float __x, float *__iptr) __attribute__ ((__nothrow__ , __leaf__)); extern float __modff (float __x, float *__iptr) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



 extern float exp10f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __exp10f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float exp2m1f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __exp2m1f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float exp10m1f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __exp10m1f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float log2p1f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __log2p1f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float log10p1f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __log10p1f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float logp1f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __logp1f (float __x) __attribute__ ((__nothrow__ , __leaf__));




 extern float expm1f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __expm1f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float log1pf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __log1pf (float __x) __attribute__ ((__nothrow__ , __leaf__));


extern float logbf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __logbf (float __x) __attribute__ ((__nothrow__ , __leaf__));




 extern float exp2f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __exp2f (float __x) __attribute__ ((__nothrow__ , __leaf__));


 extern float log2f (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __log2f (float __x) __attribute__ ((__nothrow__ , __leaf__));






 extern float powf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __powf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));


extern float sqrtf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __sqrtf (float __x) __attribute__ ((__nothrow__ , __leaf__));



 extern float hypotf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __hypotf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));




 extern float cbrtf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __cbrtf (float __x) __attribute__ ((__nothrow__ , __leaf__));




extern float compoundnf (float __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __compoundnf (float __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


extern float pownf (float __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __pownf (float __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


extern float powrf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __powrf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));


extern float rootnf (float __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __rootnf (float __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


 extern float rsqrtf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __rsqrtf (float __x) __attribute__ ((__nothrow__ , __leaf__));






extern float ceilf (float __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fabsf (float __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float floorf (float __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fmodf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __fmodf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));
# 231 "/usr/include/bits/mathcalls.h" 3
extern int isinff (float __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));




extern int finitef (float __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern float dremf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __dremf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));



extern float significandf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __significandf (float __x) __attribute__ ((__nothrow__ , __leaf__));






extern float copysignf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern float nanf (const char *__tagb) __attribute__ ((__nothrow__ , __leaf__)); extern float __nanf (const char *__tagb) __attribute__ ((__nothrow__ , __leaf__));
# 267 "/usr/include/bits/mathcalls.h" 3
extern int isnanf (float __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));





extern float j0f (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __j0f (float) __attribute__ ((__nothrow__ , __leaf__));
extern float j1f (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __j1f (float) __attribute__ ((__nothrow__ , __leaf__));
extern float jnf (int, float) __attribute__ ((__nothrow__ , __leaf__)); extern float __jnf (int, float) __attribute__ ((__nothrow__ , __leaf__));
extern float y0f (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __y0f (float) __attribute__ ((__nothrow__ , __leaf__));
extern float y1f (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __y1f (float) __attribute__ ((__nothrow__ , __leaf__));
extern float ynf (int, float) __attribute__ ((__nothrow__ , __leaf__)); extern float __ynf (int, float) __attribute__ ((__nothrow__ , __leaf__));





 extern float erff (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __erff (float) __attribute__ ((__nothrow__ , __leaf__));
 extern float erfcf (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __erfcf (float) __attribute__ ((__nothrow__ , __leaf__));
extern float lgammaf (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __lgammaf (float) __attribute__ ((__nothrow__ , __leaf__));




extern float tgammaf (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __tgammaf (float) __attribute__ ((__nothrow__ , __leaf__));





extern float gammaf (float) __attribute__ ((__nothrow__ , __leaf__)); extern float __gammaf (float) __attribute__ ((__nothrow__ , __leaf__));







extern float lgammaf_r (float, int *__signgamp) __attribute__ ((__nothrow__ , __leaf__)); extern float __lgammaf_r (float, int *__signgamp) __attribute__ ((__nothrow__ , __leaf__));






extern float rintf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __rintf (float __x) __attribute__ ((__nothrow__ , __leaf__));


extern float nextafterf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __nextafterf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));

extern float nexttowardf (float __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __nexttowardf (float __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));




extern float nextdownf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __nextdownf (float __x) __attribute__ ((__nothrow__ , __leaf__));

extern float nextupf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __nextupf (float __x) __attribute__ ((__nothrow__ , __leaf__));



extern float remainderf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __remainderf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));



extern float scalbnf (float __x, int __n) __attribute__ ((__nothrow__ , __leaf__)); extern float __scalbnf (float __x, int __n) __attribute__ ((__nothrow__ , __leaf__));



extern int ilogbf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern int __ilogbf (float __x) __attribute__ ((__nothrow__ , __leaf__));




extern long int llogbf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __llogbf (float __x) __attribute__ ((__nothrow__ , __leaf__));




extern float scalblnf (float __x, long int __n) __attribute__ ((__nothrow__ , __leaf__)); extern float __scalblnf (float __x, long int __n) __attribute__ ((__nothrow__ , __leaf__));



extern float nearbyintf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern float __nearbyintf (float __x) __attribute__ ((__nothrow__ , __leaf__));



extern float roundf (float __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern float truncf (float __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern float remquof (float __x, float __y, int *__quo) __attribute__ ((__nothrow__ , __leaf__)); extern float __remquof (float __x, float __y, int *__quo) __attribute__ ((__nothrow__ , __leaf__));






extern long int lrintf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __lrintf (float __x) __attribute__ ((__nothrow__ , __leaf__));
__extension__
extern long long int llrintf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern long long int __llrintf (float __x) __attribute__ ((__nothrow__ , __leaf__));



extern long int lroundf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __lroundf (float __x) __attribute__ ((__nothrow__ , __leaf__));
__extension__
extern long long int llroundf (float __x) __attribute__ ((__nothrow__ , __leaf__)); extern long long int __llroundf (float __x) __attribute__ ((__nothrow__ , __leaf__));



extern float fdimf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)); extern float __fdimf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__));



extern float fmaxf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fminf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern float fmaf (float __x, float __y, float __z) __attribute__ ((__nothrow__ , __leaf__)); extern float __fmaf (float __x, float __y, float __z) __attribute__ ((__nothrow__ , __leaf__));




extern float roundevenf (float __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern float fromfpf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern float __fromfpf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));



extern float ufromfpf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern float __ufromfpf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));




extern float fromfpxf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern float __fromfpxf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));




extern float ufromfpxf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern float __ufromfpxf (float __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));


extern int canonicalizef (float *__cx, const float *__x) __attribute__ ((__nothrow__ , __leaf__));
# 435 "/usr/include/bits/mathcalls.h" 3
extern float fmaximumf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fminimumf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fmaximum_numf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fminimum_numf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fmaximum_magf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fminimum_magf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fmaximum_mag_numf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern float fminimum_mag_numf (float __x, float __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 485 "/usr/include/bits/mathcalls.h" 3
extern float scalbf (float __x, float __n) __attribute__ ((__nothrow__ , __leaf__)); extern float __scalbf (float __x, float __n) __attribute__ ((__nothrow__ , __leaf__));
# 468 "/usr/include/math.h" 2 3
# 535 "/usr/include/math.h" 3
# 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3
# 20 "/usr/include/bits/mathcalls-helper-functions.h" 3
extern int __fpclassifyl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));


extern int __signbitl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));



extern int __isinfl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __finitel (long double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __isnanl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __iseqsigl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern int __issignalingl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));
# 536 "/usr/include/math.h" 2 3
# 1 "/usr/include/bits/mathcalls.h" 1 3
# 53 "/usr/include/bits/mathcalls.h" 3
 extern long double acosl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __acosl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double asinl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __asinl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double atanl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atanl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double atan2l (long double __y, long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atan2l (long double __y, long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double cosl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __cosl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double sinl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __sinl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double tanl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __tanl (long double __x) __attribute__ ((__nothrow__ , __leaf__));



extern long double acospil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __acospil (long double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern long double acospil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __acospil (long double __x) __attribute__ ((__nothrow__ , __leaf__));

extern long double asinpil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __asinpil (long double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern long double asinpil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __asinpil (long double __x) __attribute__ ((__nothrow__ , __leaf__));

extern long double atanpil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atanpil (long double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern long double atanpil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atanpil (long double __x) __attribute__ ((__nothrow__ , __leaf__));

extern long double atan2pil (long double __y, long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atan2pil (long double __y, long double __x) __attribute__ ((__nothrow__ , __leaf__));
 extern long double atan2pil (long double __y, long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atan2pil (long double __y, long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double cospil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __cospil (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double sinpil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __sinpil (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double tanpil (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __tanpil (long double __x) __attribute__ ((__nothrow__ , __leaf__));





 extern long double coshl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __coshl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double sinhl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __sinhl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double tanhl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __tanhl (long double __x) __attribute__ ((__nothrow__ , __leaf__));
# 107 "/usr/include/bits/mathcalls.h" 3
 extern long double acoshl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __acoshl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double asinhl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __asinhl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

 extern long double atanhl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __atanhl (long double __x) __attribute__ ((__nothrow__ , __leaf__));





 extern long double expl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __expl (long double __x) __attribute__ ((__nothrow__ , __leaf__));


extern long double frexpl (long double __x, int *__exponent) __attribute__ ((__nothrow__ , __leaf__)); extern long double __frexpl (long double __x, int *__exponent) __attribute__ ((__nothrow__ , __leaf__));


extern long double ldexpl (long double __x, int __exponent) __attribute__ ((__nothrow__ , __leaf__)); extern long double __ldexpl (long double __x, int __exponent) __attribute__ ((__nothrow__ , __leaf__));


 extern long double logl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __logl (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double log10l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __log10l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


extern long double modfl (long double __x, long double *__iptr) __attribute__ ((__nothrow__ , __leaf__)); extern long double __modfl (long double __x, long double *__iptr) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



 extern long double exp10l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __exp10l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double exp2m1l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __exp2m1l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double exp10m1l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __exp10m1l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double log2p1l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __log2p1l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double log10p1l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __log10p1l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double logp1l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __logp1l (long double __x) __attribute__ ((__nothrow__ , __leaf__));




 extern long double expm1l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __expm1l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double log1pl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __log1pl (long double __x) __attribute__ ((__nothrow__ , __leaf__));


extern long double logbl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __logbl (long double __x) __attribute__ ((__nothrow__ , __leaf__));




 extern long double exp2l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __exp2l (long double __x) __attribute__ ((__nothrow__ , __leaf__));


 extern long double log2l (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __log2l (long double __x) __attribute__ ((__nothrow__ , __leaf__));






 extern long double powl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __powl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern long double sqrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __sqrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__));



 extern long double hypotl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __hypotl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));




 extern long double cbrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __cbrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__));




extern long double compoundnl (long double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __compoundnl (long double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


extern long double pownl (long double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __pownl (long double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


extern long double powrl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __powrl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern long double rootnl (long double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __rootnl (long double __x, long long int __y) __attribute__ ((__nothrow__ , __leaf__));


 extern long double rsqrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __rsqrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__));






extern long double ceill (long double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fabsl (long double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double floorl (long double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fmodl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __fmodl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));
# 231 "/usr/include/bits/mathcalls.h" 3
extern int isinfl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));




extern int finitel (long double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern long double dreml (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __dreml (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));



extern long double significandl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __significandl (long double __x) __attribute__ ((__nothrow__ , __leaf__));






extern long double copysignl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern long double nanl (const char *__tagb) __attribute__ ((__nothrow__ , __leaf__)); extern long double __nanl (const char *__tagb) __attribute__ ((__nothrow__ , __leaf__));
# 267 "/usr/include/bits/mathcalls.h" 3
extern int isnanl (long double __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));





extern long double j0l (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __j0l (long double) __attribute__ ((__nothrow__ , __leaf__));
extern long double j1l (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __j1l (long double) __attribute__ ((__nothrow__ , __leaf__));
extern long double jnl (int, long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __jnl (int, long double) __attribute__ ((__nothrow__ , __leaf__));
extern long double y0l (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __y0l (long double) __attribute__ ((__nothrow__ , __leaf__));
extern long double y1l (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __y1l (long double) __attribute__ ((__nothrow__ , __leaf__));
extern long double ynl (int, long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __ynl (int, long double) __attribute__ ((__nothrow__ , __leaf__));





 extern long double erfl (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __erfl (long double) __attribute__ ((__nothrow__ , __leaf__));
 extern long double erfcl (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __erfcl (long double) __attribute__ ((__nothrow__ , __leaf__));
extern long double lgammal (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __lgammal (long double) __attribute__ ((__nothrow__ , __leaf__));




extern long double tgammal (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __tgammal (long double) __attribute__ ((__nothrow__ , __leaf__));





extern long double gammal (long double) __attribute__ ((__nothrow__ , __leaf__)); extern long double __gammal (long double) __attribute__ ((__nothrow__ , __leaf__));







extern long double lgammal_r (long double, int *__signgamp) __attribute__ ((__nothrow__ , __leaf__)); extern long double __lgammal_r (long double, int *__signgamp) __attribute__ ((__nothrow__ , __leaf__));






extern long double rintl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __rintl (long double __x) __attribute__ ((__nothrow__ , __leaf__));


extern long double nextafterl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __nextafterl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));

extern long double nexttowardl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __nexttowardl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));




extern long double nextdownl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __nextdownl (long double __x) __attribute__ ((__nothrow__ , __leaf__));

extern long double nextupl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __nextupl (long double __x) __attribute__ ((__nothrow__ , __leaf__));



extern long double remainderl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __remainderl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));



extern long double scalbnl (long double __x, int __n) __attribute__ ((__nothrow__ , __leaf__)); extern long double __scalbnl (long double __x, int __n) __attribute__ ((__nothrow__ , __leaf__));



extern int ilogbl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern int __ilogbl (long double __x) __attribute__ ((__nothrow__ , __leaf__));




extern long int llogbl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __llogbl (long double __x) __attribute__ ((__nothrow__ , __leaf__));




extern long double scalblnl (long double __x, long int __n) __attribute__ ((__nothrow__ , __leaf__)); extern long double __scalblnl (long double __x, long int __n) __attribute__ ((__nothrow__ , __leaf__));



extern long double nearbyintl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long double __nearbyintl (long double __x) __attribute__ ((__nothrow__ , __leaf__));



extern long double roundl (long double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern long double truncl (long double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern long double remquol (long double __x, long double __y, int *__quo) __attribute__ ((__nothrow__ , __leaf__)); extern long double __remquol (long double __x, long double __y, int *__quo) __attribute__ ((__nothrow__ , __leaf__));






extern long int lrintl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __lrintl (long double __x) __attribute__ ((__nothrow__ , __leaf__));
__extension__
extern long long int llrintl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long long int __llrintl (long double __x) __attribute__ ((__nothrow__ , __leaf__));



extern long int lroundl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long int __lroundl (long double __x) __attribute__ ((__nothrow__ , __leaf__));
__extension__
extern long long int llroundl (long double __x) __attribute__ ((__nothrow__ , __leaf__)); extern long long int __llroundl (long double __x) __attribute__ ((__nothrow__ , __leaf__));



extern long double fdiml (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)); extern long double __fdiml (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));



extern long double fmaxl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fminl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern long double fmal (long double __x, long double __y, long double __z) __attribute__ ((__nothrow__ , __leaf__)); extern long double __fmal (long double __x, long double __y, long double __z) __attribute__ ((__nothrow__ , __leaf__));




extern long double roundevenl (long double __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern long double fromfpl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern long double __fromfpl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));



extern long double ufromfpl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern long double __ufromfpl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));




extern long double fromfpxl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern long double __fromfpxl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));




extern long double ufromfpxl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__)); extern long double __ufromfpxl (long double __x, int __round, unsigned int __width) __attribute__ ((__nothrow__ , __leaf__));


extern int canonicalizel (long double *__cx, const long double *__x) __attribute__ ((__nothrow__ , __leaf__));
# 435 "/usr/include/bits/mathcalls.h" 3
extern long double fmaximuml (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fminimuml (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fmaximum_numl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fminimum_numl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fmaximum_magl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fminimum_magl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fmaximum_mag_numl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));


extern long double fminimum_mag_numl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 485 "/usr/include/bits/mathcalls.h" 3
extern long double scalbl (long double __x, long double __n) __attribute__ ((__nothrow__ , __leaf__)); extern long double __scalbl (long double __x, long double __n) __attribute__ ((__nothrow__ , __leaf__));
# 537 "/usr/include/math.h" 2 3
# 618 "/usr/include/math.h" 3
# 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3
# 20 "/usr/include/bits/mathcalls-helper-functions.h" 3
extern int __fpclassifyf128 (_Float128 __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));


extern int __signbitf128 (_Float128 __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));



extern int __isinff128 (_Float128 __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __finitef128 (_Float128 __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __isnanf128 (_Float128 __value) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__const__));


extern int __iseqsigf128 (_Float128 __x, _Float128 __y) __attribute__ ((__nothrow__ , __leaf__));


extern int __issignalingf128 (_Float128 __value) __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__const__));
# 619 "/usr/include/math.h" 2 3
# 703 "/usr/include/math.h" 3
# 1 "/usr/include/bits/mathcalls-narrow.h" 1 3
# 24 "/usr/include/bits/mathcalls-narrow.h" 3
extern float fadd (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));


extern float fdiv (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));


extern float ffma (double __x, double __y, double __z) __attribute__ ((__nothrow__ , __leaf__));


extern float fmul (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));


extern float fsqrt (double __x) __attribute__ ((__nothrow__ , __leaf__));


extern float fsub (double __x, double __y) __attribute__ ((__nothrow__ , __leaf__));
# 704 "/usr/include/math.h" 2 3
# 724 "/usr/include/math.h" 3
# 1 "/usr/include/bits/mathcalls-narrow.h" 1 3
# 24 "/usr/include/bits/mathcalls-narrow.h" 3
extern float faddl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern float fdivl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern float ffmal (long double __x, long double __y, long double __z) __attribute__ ((__nothrow__ , __leaf__));


extern float fmull (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern float fsqrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__));


extern float fsubl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));
# 725 "/usr/include/math.h" 2 3
# 753 "/usr/include/math.h" 3
# 1 "/usr/include/bits/mathcalls-narrow.h" 1 3
# 24 "/usr/include/bits/mathcalls-narrow.h" 3
extern double daddl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern double ddivl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern double dfmal (long double __x, long double __y, long double __z) __attribute__ ((__nothrow__ , __leaf__));


extern double dmull (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));


extern double dsqrtl (long double __x) __attribute__ ((__nothrow__ , __leaf__));


extern double dsubl (long double __x, long double __y) __attribute__ ((__nothrow__ , __leaf__));
# 754 "/usr/include/math.h" 2 3
# 991 "/usr/include/math.h" 3
extern int signgam;
# 1071 "/usr/include/math.h" 3
enum
  {
    FP_NAN =

      0,
    FP_INFINITE =

      1,
    FP_ZERO =

      2,
    FP_SUBNORMAL =

      3,
    FP_NORMAL =

      4
  };
# 1192 "/usr/include/math.h" 3
# 1 "/usr/include/bits/iscanonical.h" 1 3
# 23 "/usr/include/bits/iscanonical.h" 3
extern int __iscanonicall (long double __x)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 1193 "/usr/include/math.h" 2 3
# 1609 "/usr/include/math.h" 3

# 38 "src/louro/louro.h" 2
# 51 "src/louro/louro.h"

# 51 "src/louro/louro.h"
typedef struct LouroExpression {
    int type;
    union {double value; const double *bound; const void *function;};
    void *parameters[1];
} LouroExpression;


enum {
    LOURO_VARIABLE = 0,

    LOURO_FUNCTION0 = 8, LOURO_FUNCTION1, LOURO_FUNCTION2, LOURO_FUNCTION3,
    LOURO_FUNCTION4, LOURO_FUNCTION5, LOURO_FUNCTION6, LOURO_FUNCTION7,

    LOURO_CLOSURE0 = 16, LOURO_CLOSURE1, LOURO_CLOSURE2, LOURO_CLOSURE3,
    LOURO_CLOSURE4, LOURO_CLOSURE5, LOURO_CLOSURE6, LOURO_CLOSURE7,

    LOURO_FLAG_PURE = 32,
    LOURO_OPERATOR = 64,
    LOURO_FLAG_RIGHT_ASSOC = 128,

    LOURO_FLAG_INFIX = 256,
    LOURO_FLAG_PREFIX = 512,
    LOURO_FLAG_POSTFIX = 1024
};


typedef struct LouroVariable {
    const char *name;
    const void *address;
    int type;
    void *context;
} LouroVariable;
# 109 "src/louro/louro.h"
static inline LouroExpression *louro_compile(const char *expression, const LouroVariable *variables, int var_count, int *error);


static inline double louro_evaluate(const LouroExpression *n);



static inline void louro_free(LouroExpression *n);
# 125 "src/louro/louro.h"
typedef double (*lr_fun2)(double, double);

enum {
    TOK_NULL = LOURO_CLOSURE7+1, TOK_ERROR, TOK_END, TOK_SEP,
    TOK_OPEN, TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_OPERATOR
};


enum {LOURO_CONSTANT = 1};


typedef struct state {
    const char *start;
    const char *next;
    int type;
    union {double value; const double *bound; const void *function;};
    void *context;

    const LouroVariable *lookup;
    int lookup_len;

    int expecting_operator;
    int op_precedence;
    int op_flags;
} state;
# 161 "src/louro/louro.h"
static inline LouroExpression *new_expr(const int type, const LouroExpression *parameters[]) {
    const int arity = ( ((type) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((type) & 0x00000007) : 0 );
    const int psize = sizeof(void*) * arity;
    const int size = (sizeof(LouroExpression) - sizeof(void*)) + psize + ((((type) & LOURO_CLOSURE0) != 0) ? sizeof(void*) : 0);
    LouroExpression *ret = (LouroExpression*)malloc(size);
    if ((ret) == 
# 166 "src/louro/louro.h" 3
   ((void *)0)
# 166 "src/louro/louro.h"
   ) { ; { printf("NULL at %d\n", 166); return 
# 166 "src/louro/louro.h" 3
   ((void *)0)
# 166 "src/louro/louro.h"
   ; }; };

    memset(ret, 0, size);
    if (arity && parameters) {
        memcpy(ret->parameters, parameters, psize);
    }
    ret->type = type;
    ret->bound = 0;
    return ret;
}


static inline void louro_free_parameters(LouroExpression *n) {
    if (!n) return;
    switch (((n->type)&0x0000001F)) {
        case LOURO_FUNCTION7: case LOURO_CLOSURE7: louro_free((LouroExpression*)n->parameters[6]);
        case LOURO_FUNCTION6: case LOURO_CLOSURE6: louro_free((LouroExpression*)n->parameters[5]);
        case LOURO_FUNCTION5: case LOURO_CLOSURE5: louro_free((LouroExpression*)n->parameters[4]);
        case LOURO_FUNCTION4: case LOURO_CLOSURE4: louro_free((LouroExpression*)n->parameters[3]);
        case LOURO_FUNCTION3: case LOURO_CLOSURE3: louro_free((LouroExpression*)n->parameters[2]);
        case LOURO_FUNCTION2: case LOURO_CLOSURE2: louro_free((LouroExpression*)n->parameters[1]);
        case LOURO_FUNCTION1: case LOURO_CLOSURE1: louro_free((LouroExpression*)n->parameters[0]);
    }
}


static inline void louro_free(LouroExpression *n) {
    if (!n) return;
    louro_free_parameters(n);
    free(n);
}




static inline const LouroVariable *find_lookup(const state *s, const char *name, int len) {
    int iters;
    const LouroVariable *var;
    if (!s->lookup) return 0;

    for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) {
        if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0') {
            return var;
        }
    }
    return 0;
}

static inline void next_token(state *s) {
    s->type = TOK_NULL;

    do {
        if (!*s->next){
            s->type = TOK_END;
            return;
        }


        if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') {
            s->value = strtod(s->next, (char**)&s->next);
            s->type = TOK_NUMBER;
            return;
        }




        const LouroVariable *best_match = 
# 233 "src/louro/louro.h" 3
                                         ((void *)0)
# 233 "src/louro/louro.h"
                                             ;
        int best_match_len = 0;

        if (s->lookup) {
            for (int i = 0; i < s->lookup_len; ++i) {
                const LouroVariable *var = &s->lookup[i];
                int len = strlen(var->name);
                if (strncmp(s->next, var->name, len) == 0) {

                    if (
# 242 "src/louro/louro.h" 3
                       ((*__ctype_b_loc ())[(int) ((
# 242 "src/louro/louro.h"
                       var->name[0]
# 242 "src/louro/louro.h" 3
                       ))] & (unsigned short int) _ISalpha)
# 242 "src/louro/louro.h"
                                            ) {
                        if (
# 243 "src/louro/louro.h" 3
                           ((*__ctype_b_loc ())[(int) ((
# 243 "src/louro/louro.h"
                           s->next[len]
# 243 "src/louro/louro.h" 3
                           ))] & (unsigned short int) _ISalnum) 
# 243 "src/louro/louro.h"
                                                 || s->next[len] == '_') continue;
                    }

                    if (var->type & LOURO_OPERATOR) {
                        if (s->expecting_operator) {
                            if (!(var->type & (LOURO_FLAG_INFIX | LOURO_FLAG_POSTFIX))) continue;
                        } else {
                            if (!(var->type & LOURO_FLAG_PREFIX)) continue;
                        }
                    }

                    if (len > best_match_len) {
                        best_match = var;
                        best_match_len = len;
                    }
                }
            }
        }

        if (best_match) {
            s->next += best_match_len;
            if (best_match->type & LOURO_OPERATOR) {
                s->type = TOK_OPERATOR;
                s->function = best_match->address;
                s->op_precedence = (best_match->type >> 12);
                s->op_flags = best_match->type & (LOURO_FLAG_RIGHT_ASSOC | LOURO_FLAG_INFIX | LOURO_FLAG_PREFIX | LOURO_FLAG_POSTFIX);
                return;
            } else {
                switch(((best_match->type)&0x0000001F)) {
                    case LOURO_VARIABLE:
                        s->type = TOK_VARIABLE;
                        s->bound = (const double*)best_match->address;
                        return;
                    case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3:
                    case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
                        s->context = best_match->context;
                    case LOURO_FUNCTION0: case LOURO_FUNCTION1: case LOURO_FUNCTION2: case LOURO_FUNCTION3:
                    case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7:
                        s->type = best_match->type;
                        s->function = best_match->address;
                        return;
                }
            }
        }


        int matched = 1;
        switch (s->next[0]) {
            case '(': s->type = TOK_OPEN; s->next++; break;
            case ')': s->type = TOK_CLOSE; s->next++; break;
            case ',': s->type = TOK_SEP; s->function = 0; s->op_precedence = 10; s->op_flags = 0; s->next++; break;
            case ' ': case '\t': case '\n': case '\r': s->next++; matched = 0; break;
            default:

                if (
# 297 "src/louro/louro.h" 3
                   ((*__ctype_b_loc ())[(int) ((
# 297 "src/louro/louro.h"
                   s->next[0]
# 297 "src/louro/louro.h" 3
                   ))] & (unsigned short int) _ISalpha)
# 297 "src/louro/louro.h"
                                      ) {
                    while (
# 298 "src/louro/louro.h" 3
                          ((*__ctype_b_loc ())[(int) ((
# 298 "src/louro/louro.h"
                          s->next[0]
# 298 "src/louro/louro.h" 3
                          ))] & (unsigned short int) _ISalpha) 
# 298 "src/louro/louro.h"
                                              || 
# 298 "src/louro/louro.h" 3
                                                 ((*__ctype_b_loc ())[(int) ((
# 298 "src/louro/louro.h"
                                                 s->next[0]
# 298 "src/louro/louro.h" 3
                                                 ))] & (unsigned short int) _ISdigit) 
# 298 "src/louro/louro.h"
                                                                     || (s->next[0] == '_')) s->next++;
                } else {
                    s->next++;
                }
                s->type = TOK_ERROR;
                break;
        }
        if (matched && s->type != TOK_NULL) return;

    } while (s->type == TOK_NULL);
}

static inline LouroExpression *parse_expr_dynamic(state *s, int precedence);

static inline LouroExpression *base(state *s) {
    LouroExpression *ret;
    int arity;

    switch (((s->type)&0x0000001F)) {
        case TOK_NUMBER:
            ret = new_expr(LOURO_CONSTANT, 0);
            if(!ret) { s->type = TOK_ERROR; { printf("NULL at %d\n", 319); return 
# 319 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 319 "src/louro/louro.h"
                                                                                           ; }; }
            ret->value = s->value;
            s->expecting_operator = 1;
            next_token(s);
            break;

        case TOK_VARIABLE:
            ret = new_expr(LOURO_VARIABLE, 0);
            if(!ret) { s->type = TOK_ERROR; { printf("NULL at %d\n", 327); return 
# 327 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 327 "src/louro/louro.h"
                                                                                           ; }; }
            ret->bound = s->bound;
            s->expecting_operator = 1;
            next_token(s);
            break;

        case LOURO_FUNCTION0:
        case LOURO_CLOSURE0:
            ret = new_expr(s->type, 0);
            if(!ret) { s->type = TOK_ERROR; { printf("NULL at %d\n", 336); return 
# 336 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 336 "src/louro/louro.h"
                                                                                           ; }; }
            ret->function = s->function;
            if ((((s->type) & LOURO_CLOSURE0) != 0)) ret->parameters[0] = s->context;

            s->expecting_operator = 1;
            next_token(s);
            if (s->type == TOK_OPEN) {
                s->expecting_operator = 0;
                next_token(s);
                if (s->type != TOK_CLOSE) {
                    s->type = TOK_ERROR;
                } else {
                    s->expecting_operator = 1;
                    next_token(s);
                }
            }
            break;

        case LOURO_FUNCTION1:
        case LOURO_CLOSURE1:
            ret = new_expr(s->type, 0);
            if(!ret) { s->type = TOK_ERROR; { printf("NULL at %d\n", 357); return 
# 357 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 357 "src/louro/louro.h"
                                                                                           ; }; }
            ret->function = s->function;
            if ((((s->type) & LOURO_CLOSURE0) != 0)) ret->parameters[1] = s->context;

            s->expecting_operator = 0;
            next_token(s);

            ret->parameters[0] = parse_expr_dynamic(s, 60);
            if(!ret->parameters[0]) { louro_free(ret); { printf("NULL at %d\n", 365); return 
# 365 "src/louro/louro.h" 3
                                                                                                  ((void *)0)
# 365 "src/louro/louro.h"
                                                                                                      ; }; }
            break;

        case LOURO_FUNCTION2: case LOURO_FUNCTION3: case LOURO_FUNCTION4:
        case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7:
        case LOURO_CLOSURE2: case LOURO_CLOSURE3: case LOURO_CLOSURE4:
        case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
            arity = ( ((s->type) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((s->type) & 0x00000007) : 0 );
            ret = new_expr(s->type, 0);
            if(!ret) { s->type = TOK_ERROR; { printf("NULL at %d\n", 374); return 
# 374 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 374 "src/louro/louro.h"
                                                                                           ; }; }
            ret->function = s->function;
            if ((((s->type) & LOURO_CLOSURE0) != 0)) ret->parameters[arity] = s->context;

            s->expecting_operator = 0;
            next_token(s);

            if (s->type != TOK_OPEN) {
                s->type = TOK_ERROR;
            } else {
                int i;
                for(i = 0; i < arity; i++) {
                    s->expecting_operator = 0;
                    next_token(s);
                    ret->parameters[i] = parse_expr_dynamic(s, 0);
                    if(!ret->parameters[i]) { louro_free(ret); { printf("NULL at %d\n", 389); return 
# 389 "src/louro/louro.h" 3
                                                                                                          ((void *)0)
# 389 "src/louro/louro.h"
                                                                                                              ; }; }

                    if(s->type != TOK_SEP) {
                        break;
                    }
                }
                if(s->type != TOK_CLOSE || i != arity - 1) {
                    s->type = TOK_ERROR;
                } else {
                    s->expecting_operator = 1;
                    next_token(s);
                }
            }
            break;

        case TOK_OPEN:
            s->expecting_operator = 0;
            next_token(s);
            ret = parse_expr_dynamic(s, 0);
            if(!ret) { printf("NULL at %d\n", 408); return 
# 408 "src/louro/louro.h" 3
                                                                ((void *)0)
# 408 "src/louro/louro.h"
                                                                    ; };

            if (s->type != TOK_CLOSE) {
                s->type = TOK_ERROR;
            } else {
                s->expecting_operator = 1;
                next_token(s);
            }
            break;

        default:
            ret = new_expr(0, 0);
            if(!ret) { s->type = TOK_ERROR; { printf("NULL at %d\n", 420); return 
# 420 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 420 "src/louro/louro.h"
                                                                                           ; }; }
            s->type = TOK_ERROR;
            ret->value = 
# 422 "src/louro/louro.h" 3
                        (__builtin_nanf (""))
# 422 "src/louro/louro.h"
                           ;
            break;
    }

    return ret;
}

static inline LouroExpression *parse_prefix(state *s) {
    if (s->type == TOK_OPERATOR && (s->op_flags & LOURO_FLAG_PREFIX)) {
        const void *func = s->function;
        int prec = s->op_precedence;

        s->expecting_operator = 0;
        next_token(s);

        LouroExpression *operand = parse_expr_dynamic(s, prec);
        if (!operand) { printf("NULL at %d\n", 438); return 
# 438 "src/louro/louro.h" 3
                                                                 ((void *)0)
# 438 "src/louro/louro.h"
                                                                     ; };

        LouroExpression *ret = new_expr(LOURO_FUNCTION1 | LOURO_FLAG_PURE, 0);
        if (!ret) { louro_free(operand); { printf("NULL at %d\n", 441); return 
# 441 "src/louro/louro.h" 3
                                                                                    ((void *)0)
# 441 "src/louro/louro.h"
                                                                                        ; }; }
        ret->function = func;
        ret->parameters[0] = operand;
        return ret;
    }
    return base(s);
}

static inline LouroExpression *parse_expr_dynamic(state *s, int current_precedence) {
    LouroExpression *left = parse_prefix(s);
    if (!left) { printf("NULL at %d\n", 451); return 
# 451 "src/louro/louro.h" 3
                                                          ((void *)0)
# 451 "src/louro/louro.h"
                                                              ; };

    while (s->type == TOK_OPERATOR) {
        if (s->op_flags & LOURO_FLAG_POSTFIX) {
            if (s->op_precedence < current_precedence) break;
            const void *func = s->function;

            s->expecting_operator = 1;
            next_token(s);

            LouroExpression *new_left = new_expr(LOURO_FUNCTION1 | LOURO_FLAG_PURE, 0);
            if (!new_left) { louro_free(left); { printf("NULL at %d\n", 462); return 
# 462 "src/louro/louro.h" 3
                                                                                          ((void *)0)
# 462 "src/louro/louro.h"
                                                                                              ; }; }
            new_left->function = func;
            new_left->parameters[0] = left;
            left = new_left;
            continue;
        }

        if (s->op_flags & LOURO_FLAG_INFIX) {
            if (s->op_precedence < current_precedence) break;

            int op_prec = s->op_precedence;
            int right_assoc = (s->op_flags & LOURO_FLAG_RIGHT_ASSOC);
            const void *func = s->function;

            s->expecting_operator = 0;
            next_token(s);

            int next_prec = right_assoc ? op_prec : (op_prec + 1);
            LouroExpression *right = parse_expr_dynamic(s, next_prec);
            if (!right) { louro_free(left); { printf("NULL at %d\n", 481); return 
# 481 "src/louro/louro.h" 3
                                                                                       ((void *)0)
# 481 "src/louro/louro.h"
                                                                                           ; }; }

            LouroExpression *new_left = new_expr(LOURO_FUNCTION2 | LOURO_FLAG_PURE, 0);
            if (!new_left) { louro_free(left); louro_free(right); { printf("NULL at %d\n", 484); return 
# 484 "src/louro/louro.h" 3
                                                                                                             ((void *)0)
# 484 "src/louro/louro.h"
                                                                                                                 ; }; }
            new_left->function = func;
            new_left->parameters[0] = left;
            new_left->parameters[1] = right;
            left = new_left;
            continue;
        }

        break;
    }
    return left;
}





static inline double louro_evaluate(const LouroExpression *n) {
    if (!n) return 
# 502 "src/louro/louro.h" 3
                  (__builtin_nanf (""))
# 502 "src/louro/louro.h"
                     ;

    switch(((n->type)&0x0000001F)) {
        case LOURO_CONSTANT: return n->value;
        case LOURO_VARIABLE: return *n->bound;

        case LOURO_FUNCTION0: case LOURO_FUNCTION1: case LOURO_FUNCTION2: case LOURO_FUNCTION3:
        case LOURO_FUNCTION4: case LOURO_FUNCTION5: case LOURO_FUNCTION6: case LOURO_FUNCTION7:
            switch(( ((n->type) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((n->type) & 0x00000007) : 0 )) {
                case 0: return ((double(*)(void))n->function)();
                case 1: return ((double(*)(double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]));
                case 2: return ((double(*)(double, double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]));
                case 3: return ((double(*)(double, double, double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]));
                case 4: return ((double(*)(double, double, double, double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]));
                case 5: return ((double(*)(double, double, double, double, double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]), louro_evaluate((LouroExpression*)n->parameters[4]));
                case 6: return ((double(*)(double, double, double, double, double, double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]), louro_evaluate((LouroExpression*)n->parameters[4]), louro_evaluate((LouroExpression*)n->parameters[5]));
                case 7: return ((double(*)(double, double, double, double, double, double, double))n->function)(louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]), louro_evaluate((LouroExpression*)n->parameters[4]), louro_evaluate((LouroExpression*)n->parameters[5]), louro_evaluate((LouroExpression*)n->parameters[6]));
                default: return 
# 519 "src/louro/louro.h" 3
                               (__builtin_nanf (""))
# 519 "src/louro/louro.h"
                                  ;
            }

        case LOURO_CLOSURE0: case LOURO_CLOSURE1: case LOURO_CLOSURE2: case LOURO_CLOSURE3:
        case LOURO_CLOSURE4: case LOURO_CLOSURE5: case LOURO_CLOSURE6: case LOURO_CLOSURE7:
            switch(( ((n->type) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((n->type) & 0x00000007) : 0 )) {
                case 0: return ((double(*)(void*))n->function)(n->parameters[0]);
                case 1: return ((double(*)(void*, double))n->function)(n->parameters[1], louro_evaluate((LouroExpression*)n->parameters[0]));
                case 2: return ((double(*)(void*, double, double))n->function)(n->parameters[2], louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]));
                case 3: return ((double(*)(void*, double, double, double))n->function)(n->parameters[3], louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]));
                case 4: return ((double(*)(void*, double, double, double, double))n->function)(n->parameters[4], louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]));
                case 5: return ((double(*)(void*, double, double, double, double, double))n->function)(n->parameters[5], louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]), louro_evaluate((LouroExpression*)n->parameters[4]));
                case 6: return ((double(*)(void*, double, double, double, double, double, double))n->function)(n->parameters[6], louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]), louro_evaluate((LouroExpression*)n->parameters[4]), louro_evaluate((LouroExpression*)n->parameters[5]));
                case 7: return ((double(*)(void*, double, double, double, double, double, double, double))n->function)(n->parameters[7], louro_evaluate((LouroExpression*)n->parameters[0]), louro_evaluate((LouroExpression*)n->parameters[1]), louro_evaluate((LouroExpression*)n->parameters[2]), louro_evaluate((LouroExpression*)n->parameters[3]), louro_evaluate((LouroExpression*)n->parameters[4]), louro_evaluate((LouroExpression*)n->parameters[5]), louro_evaluate((LouroExpression*)n->parameters[6]));
                default: return 
# 533 "src/louro/louro.h" 3
                               (__builtin_nanf (""))
# 533 "src/louro/louro.h"
                                  ;
            }

        default: return 
# 536 "src/louro/louro.h" 3
                       (__builtin_nanf (""))
# 536 "src/louro/louro.h"
                          ;
    }

}




static inline void optimize(LouroExpression *n) {

    if (n->type == LOURO_CONSTANT) return;
    if (n->type == LOURO_VARIABLE) return;


    if ((((n->type) & LOURO_FLAG_PURE) != 0)) {
        const int arity = ( ((n->type) & (LOURO_FUNCTION0 | LOURO_CLOSURE0)) ? ((n->type) & 0x00000007) : 0 );
        int known = 1;
        int i;
        for (i = 0; i < arity; ++i) {
            optimize((LouroExpression*)n->parameters[i]);
            if (((LouroExpression*)(n->parameters[i]))->type != LOURO_CONSTANT) {
                known = 0;
            }
        }
        if (known) {
            const double value = louro_evaluate(n);
            louro_free_parameters(n);
            n->type = LOURO_CONSTANT;
            n->value = value;
        }
    }
}


static inline LouroExpression *louro_compile(const char *expression, const LouroVariable *variables, int var_count, int *error) {
    state s = { 0 };
    s.start = s.next = expression;
    s.context = 0;
    s.lookup = variables;
    s.lookup_len = var_count;
    s.expecting_operator = 0;

    next_token(&s);
    LouroExpression *root = parse_expr_dynamic(&s, 0);
    if (root == 
# 580 "src/louro/louro.h" 3
               ((void *)0)
# 580 "src/louro/louro.h"
                   ) {
        if (error) *error = -1;
         { printf("NULL at %d\n", 582); return 
# 582 "src/louro/louro.h" 3
                                                   ((void *)0)
# 582 "src/louro/louro.h"
                                                       ; };
    }

    if (s.type != TOK_END) {
        louro_free(root);
        if (error) {
            *error = (s.next - s.start);
            if (*error == 0) *error = 1;
        }
        return 0;
    } else {
        optimize(root);
        if (error) *error = 0;
        return root;
    }
}
# 12 "src/papagaio.c" 2
# 1 "src/louro/libs/louro_std.h" 1



# 1 "src/louro/libs/../louro.h" 1
# 5 "src/louro/libs/louro_std.h" 2



static inline double lr_add(double a, double b) {return a + b;}
static inline double lr_sub(double a, double b) {return a - b;}
static inline double lr_mul(double a, double b) {return a * b;}
static inline double lr_divide(double a, double b) {return a / b;}
static inline double lr_negate(double a) {return -a;}
static inline double lr_comma(double a, double b) {(void)a; return b;}


static inline double lr_cmp_lt(double a, double b) {return a < b ? 1.0 : 0.0;}
static inline double lr_cmp_gt(double a, double b) {return a > b ? 1.0 : 0.0;}
static inline double lr_cmp_le(double a, double b) {return a <= b ? 1.0 : 0.0;}
static inline double lr_cmp_ge(double a, double b) {return a >= b ? 1.0 : 0.0;}
static inline double lr_cmp_eq(double a, double b) {return a == b ? 1.0 : 0.0;}
static inline double lr_cmp_ne(double a, double b) {return a != b ? 1.0 : 0.0;}
# 13 "src/papagaio.c" 2
# 1 "src/louro/libs/louro_math.h" 1
# 11 "src/louro/libs/louro_math.h"
static inline double lr_pi(void) { return 3.14159265358979323846; }
# 14 "src/papagaio.c" 2







typedef struct { const char *ptr; size_t len; } StrView;
typedef struct { char *data; size_t len; size_t cap; } StrBuf;

typedef enum {
    TOK_LITERAL, TOK_VAR, TOK_BLOCK, TOK_WS,
    TOK_OPTIONS_OBSOLETE, TOK_OPTIONAL_LIT
} PapTokenType;

typedef enum {
    MOD_NONE, MOD_INT, MOD_FLOAT, MOD_NUMBER,
    MOD_UPPER, MOD_LOWER, MOD_CAPITALIZED,
    MOD_WORD, MOD_IDENTIFIER, MOD_HEX, MOD_PATH,
    MOD_BINARY, MOD_PERCENT, MOD_ALIASES,
    MOD_GROUP, MOD_STARTS, MOD_ENDS,
    MOD_PREFIX, MOD_SUFFIX, MOD_INFIX,
    MOD_INCLUDES, MOD_REPEAT, MOD_WHILE, MOD_UNTIL, MOD_BYTE
} VarModifier;


typedef struct Pattern_s Pattern;

typedef struct {
    PapTokenType type;
    VarModifier modifier;
    StrView value;
    StrView var;
    StrView open;
    StrView close;

    char *open_str;
    char *close_str;
    unsigned optional : 1;
    unsigned ws_consume : 1;
    int next_sig;
    unsigned all_opt : 1;
    char **alts;
    int alt_count;
    char *literal_str;
    Pattern *sub_pattern;
    Pattern **alt_patterns;
    int alt_pattern_count;
} PapToken;

typedef struct {
    char sigil[16];
    char open[16];
    char close[16];
    char optional[16];
    const char *pattern, *block, *changequote;
} Symbols;

struct Pattern_s { PapToken *t; int count; int cap; Symbols sym; };
typedef struct { StrView name; StrView value; char *owned; } Capture;

typedef struct {
    Capture *cap;
    int count;
    int cap_size;
    int start;
    int end;
    const char *src;

    Papagaio *ctx;
} Match;

typedef struct { Pattern pattern; const char *replacement; } Rule;
typedef struct { char *m; char *r; } PatternPair;





typedef struct {
    PapFinalizer fn;
    void *userdata;
} RegisteredFinalizer;

typedef struct {
    char *name;
    PapCommandHandler handler;
    void *userdata;
} RegisteredCommand;

typedef struct {
    char *name;
    PapModifierHandler handler;
    void *userdata;
} RegisteredModifier;


static char *file_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);

typedef struct Scope {
    PatternPair *rules;
    int rule_count, rule_cap;
    struct Scope *parent;
} Scope;

struct Papagaio {
    RegisteredCommand *commands;
    int cmd_count, cmd_cap;

    RegisteredModifier *modifiers;
    int mod_count, mod_cap;

    RegisteredFinalizer *finalizers;
    int fin_count, fin_cap;


    int argc;
    char **argv;


    int auto_export;


    Scope *global_scope;
    Scope *current_scope;
    int depth;
    int disable_sandbox;
    int disable_patterns;


    char *original_doc;
    size_t original_len;
};
# 165 "src/papagaio.c"
static void sb_init(StrBuf *b)
{
    b->cap = 256; b->len = 0;
    b->data = (char *)malloc(b->cap);
    b->data[0] = '\0';
}
static void sb_grow(StrBuf *b, size_t n)
{
    size_t need = b->len + n + 1;
    if (need <= b->cap) return;
    size_t cap = b->cap;
    while (cap < need) cap <<= 1;
    b->data = (char *)realloc(b->data, cap);
    b->cap = cap;
}
static void sb_append_n(StrBuf *b, const char *s, size_t n)
{
    if (!n) return;
    sb_grow(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}
static void sb_append_char(StrBuf *b, char c)
{
    sb_grow(b, 1);
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}
static void sb_free(StrBuf *b)
{
    free(b->data); b->data = 
# 196 "src/papagaio.c" 3
                            ((void *)0)
# 196 "src/papagaio.c"
                                ; b->len = 0; b->cap = 0;
}





static Symbols make_symbols(const char *sigil, const char *open, const char *close)
{
    Symbols s;
    memset(&s, 0, sizeof(s));
    if (sigil) { strncpy(s.sigil, sigil, 15); s.sigil[15] = '\0'; }
    if (open) { strncpy(s.open, open, 15); s.open[15] = '\0'; }
    if (close) { strncpy(s.close, close, 15); s.close[15] = '\0'; }
    strcpy(s.optional, "?");
    s.pattern = "pattern";
    s.block = "block";
    s.changequote = "changequote";
    return s;
}

static StrView trim_sv(StrView v)
{
    size_t s = 0, e = v.len;
    while (s < v.len && 
# 220 "src/papagaio.c" 3
                       ((*__ctype_b_loc ())[(int) ((
# 220 "src/papagaio.c"
                       (unsigned char)v.ptr[s]
# 220 "src/papagaio.c" 3
                       ))] & (unsigned short int) _ISspace)
# 220 "src/papagaio.c"
                                                       ) s++;
    while (e > s && 
# 221 "src/papagaio.c" 3
                      ((*__ctype_b_loc ())[(int) ((
# 221 "src/papagaio.c"
                      (unsigned char)v.ptr[e-1]
# 221 "src/papagaio.c" 3
                      ))] & (unsigned short int) _ISspace)
# 221 "src/papagaio.c"
                                                        ) e--;
    return (StrView){ v.ptr + s, e - s };
}

static int sv_eq(StrView a, StrView b)
{ return a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0; }

static int sv_pfx(const char *s, StrView v)
{
    if (v.len == 0) return 0;
    for (size_t i = 0; i < v.len; i++) {
        if (s[i] == '\0' || s[i] != v.ptr[i]) return 0;
    }
    return 1;
}

static int str_pfx(const char *s, const char *p)
{
    if (!p || *p == '\0') return 0;
    while (*p) {
        if (*s == '\0' || *s != *p) return 0;
        s++; p++;
    }
    return 1;
}

static void skip_ws(const char *s, int *p)
{ while (
# 248 "src/papagaio.c" 3
        ((*__ctype_b_loc ())[(int) ((
# 248 "src/papagaio.c"
        (unsigned char)s[*p]
# 248 "src/papagaio.c" 3
        ))] & (unsigned short int) _ISspace)
# 248 "src/papagaio.c"
                                     ) (*p)++; }





static void free_pattern(Pattern *p)
{
    if (!p || !p->t) return;
    for (int i = 0; i < p->count; i++) {
        free(p->t[i].open_str);
        free(p->t[i].close_str);
        free(p->t[i].literal_str);

        if (p->t[i].alts) {
            for (int j = 0; j < p->t[i].alt_count; j++)
                free(p->t[i].alts[j]);
            free(p->t[i].alts);
        }
        if (p->t[i].sub_pattern) {
            free_pattern(p->t[i].sub_pattern);
            free(p->t[i].sub_pattern);
        }
        if (p->t[i].alt_patterns) {
            for (int j = 0; j < p->t[i].alt_pattern_count; j++) {
                if (p->t[i].alt_patterns[j]) {
                    free_pattern(p->t[i].alt_patterns[j]);
                    free(p->t[i].alt_patterns[j]);
                }
            }
            free(p->t[i].alt_patterns);
        }

    }
    free(p->t); p->t = 
# 282 "src/papagaio.c" 3
                      ((void *)0)
# 282 "src/papagaio.c"
                          ; p->count = 0; p->cap = 0;
}

static void free_match(Match *m)
{
    if (!m) return;
    if (m->cap) {
        for (int i = 0; i < m->count; i++)
            if (m->cap[i].owned) { free(m->cap[i].owned); m->cap[i].owned = 
# 290 "src/papagaio.c" 3
                                                                           ((void *)0)
# 290 "src/papagaio.c"
                                                                               ; }
        free(m->cap); m->cap = 
# 291 "src/papagaio.c" 3
                              ((void *)0)
# 291 "src/papagaio.c"
                                  ;
    }

    m->count = 0; m->cap_size = 0;
}

static void ensure_cap(Match *m)
{
    if (m->count >= m->cap_size) {
        m->cap_size <<= 1;
        m->cap = (Capture *)realloc(m->cap, sizeof(Capture) * m->cap_size);
    }
}

static void free_pairs(PatternPair *p, int n)
{
    if (!p) return;
    for (int i = 0; i < n; i++) { free(p[i].m); free(p[i].r); }
    free(p);
}







static int extract_block(const char *src, int pos,
                          StrView o, StrView c, StrView *out);

static char *unescape_delim(StrView v, size_t *out_len)
{
    StrBuf out; sb_init(&out);
    for (size_t i = 0; i < v.len; i++) {
        char c = v.ptr[i];
        if (c == '\\' && i + 1 < v.len) {
            char n = v.ptr[i+1];
            if (n == '{' || n == '}' || n == '[' || n == ']' || n == '(' || n == ')' || n == '\\') {
                sb_append_char(&out, n); i++; continue;
            }
        }
        sb_append_char(&out, c);
    }
    if (out_len) *out_len = out.len;
    return out.data;
}







static int extract_block(const char *src, int pos,
                          StrView o, StrView c, StrView *out)
{
    if (o.len == c.len && o.len > 0 && memcmp(o.ptr, c.ptr, o.len) == 0) {
        if (!sv_pfx(src + pos, o)) return pos;
        pos += (int)o.len;
        int start = pos;
        while (src[pos]) {
            if (sv_pfx(src + pos, c)) {
                out->ptr = src + start; out->len = (size_t)(pos - start);
                return pos + (int)c.len;
            }
            pos++;
        }
        out->ptr = src + start; out->len = strlen(src + start);
        return (int)strlen(src);
    }
    if (!sv_pfx(src + pos, o)) return pos;
    pos += (int)o.len;
    int start = pos, depth = 1;
    while (src[pos] && depth) {
        if (sv_pfx(src + pos, o)) { depth++; pos += (int)o.len; }
        else if (sv_pfx(src + pos, c)) {
            if (!--depth) {
                out->ptr = src + start; out->len = (size_t)(pos - start);
                return pos + (int)c.len;
            }
            pos += (int)c.len;
        } else pos++;
    }
    out->ptr = src + start; out->len = strlen(src + start);
    return (int)strlen(src);
}







static char *extract_nested(const char *src, const Symbols *sym,
                              PatternPair **out_pairs, int *out_count)
{
    if (out_pairs) *out_pairs = 
# 387 "src/papagaio.c" 3
                               ((void *)0)
# 387 "src/papagaio.c"
                                   ;
    if (out_count) *out_count = 0;
    if (!src || !sym || !sym->pattern) return 
# 389 "src/papagaio.c" 3
                                             ((void *)0)
# 389 "src/papagaio.c"
                                                 ;

    int collect = out_pairs && out_count;
    PatternPair *pairs = 
# 392 "src/papagaio.c" 3
                        ((void *)0)
# 392 "src/papagaio.c"
                            ;
    int pc = 0, pcap = 0;

    StrBuf out; sb_init(&out);
    size_t sl = strlen(sym->sigil), pl = strlen(sym->pattern);
    StrView o = { sym->open, strlen(sym->open) };
    StrView c = { sym->close, strlen(sym->close) };
    size_t len = strlen(src), i = 0;

    while (i < len) {
        if (sl > 0 && i + sl + pl <= len &&
            memcmp(src + i, sym->sigil, sl) == 0 &&
            memcmp(src + i + sl, sym->pattern, pl) == 0) {

            size_t j = i + sl + pl;
            while (j < len && 
# 407 "src/papagaio.c" 3
                             ((*__ctype_b_loc ())[(int) ((
# 407 "src/papagaio.c"
                             (unsigned char)src[j]
# 407 "src/papagaio.c" 3
                             ))] & (unsigned short int) _ISspace)
# 407 "src/papagaio.c"
                                                           ) j++;

            if (j < len && sv_pfx(src + j, o)) {
                StrView mp;
                int next = extract_block(src, (int)j, o, c, &mp);
                size_t k = (size_t)next;
                while (k < len && 
# 413 "src/papagaio.c" 3
                                 ((*__ctype_b_loc ())[(int) ((
# 413 "src/papagaio.c"
                                 (unsigned char)src[k]
# 413 "src/papagaio.c" 3
                                 ))] & (unsigned short int) _ISspace)
# 413 "src/papagaio.c"
                                                               ) k++;

                if (k < len && sv_pfx(src + k, o)) {
                    StrView rp;
                    int end = extract_block(src, (int)k, o, c, &rp);
                    StrView mt = trim_sv(mp), rt = trim_sv(rp);

                    if (collect) {
                        if (pc >= pcap) {
                            pcap = pcap ? pcap * 2 : 8;
                            pairs = (PatternPair *)realloc(pairs, sizeof(PatternPair) * pcap);
                        }
                        pairs[pc].m = (char *)malloc(mt.len + 1);
                        pairs[pc].r = (char *)malloc(rt.len + 1);
                        if (pairs[pc].m && pairs[pc].r) {
                            memcpy(pairs[pc].m, mt.ptr, mt.len); pairs[pc].m[mt.len] = '\0';
                            memcpy(pairs[pc].r, rt.ptr, rt.len); pairs[pc].r[rt.len] = '\0';
                            pc++;
                        } else { free(pairs[pc].m); free(pairs[pc].r); }
                    }
                    i = (size_t)end; continue;
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }
    if (out_pairs) *out_pairs = pairs;
    if (out_count) *out_count = pc;
    return out.data;
}
# 454 "src/papagaio.c"
static void parse_pattern_ex(const char *pat, Pattern *p, const Symbols *sym);
static int match_pattern(Papagaio *ctx, const char *src, int src_len,
                           Pattern *p, int start, Match *m);
static char *apply_replacement_ex(const char *rep, const Match *m,
                                   const Symbols *sym);
static char *resolve_preprocessor(Papagaio *ctx, const char *src, Symbols *sym);
static char *dispatch_commands(Papagaio *ctx, const char *src, const Symbols *sym);





static void parse_pattern_ex(const char *pat, Pattern *p, const Symbols *sym)
{
    int n = (int)strlen(pat);
    p->cap = 16; p->count = 0;
    p->t = (PapToken *)malloc(sizeof(PapToken) * p->cap);
    p->sym = *sym;

    int sl = (int)strlen(sym->sigil);
    int ol = (int)strlen(sym->open);
    int cl = (int)strlen(sym->close);
    int i = 0;

    while (i < n) {
        if (p->count == p->cap) {
            p->cap <<= 1;
            p->t = (PapToken *)realloc(p->t, sizeof(PapToken) * p->cap);
        }
        PapToken *t = &p->t[p->count];
        memset(t, 0, sizeof(*t));


        if (
# 487 "src/papagaio.c" 3
           ((*__ctype_b_loc ())[(int) ((
# 487 "src/papagaio.c"
           (unsigned char)pat[i]
# 487 "src/papagaio.c" 3
           ))] & (unsigned short int) _ISspace)
# 487 "src/papagaio.c"
                                         ) {
            while (i < n && 
# 488 "src/papagaio.c" 3
                           ((*__ctype_b_loc ())[(int) ((
# 488 "src/papagaio.c"
                           (unsigned char)pat[i]
# 488 "src/papagaio.c" 3
                           ))] & (unsigned short int) _ISspace)
# 488 "src/papagaio.c"
                                                         ) i++;
            t->type = TOK_WS; p->count++; continue;
        }


        if (str_pfx(pat + i, sym->sigil)) {
            i += sl;
            int v = i;
            while (i < n && (
# 496 "src/papagaio.c" 3
                            ((*__ctype_b_loc ())[(int) ((
# 496 "src/papagaio.c"
                            (unsigned char)pat[i]
# 496 "src/papagaio.c" 3
                            ))] & (unsigned short int) _ISalnum) 
# 496 "src/papagaio.c"
                                                           || pat[i] == '_')) i++;
            size_t vlen = (size_t)(i - v);

            if (vlen == 0) {
                t->type = TOK_LITERAL;
                t->value = (StrView){ sym->sigil, (size_t)sl };
                p->count++; continue;
            }

            t->var = (StrView){ pat + v, vlen };


            if (i + sl <= n && memcmp(pat + i, sym->sigil, sl) == 0) {
                i += sl;
                int ms = i;
                while (i < n && (
# 511 "src/papagaio.c" 3
                                ((*__ctype_b_loc ())[(int) ((
# 511 "src/papagaio.c"
                                (unsigned char)pat[i]
# 511 "src/papagaio.c" 3
                                ))] & (unsigned short int) _ISalnum) 
# 511 "src/papagaio.c"
                                                               || pat[i] == '_')) i++;
                StrView mod = { pat + ms, (size_t)(i - ms) };

                if (sv_eq(mod, (StrView){"int", 3 })) t->modifier = MOD_INT;
                else if (sv_eq(mod, (StrView){"float", 5 })) t->modifier = MOD_FLOAT;
                else if (sv_eq(mod, (StrView){"number", 6 })) t->modifier = MOD_NUMBER;
                else if (sv_eq(mod, (StrView){"upper", 5 })) t->modifier = MOD_UPPER;
                else if (sv_eq(mod, (StrView){"lower", 5 })) t->modifier = MOD_LOWER;
                else if (sv_eq(mod, (StrView){"capitalized", 11})) t->modifier = MOD_CAPITALIZED;
                else if (sv_eq(mod, (StrView){"word", 4 })) t->modifier = MOD_WORD;
                else if (sv_eq(mod, (StrView){"identifier", 10})) t->modifier = MOD_IDENTIFIER;
                else if (sv_eq(mod, (StrView){"hex", 3 })) t->modifier = MOD_HEX;
                else if (sv_eq(mod, (StrView){"path", 4 })) t->modifier = MOD_PATH;
                else if (sv_eq(mod, (StrView){"binary", 6 })) t->modifier = MOD_BINARY;
                else if (sv_eq(mod, (StrView){"percent", 7 })) t->modifier = MOD_PERCENT;

                else if (sv_eq(mod, (StrView){ sym->block, strlen(sym->block) })) {
                    t->type = TOK_BLOCK;

                    while (i < n && 
# 530 "src/papagaio.c" 3
                                   ((*__ctype_b_loc ())[(int) ((
# 530 "src/papagaio.c"
                                   (unsigned char)pat[i]
# 530 "src/papagaio.c" 3
                                   ))] & (unsigned short int) _ISspace)
# 530 "src/papagaio.c"
                                                                 ) i++;
                    if (i < n && str_pfx(pat + i, sym->open)) {
                        i += ol;
                        int o = i;
                        while (i < n && !str_pfx(pat + i, sym->close)) i++;
                        StrView raw_open = { pat + o, (size_t)(i - o) };
                        if (str_pfx(pat + i, sym->close)) i += cl;

                        StrView raw_close = { sym->close, strlen(sym->close) };
                        if (str_pfx(pat + i, sym->open)) {
                            i += ol; int c = i;
                            while (i < n && !str_pfx(pat + i, sym->close)) i++;
                            raw_close = (StrView){ pat + c, (size_t)(i - c) };
                            if (str_pfx(pat + i, sym->close)) i += cl;
                        }

                        StrView ot = trim_sv(raw_open); size_t olen = 0;
                        char *ou = unescape_delim(ot, &olen);
                        if (olen == 0) { free(ou); t->open = (StrView){ sym->open, strlen(sym->open) }; }
                        else { t->open_str = ou; t->open = (StrView){ t->open_str, olen }; }

                        StrView ct2 = trim_sv(raw_close); size_t clen = 0;
                        char *cu = unescape_delim(ct2, &clen);
                        if (clen == 0) { free(cu); t->close = (StrView){ sym->close, strlen(sym->close) }; }
                        else { t->close_str = cu; t->close = (StrView){ t->close_str, clen }; }
                    }
                }
                else if (sv_eq(mod, (StrView){"aliases", 7 })) {
                    t->modifier = MOD_ALIASES;
                    int acap = 4;
                    t->alts = (char **)malloc(sizeof(char *) * acap);
                    t->alt_count = 0;
                    t->alt_patterns = (Pattern **)malloc(sizeof(Pattern *) * acap);
                    t->alt_pattern_count = 0;

                    StrView so = { sym->open, strlen(sym->open) };
                    StrView sc = { sym->close, strlen(sym->close) };

                    while (i < n) {
                        while (i < n && 
# 569 "src/papagaio.c" 3
                                       ((*__ctype_b_loc ())[(int) ((
# 569 "src/papagaio.c"
                                       (unsigned char)pat[i]
# 569 "src/papagaio.c" 3
                                       ))] & (unsigned short int) _ISspace)
# 569 "src/papagaio.c"
                                                                     ) i++;
                        if (i < n && sv_pfx(pat + i, so)) {
                            StrView blk;
                            i = (size_t)extract_block(pat, (int)i, so, sc, &blk);
                            if (t->alt_count >= acap) {
                                acap *= 2;
                                t->alts = (char **)realloc(t->alts, sizeof(char *) * acap);
                                t->alt_patterns = (Pattern **)realloc(t->alt_patterns, sizeof(Pattern *) * acap);
                            }
                            char *valt = (char*)malloc(blk.len + 1);
                            memcpy(valt, blk.ptr, blk.len); valt[blk.len] = '\0';
                            t->alts[t->alt_count++] = valt;

                            Pattern *sp = (Pattern *)malloc(sizeof(Pattern));
                            memset(sp, 0, sizeof(Pattern));
                            parse_pattern_ex(valt, sp, sym);
                            t->alt_patterns[t->alt_pattern_count++] = sp;
                        } else break;
                    }
                }
                else if (sv_eq(mod, (StrView){"group", 5 }) ||
                         sv_eq(mod, (StrView){"optional", 8 }) ||
                         sv_eq(mod, (StrView){"starts", 6 }) ||
                         sv_eq(mod, (StrView){"ends", 4 }) ||
                         sv_eq(mod, (StrView){"prefix", 6 }) ||
                         sv_eq(mod, (StrView){"suffix", 6 }) ||
                         sv_eq(mod, (StrView){"infix", 5 }) ||
                         sv_eq(mod, (StrView){"includes", 8 })) {
                    if (sv_eq(mod, (StrView){"group", 5}) ||
                        sv_eq(mod, (StrView){"optional", 8})) {
                        t->modifier = MOD_GROUP;
                    } else if (sv_eq(mod, (StrView){"starts", 6})) {
                        t->modifier = MOD_STARTS;
                    } else if (sv_eq(mod, (StrView){"ends", 4})) {
                        t->modifier = MOD_ENDS;
                    } else if (sv_eq(mod, (StrView){"prefix", 6})) {
                        t->modifier = MOD_PREFIX;
                    } else if (sv_eq(mod, (StrView){"suffix", 6})) {
                        t->modifier = MOD_SUFFIX;
                    } else if (sv_eq(mod, (StrView){"infix", 5})) {
                        t->modifier = MOD_INFIX;
                    } else {
                        t->modifier = MOD_INCLUDES;
                    }

                    while (i < n && 
# 614 "src/papagaio.c" 3
                                   ((*__ctype_b_loc ())[(int) ((
# 614 "src/papagaio.c"
                                   (unsigned char)pat[i]
# 614 "src/papagaio.c" 3
                                   ))] & (unsigned short int) _ISspace)
# 614 "src/papagaio.c"
                                                                 ) i++;
                    if (i < n && str_pfx(pat + i, sym->open)) {
                        StrView blk;
                        StrView so = { sym->open, (size_t)ol };
                        StrView sc = { sym->close, (size_t)cl };
                        int next = extract_block(pat, i, so, sc, &blk);
                        StrView phrase = trim_sv(blk);
                        t->literal_str = (char *)malloc(phrase.len + 1);
                        if (t->literal_str) {
                            memcpy(t->literal_str, phrase.ptr, phrase.len);
                            t->literal_str[phrase.len] = '\0';
                            t->value.ptr = t->literal_str;
                            t->value.len = phrase.len;
                        }

                        int has_sigil = 0;
                        for (size_t si = 0; si + (size_t)sl <= phrase.len; si++) {
                            if (memcmp(phrase.ptr + si, sym->sigil, sl) == 0) { has_sigil = 1; break; }
                        }
                        if (has_sigil) {
                            char *sub_str = (char *)malloc(phrase.len + 1);
                            memcpy(sub_str, phrase.ptr, phrase.len);
                            sub_str[phrase.len] = '\0';
                            t->sub_pattern = (Pattern *)malloc(sizeof(Pattern));
                            memset(t->sub_pattern, 0, sizeof(Pattern));
                            parse_pattern_ex(sub_str, t->sub_pattern, sym);
                            free(sub_str);
                        }
                        i = next;
                    }
                }
            }

            if (i < n && str_pfx(pat + i, sym->optional)) { t->optional = 1; i += (int)strlen(sym->optional); }

            if (i < n && str_pfx(pat + i, sym->sigil)) {
                size_t sl2 = strlen(sym->sigil);
                size_t j2 = i + sl2;

                if (j2 >= (size_t)n || (!
# 653 "src/papagaio.c" 3
                                        ((*__ctype_b_loc ())[(int) ((
# 653 "src/papagaio.c"
                                        (unsigned char)pat[j2]
# 653 "src/papagaio.c" 3
                                        ))] & (unsigned short int) _ISalnum) 
# 653 "src/papagaio.c"
                                                                        && pat[j2] != '_')) {
                    t->ws_consume = 1;
                    i += (int)sl2;
                }
            }
            if (t->type != TOK_BLOCK) {
                t->type = TOK_VAR;
            }
            p->count++; continue;
        }


        int l = i;
        while (i < n && !
# 666 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 666 "src/papagaio.c"
                        (unsigned char)pat[i]
# 666 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISspace) 
# 666 "src/papagaio.c"
                                                       && !str_pfx(pat + i, sym->sigil) && !str_pfx(pat + i, sym->optional)) i++;
        t->type = TOK_LITERAL;
        t->value = (StrView){ pat + l, (size_t)(i - l) };
        p->count++;
        if (i < n && str_pfx(pat + i, sym->optional)) { p->t[p->count-1].optional = 1; i += (int)strlen(sym->optional); }

        if (i < n && str_pfx(pat + i, sym->sigil)) {
            size_t sl2 = strlen(sym->sigil);
            size_t j2 = i + sl2;
            if (j2 >= (size_t)n || (!
# 675 "src/papagaio.c" 3
                                    ((*__ctype_b_loc ())[(int) ((
# 675 "src/papagaio.c"
                                    (unsigned char)pat[j2]
# 675 "src/papagaio.c" 3
                                    ))] & (unsigned short int) _ISalnum) 
# 675 "src/papagaio.c"
                                                                    && pat[j2] != '_')) {
                p->t[p->count-1].ws_consume = 1;
                i += (int)sl2;
            }
        }
    }


    for (int a = 0; a < p->count; a++) {
        p->t[a].next_sig = -1;
        for (int b = a + 1; b < p->count; b++)
            if (p->t[b].type != TOK_WS) { p->t[a].next_sig = b; break; }
        int all = 1;
        for (int b = a + 1; b < p->count; b++) {
            if (p->t[b].type == TOK_WS) continue;
            if (!p->t[b].optional) { all = 0; break; }
        }
        p->t[a].all_opt = (unsigned)all;
    }


    for (int a = 0; a < p->count; a++) {
        if (p->t[a].type != TOK_WS) continue;
        int ns = p->t[a].next_sig;
        if (ns >= 0 && p->t[ns].optional) { p->t[a].optional = 1; continue; }
        for (int b = a - 1; b >= 0; b--) {
            if (p->t[b].type == TOK_WS) continue;
            if (p->t[b].optional) p->t[a].optional = 1;

            if (p->t[b].ws_consume) p->t[a].optional = 1;
            break;
        }
    }
}
# 731 "src/papagaio.c"
static int sub_pattern_matches_at(Papagaio *ctx, const char *src, int src_len, Pattern *sp, int at)
{
    Match sub_m; sub_m.ctx = ctx;
    if (match_pattern(ctx, src, src_len, sp, at, &sub_m)) {
        int end = sub_m.end;
        free_match(&sub_m);
        return end;
    }
    return -1;
}


static int match_pattern(Papagaio *ctx, const char *src, int src_len,
                          Pattern *p, int start, Match *m)
{
    m->ctx = ctx;
    m->cap_size = 16; m->count = 0;
    m->cap = (Capture *)malloc(sizeof(Capture) * m->cap_size);
    m->start = start; m->src = src;


    int pos = start;

    for (int i = 0; i < p->count; i++) {
        PapToken *t = &p->t[i];
        PapToken *nx = (t->next_sig >= 0) ? &p->t[t->next_sig] : 
# 756 "src/papagaio.c" 3
                                                                ((void *)0)
# 756 "src/papagaio.c"
                                                                    ;

        if (t->type == TOK_WS) {
            if (!
# 759 "src/papagaio.c" 3
                ((*__ctype_b_loc ())[(int) ((
# 759 "src/papagaio.c"
                (unsigned char)src[pos]
# 759 "src/papagaio.c" 3
                ))] & (unsigned short int) _ISspace)
# 759 "src/papagaio.c"
                                                ) {
                if (!t->all_opt && !t->optional) goto fail;
                continue;
            }
            skip_ws(src, &pos); continue;
        }

        if (t->type == TOK_LITERAL) {
            if (!sv_pfx(src + pos, t->value)) {
                if (t->optional) continue;
                goto fail;
            }
            pos += (int)t->value.len;
            if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
            continue;
        }

        if (t->type == TOK_VAR) {


            if (i == 0 || p->t[i-1].type != TOK_WS) {
                while (src[pos] == ' ' || src[pos] == '\t') pos++;
            }
            int s = pos;
            if (t->modifier == MOD_ALIASES) {
                int hit = 0;

                for (int ai = 0; ai < t->alt_count; ai++) {
                    size_t al = strlen(t->alts[ai]);
                    if ((size_t)(src_len - pos) >= al &&
                        memcmp(src + pos, t->alts[ai], al) == 0) {
                        ensure_cap(m);
                        m->cap[m->count++] = (Capture){ t->var, { src + pos, al }, 
# 791 "src/papagaio.c" 3
                                                                                  ((void *)0) 
# 791 "src/papagaio.c"
                                                                                       };
                        pos += (int)al; hit = 1; break;
                    }
                }

                if (!hit) {
                    for (int ai = 0; ai < t->alt_pattern_count; ai++) {
                        if (!t->alt_patterns[ai]) continue;
                        Match sub_m; sub_m.ctx = ctx;
                        if (match_pattern(ctx, src, src_len, t->alt_patterns[ai], pos, &sub_m)) {

                            for (int ci = 0; ci < sub_m.count; ci++) {
                                ensure_cap(m);
                                m->cap[m->count++] = sub_m.cap[ci];
                            }

                            ensure_cap(m);
                            m->cap[m->count++] = (Capture){ t->var, { src + pos, (size_t)(sub_m.end - pos) }, 
# 808 "src/papagaio.c" 3
                                                                                                             ((void *)0) 
# 808 "src/papagaio.c"
                                                                                                                  };
                            pos = sub_m.end;
                            hit = 1;

                            free(sub_m.cap);
                            break;
                        }
                    }
                }
                if (!hit) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 819 "src/papagaio.c" 3
                                                                                     ((void *)0) 
# 819 "src/papagaio.c"
                                                                                          };
                    continue;
                }
                continue;
            }
            if (t->modifier == MOD_GROUP) {
                if (t->sub_pattern) {
                    Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
                    if (!match_pattern(ctx, src, src_len, t->sub_pattern, pos, &sub_m)) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 829 "src/papagaio.c" 3
                                                                                         ((void *)0) 
# 829 "src/papagaio.c"
                                                                                              };
                        continue;
                    }
                    for (int ci = 0; ci < sub_m.count; ci++) {
                        ensure_cap(m);
                        m->cap[m->count++] = sub_m.cap[ci];
                    }
                    pos = sub_m.end;
                    free(sub_m.cap);
                } else {
                    if ((size_t)(src_len-pos) >= t->value.len && t->value.len > 0 &&
                        memcmp(src+pos, t->value.ptr, t->value.len) == 0)
                        pos += (int)t->value.len;
                    else if (!t->optional) goto fail;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, 
# 845 "src/papagaio.c" 3
                                                                                   ((void *)0) 
# 845 "src/papagaio.c"
                                                                                        };
                continue;
            }
            if (t->modifier == MOD_STARTS || t->modifier == MOD_PREFIX) {
                if (t->sub_pattern) {
                    Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
                    if (!match_pattern(ctx, src, src_len, t->sub_pattern, pos, &sub_m)) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 853 "src/papagaio.c" 3
                                                                                         ((void *)0) 
# 853 "src/papagaio.c"
                                                                                              };
                        continue;
                    }
                    for (int ci = 0; ci < sub_m.count; ci++) {
                        ensure_cap(m);
                        m->cap[m->count++] = sub_m.cap[ci];
                    }
                    free(sub_m.cap);
                } else {
                    if ((size_t)(src_len-pos) < t->value.len ||
                        memcmp(src+pos, t->value.ptr, t->value.len) != 0) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 865 "src/papagaio.c" 3
                                                                                         ((void *)0) 
# 865 "src/papagaio.c"
                                                                                              };
                        continue;
                    }
                }
            }

            if (nx && (nx->type == TOK_LITERAL || nx->type == TOK_BLOCK)) {
                while (src[pos]) {
                    if (src[pos] == '\n') break;
                    if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                    if (nx->type == TOK_BLOCK &&
                        sv_pfx(src+pos, nx->open)) break;
                    if (!( !(t->modifier == MOD_INT && !(
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISdigit) 
# 877 "src/papagaio.c"
                        || ((pos)==(s) && (src[pos])=='-'))) && !(t->modifier == MOD_FLOAT && !(
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISdigit) 
# 877 "src/papagaio.c"
                        || (src[pos])=='.' || ((pos)==(s) && (src[pos])=='-'))) && !(t->modifier == MOD_NUMBER && !(
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISdigit) 
# 877 "src/papagaio.c"
                        || (src[pos])=='.' || ((pos)==(s) && (src[pos])=='-'))) && !(t->modifier == MOD_UPPER && !
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISupper)
# 877 "src/papagaio.c"
                        ) && !(t->modifier == MOD_LOWER && !
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISlower)
# 877 "src/papagaio.c"
                        ) && !(t->modifier == MOD_CAPITALIZED && (((pos)==(s)) ? !
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISupper) 
# 877 "src/papagaio.c"
                        : !
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISlower)
# 877 "src/papagaio.c"
                        )) && !(t->modifier == MOD_WORD && !
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISalpha)
# 877 "src/papagaio.c"
                        ) && !(t->modifier == MOD_IDENTIFIER && (!(
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISalnum) 
# 877 "src/papagaio.c"
                        || (src[pos])=='_') || ((pos)==(s) && 
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISdigit)
# 877 "src/papagaio.c"
                        ))) && !(t->modifier == MOD_HEX && (!
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISxdigit) 
# 877 "src/papagaio.c"
                        && !((src[pos])=='x' && (pos)>(s) && src[(pos)-1]=='0') && !((src[pos])=='X' && (pos)>(s) && src[(pos)-1]=='0'))) && !(t->modifier == MOD_PATH && (
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISspace) 
# 877 "src/papagaio.c"
                        || (src[pos])=='\n')) && !(t->modifier == MOD_BINARY && ((src[pos])!='0' && (src[pos])!='1' && (src[pos])!='b' && (src[pos])!='B')) && !(t->modifier == MOD_PERCENT && !(
# 877 "src/papagaio.c" 3
                        ((*__ctype_b_loc ())[(int) ((
# 877 "src/papagaio.c"
                        (unsigned char)(src[pos])
# 877 "src/papagaio.c" 3
                        ))] & (unsigned short int) _ISdigit) 
# 877 "src/papagaio.c"
                        || (src[pos])=='.' || (src[pos])=='%' || ((pos)==(s) && (src[pos])=='-'))))) break;
                    pos++;
                    if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && !t->sub_pattern &&
                        t->value.len > 0 && (size_t)(pos-s) >= t->value.len &&
                        memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) == 0) break;
                    if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && t->sub_pattern) {

                        for (int bp = s; bp < pos; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                            if (me == pos) { goto ends_break_1; }
                        }
                    }
                }
                ends_break_1:

                {
                int end = pos;
                while (end > s && 
# 894 "src/papagaio.c" 3
                                 ((*__ctype_b_loc ())[(int) ((
# 894 "src/papagaio.c"
                                 (unsigned char)src[end-1]
# 894 "src/papagaio.c" 3
                                 ))] & (unsigned short int) _ISspace)
# 894 "src/papagaio.c"
                                                                   ) end--;
                size_t clen = (size_t)(end - s);

                int failed = 0;
                if (t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) {
                    if (t->sub_pattern) {

                        int found_sp = 0;
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                            if (me == end) { found_sp = 1; break; }
                        }
                        if (!found_sp) failed = 1;
                        else if (t->modifier == MOD_SUFFIX) {

                            int earliest = end;
                            for (int bp = s; bp < end; bp++) {
                                int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                                if (me == end && bp > s) { earliest = bp; break; }
                            }
                            if (earliest <= s) failed = 1;
                        }
                    } else {
                        if (t->value.len > 0 && (clen < t->value.len || memcmp(src+end-t->value.len, t->value.ptr, t->value.len) != 0))
                            failed = 1;
                        else if (t->modifier == MOD_SUFFIX && clen <= t->value.len)
                            failed = 1;
                    }
                } else if (t->modifier == MOD_PREFIX) {
                    if (t->sub_pattern) {
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, s);
                        if (me < 0 || me >= end) failed = 1;
                    } else {
                        if (clen <= t->value.len) failed = 1;
                    }
                } else if (t->modifier == MOD_INFIX) {
                    int found = 0;
                    if (t->sub_pattern) {
                        for (int bp = s + 1; bp < end; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                            if (me > 0 && me < end && bp > s) { found = 1; break; }
                        }
                    } else {
                        if (clen >= t->value.len + 2 &&
                            memcmp(src + s, t->value.ptr, t->value.len) != 0 &&
                            memcmp(src + end - t->value.len, t->value.ptr, t->value.len) != 0)
                        {
                            for (size_t j = 1; j <= clen - t->value.len - 1; j++) {
                                if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                    found = 1; break;
                                }
                            }
                        }
                    }
                    if (!found) failed = 1;
                } else if (t->modifier == MOD_INCLUDES) {
                    int found = 0;
                    if (t->sub_pattern) {
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                            if (me > 0 && me <= end) { found = 1; break; }
                        }
                    } else {
                        if (clen >= t->value.len) {
                            for (size_t j = 0; j <= clen - t->value.len; j++) {
                                if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                    found = 1; break;
                                }
                            }
                        }
                    }
                    if (!found) failed = 1;
                }

                if (failed) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 970 "src/papagaio.c" 3
                                                                                     ((void *)0) 
# 970 "src/papagaio.c"
                                                                                          };
                    pos = s; continue;
                }
                if (end == s) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 975 "src/papagaio.c" 3
                                                                                     ((void *)0) 
# 975 "src/papagaio.c"
                                                                                          };
                    pos = s; continue;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(end-s) }, 
# 979 "src/papagaio.c" 3
                                                                                   ((void *)0) 
# 979 "src/papagaio.c"
                                                                                        };
                pos = end;
                if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
                continue;
                }
            }

            while (src[pos]) {
                if (nx && 
# 987 "src/papagaio.c" 3
                         ((*__ctype_b_loc ())[(int) ((
# 987 "src/papagaio.c"
                         (unsigned char)src[pos]
# 987 "src/papagaio.c" 3
                         ))] & (unsigned short int) _ISspace)
# 987 "src/papagaio.c"
                                                         ) break;
                if (nx) {
                    if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                    if (nx->type == TOK_BLOCK &&
                        sv_pfx(src+pos, nx->open)) break;
                } else if (src[pos] == '\n') break;
                if (!( !(t->modifier == MOD_INT && !(
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISdigit) 
# 993 "src/papagaio.c"
                    || ((pos)==(s) && (src[pos])=='-'))) && !(t->modifier == MOD_FLOAT && !(
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISdigit) 
# 993 "src/papagaio.c"
                    || (src[pos])=='.' || ((pos)==(s) && (src[pos])=='-'))) && !(t->modifier == MOD_NUMBER && !(
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISdigit) 
# 993 "src/papagaio.c"
                    || (src[pos])=='.' || ((pos)==(s) && (src[pos])=='-'))) && !(t->modifier == MOD_UPPER && !
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISupper)
# 993 "src/papagaio.c"
                    ) && !(t->modifier == MOD_LOWER && !
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISlower)
# 993 "src/papagaio.c"
                    ) && !(t->modifier == MOD_CAPITALIZED && (((pos)==(s)) ? !
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISupper) 
# 993 "src/papagaio.c"
                    : !
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISlower)
# 993 "src/papagaio.c"
                    )) && !(t->modifier == MOD_WORD && !
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISalpha)
# 993 "src/papagaio.c"
                    ) && !(t->modifier == MOD_IDENTIFIER && (!(
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISalnum) 
# 993 "src/papagaio.c"
                    || (src[pos])=='_') || ((pos)==(s) && 
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISdigit)
# 993 "src/papagaio.c"
                    ))) && !(t->modifier == MOD_HEX && (!
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISxdigit) 
# 993 "src/papagaio.c"
                    && !((src[pos])=='x' && (pos)>(s) && src[(pos)-1]=='0') && !((src[pos])=='X' && (pos)>(s) && src[(pos)-1]=='0'))) && !(t->modifier == MOD_PATH && (
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISspace) 
# 993 "src/papagaio.c"
                    || (src[pos])=='\n')) && !(t->modifier == MOD_BINARY && ((src[pos])!='0' && (src[pos])!='1' && (src[pos])!='b' && (src[pos])!='B')) && !(t->modifier == MOD_PERCENT && !(
# 993 "src/papagaio.c" 3
                    ((*__ctype_b_loc ())[(int) ((
# 993 "src/papagaio.c"
                    (unsigned char)(src[pos])
# 993 "src/papagaio.c" 3
                    ))] & (unsigned short int) _ISdigit) 
# 993 "src/papagaio.c"
                    || (src[pos])=='.' || (src[pos])=='%' || ((pos)==(s) && (src[pos])=='-'))))) break;
                pos++;
                if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && !t->sub_pattern &&
                    t->value.len > 0 && (size_t)(pos-s) >= t->value.len &&
                    memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) == 0) break;
                if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && t->sub_pattern) {
                    for (int bp = s; bp < pos; bp++) {
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                        if (me == pos) { goto ends_break_2; }
                    }
                }
            }
            ends_break_2:

            {
            int end = pos;
            while (end > s && 
# 1009 "src/papagaio.c" 3
                             ((*__ctype_b_loc ())[(int) ((
# 1009 "src/papagaio.c"
                             (unsigned char)src[end-1]
# 1009 "src/papagaio.c" 3
                             ))] & (unsigned short int) _ISspace)
# 1009 "src/papagaio.c"
                                                               ) end--;
            size_t clen = (size_t)(end - s);

            int failed = 0;
            if (t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) {
                if (t->sub_pattern) {
                    int found_sp = 0;
                    for (int bp = s; bp < end; bp++) {
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                        if (me == end) { found_sp = 1; break; }
                    }
                    if (!found_sp) failed = 1;
                    else if (t->modifier == MOD_SUFFIX) {
                        int earliest = end;
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                            if (me == end && bp > s) { earliest = bp; break; }
                        }
                        if (earliest <= s) failed = 1;
                    }
                } else {
                    if (t->value.len > 0 && (clen < t->value.len || memcmp(src+end-t->value.len, t->value.ptr, t->value.len) != 0))
                        failed = 1;
                    else if (t->modifier == MOD_SUFFIX && clen <= t->value.len)
                        failed = 1;
                }
            } else if (t->modifier == MOD_PREFIX) {
                if (t->sub_pattern) {
                    int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, s);
                    if (me < 0 || me >= end) failed = 1;
                } else {
                    if (clen <= t->value.len) failed = 1;
                }
            } else if (t->modifier == MOD_INFIX) {
                int found = 0;
                if (t->sub_pattern) {
                    for (int bp = s + 1; bp < end; bp++) {
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                        if (me > 0 && me < end && bp > s) { found = 1; break; }
                    }
                } else {
                    if (clen >= t->value.len + 2 &&
                        memcmp(src + s, t->value.ptr, t->value.len) != 0 &&
                        memcmp(src + end - t->value.len, t->value.ptr, t->value.len) != 0)
                    {
                        for (size_t j = 1; j <= clen - t->value.len - 1; j++) {
                            if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                found = 1; break;
                            }
                        }
                    }
                }
                if (!found) failed = 1;
            } else if (t->modifier == MOD_INCLUDES) {
                int found = 0;
                if (t->sub_pattern) {
                    for (int bp = s; bp < end; bp++) {
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                        if (me > 0 && me <= end) { found = 1; break; }
                    }
                } else {
                    if (clen >= t->value.len) {
                        for (size_t j = 0; j <= clen - t->value.len; j++) {
                            if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                found = 1; break;
                            }
                        }
                    }
                }
                if (!found) failed = 1;
            }

            if (failed) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 1083 "src/papagaio.c" 3
                                                                                 ((void *)0) 
# 1083 "src/papagaio.c"
                                                                                      };
                pos = s; continue;
            }
            if (pos == s) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 1088 "src/papagaio.c" 3
                                                                                 ((void *)0) 
# 1088 "src/papagaio.c"
                                                                                      };
                pos = s; continue;
            }
            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, 
# 1092 "src/papagaio.c" 3
                                                                               ((void *)0) 
# 1092 "src/papagaio.c"
                                                                                    };
            if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
            continue;
            }
        }

        if (t->type == TOK_BLOCK) {
            if (!sv_pfx(src+pos, t->open)) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, 
# 1101 "src/papagaio.c" 3
                                                                                 ((void *)0) 
# 1101 "src/papagaio.c"
                                                                                      }; continue;
            }
            StrView v;
            pos = extract_block(src, pos, t->open, t->close, &v);
            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, v, 
# 1106 "src/papagaio.c" 3
                                                      ((void *)0) 
# 1106 "src/papagaio.c"
                                                           };
            continue;
        }

    }

    m->end = pos; return 1;

fail:
    free_match(m);
    return 0;
}







static char *apply_replacement_ex(const char *rep, const Match *m,
                                   const Symbols *sym)
{
    StrBuf out; sb_init(&out);
    size_t n = strlen(rep), i = 0, sl = strlen(sym->sigil);

    while (i < n) {
        if (str_pfx(rep + i, sym->sigil)) {


            size_t ol = strlen(sym->open), cl = strlen(sym->close);
            if (ol > 0 && i + sl + ol <= n && str_pfx(rep + i + sl, sym->open)) {

                size_t ns = i + sl + ol;
                size_t ne = ns;
                int depth = 1;
                while (ne < n && depth > 0) {
                    if (cl > 0 && ne + cl <= n && str_pfx(rep + ne, sym->open) && strcmp(sym->open, sym->close) != 0) {
                        depth++; ne += ol;
                    } else if (cl > 0 && ne + cl <= n && str_pfx(rep + ne, sym->close)) {
                        depth--;
                        if (depth == 0) break;
                        ne += cl;
                    } else {
                        ne++;
                    }
                }
                if (depth == 0 && ne >= ns) {
                    StrView name = { rep + ns, ne - ns };

                    int valid_name = (name.len > 0);
                    for (size_t vi = 0; vi < name.len && valid_name; vi++) {
                        if (!
# 1157 "src/papagaio.c" 3
                            ((*__ctype_b_loc ())[(int) ((
# 1157 "src/papagaio.c"
                            (unsigned char)name.ptr[vi]
# 1157 "src/papagaio.c" 3
                            ))] & (unsigned short int) _ISalnum) 
# 1157 "src/papagaio.c"
                                                                 && name.ptr[vi] != '_') valid_name = 0;
                    }
                    if (valid_name) {
                        int found = 0;
                        for (int k = 0; k < m->count; k++) {
                            if (sv_eq(m->cap[k].name, name)) {
                                sb_append_n(&out, m->cap[k].value.ptr, m->cap[k].value.len);
                                found = 1; break;
                            }
                        }
                        if (!found) {
                            sb_append_n(&out, sym->sigil, sl);
                            sb_append_n(&out, sym->open, ol);
                            sb_append_n(&out, name.ptr, name.len);
                            sb_append_n(&out, sym->close, cl);
                        }
                        i = ne + cl;
                        continue;
                    }
                }
            }


            size_t ns = i + sl, ne = ns;
            while (ne < n && (
# 1181 "src/papagaio.c" 3
                             ((*__ctype_b_loc ())[(int) ((
# 1181 "src/papagaio.c"
                             (unsigned char)rep[ne]
# 1181 "src/papagaio.c" 3
                             ))] & (unsigned short int) _ISalnum) 
# 1181 "src/papagaio.c"
                                                             || rep[ne] == '_')) ne++;

            StrView name = { rep + ns, ne - ns };
            int found = 0;
            if (name.len > 0) {
                for (int k = 0; k < m->count; k++) {
                    if (sv_eq(m->cap[k].name, name)) {
                        sb_append_n(&out, m->cap[k].value.ptr, m->cap[k].value.len);
                        found = 1; break;
                    }
                }
            }
            if (!found) { sb_append_n(&out, sym->sigil, sl); sb_append_n(&out, name.ptr, name.len); }
            i = (ne == ns) ? i + sl : ne;
            continue;
        }
        sb_append_char(&out, rep[i++]);
    }
    return out.data;
}







static char *pap_var_lookup(Papagaio *ctx, const Symbols *sym,
                             const char *name, size_t nlen)
{
    if (!ctx || !name || nlen == 0) return 
# 1211 "src/papagaio.c" 3
                                          ((void *)0)
# 1211 "src/papagaio.c"
                                              ;
    size_t sl = strlen(sym->sigil);
    size_t ol = strlen(sym->open);
    size_t cl = strlen(sym->close);

    size_t pat_len = sl * 3 + nlen * 2 + 7 + ol + cl;
    char *pat = (char *)malloc(pat_len + 1);
    if (!pat) return 
# 1218 "src/papagaio.c" 3
                    ((void *)0)
# 1218 "src/papagaio.c"
                        ;
    char *p = pat;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, name, nlen); p += nlen;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, "aliases", 7); p += 7;
    memcpy(p, sym->open, ol); p += ol;
    memcpy(p, name, nlen); p += nlen;
    memcpy(p, sym->close, cl); p += cl;
    *p = '\0';
    char *result = 
# 1229 "src/papagaio.c" 3
                  ((void *)0)
# 1229 "src/papagaio.c"
                      ;
    Scope *s = ctx->current_scope;
    while (s) {
        for (int i = 0; i < s->rule_count; i++) {
            if (s->rules[i].m && strcmp(s->rules[i].m, pat) == 0) {
                result = strdup(s->rules[i].r);
                break;
            }
        }
        if (result) break;
        s = s->parent;
    }
    free(pat);
    return result;
}


static void push_scope(Papagaio *ctx) {
    Scope *s = (Scope *)malloc(sizeof(Scope));
    s->rules = 
# 1248 "src/papagaio.c" 3
              ((void *)0)
# 1248 "src/papagaio.c"
                  ;
    s->rule_count = 0;
    s->rule_cap = 0;
    s->parent = ctx->current_scope;
    ctx->current_scope = s;
}

static void clear_scope(Scope *s) {
    if (s && s->rules) {
        for (int i = 0; i < s->rule_count; i++) {
            free(s->rules[i].m);
            free(s->rules[i].r);
        }
        free(s->rules);
        s->rules = 
# 1262 "src/papagaio.c" 3
                  ((void *)0)
# 1262 "src/papagaio.c"
                      ;
        s->rule_count = 0;
        s->rule_cap = 0;
    }
}

static void pop_scope(Papagaio *ctx) {
    if (!ctx->current_scope) return;
    Scope *s = ctx->current_scope;
    clear_scope(s);
    ctx->current_scope = s->parent;
    free(s);
}


static void pap_var_update(Papagaio *ctx, const Symbols *sym,
                            const char *name, size_t nlen,
                            const char *new_value)
{
    if (!ctx || !name || nlen == 0 || !new_value) return;
    size_t sl = strlen(sym->sigil);
    size_t ol = strlen(sym->open);
    size_t cl = strlen(sym->close);
    size_t pat_len = sl * 3 + nlen * 2 + 7 + ol + cl;
    char *pat_str = (char *)malloc(pat_len + 1);
    if (!pat_str) return;
    char *p = pat_str;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, name, nlen); p += nlen;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, "aliases", 7); p += 7;
    memcpy(p, sym->open, ol); p += ol;
    memcpy(p, name, nlen); p += nlen;
    memcpy(p, sym->close, cl); p += cl;
    *p = '\0';
Scope *found_scope = 
# 1298 "src/papagaio.c" 3
                    ((void *)0)
# 1298 "src/papagaio.c"
                        ;
    int found_idx = -1;
    for (Scope *s = ctx->current_scope; s; s = s->parent) {
        for (int ri = 0; ri < s->rule_count; ri++) {
            if (s->rules[ri].m && strcmp(s->rules[ri].m, pat_str) == 0) {
                found_scope = s;
                found_idx = ri;
                break;
            }
        }
        if (found_scope) break;
    }

    if (found_scope) {
        free(found_scope->rules[found_idx].r);
        found_scope->rules[found_idx].r = strdup(new_value);
        free(pat_str);
    } else {
        if (ctx->current_scope->rule_count + 1 > ctx->current_scope->rule_cap) {
            ctx->current_scope->rule_cap = ctx->current_scope->rule_cap ? ctx->current_scope->rule_cap * 2 : 8;
            ctx->current_scope->rules = (PatternPair *)realloc(ctx->current_scope->rules,
                          sizeof(PatternPair) * ctx->current_scope->rule_cap);
        }
        ctx->current_scope->rules[ctx->current_scope->rule_count].m = pat_str;
        ctx->current_scope->rules[ctx->current_scope->rule_count].r = strdup(new_value);
        ctx->current_scope->rule_count++;
    }
}



static char **pap_list_split(const char *str, const char *sep,
                              size_t seplen, int *out_count)
{
    *out_count = 0;
    if (!str || str[0] == '\0') return 
# 1333 "src/papagaio.c" 3
                                      ((void *)0)
# 1333 "src/papagaio.c"
                                          ;
    if (seplen == 0) {

        int n = (int)strlen(str);
        char **arr = (char **)malloc(sizeof(char *) * (size_t)n);
        for (int ci = 0; ci < n; ci++) {
            arr[ci] = (char *)malloc(2);
            arr[ci][0] = str[ci];
            arr[ci][1] = '\0';
        }
        *out_count = n;
        return arr;
    }
    size_t len = strlen(str);
    int count = 1;
    for (size_t i = 0; i + seplen <= len; ) {
        if (memcmp(str + i, sep, seplen) == 0) { count++; i += seplen; }
        else i++;
    }
    char **arr = (char **)malloc(sizeof(char *) * count);
    int idx = 0;
    const char *start = str;
    for (size_t i = 0; i <= len; ) {
        int at_sep = (i + seplen <= len && memcmp(str + i, sep, seplen) == 0);
        if (i == len || at_sep) {
            size_t elen = (size_t)((str + i) - start);
            arr[idx] = (char *)malloc(elen + 1);
            memcpy(arr[idx], start, elen);
            arr[idx][elen] = '\0';
            idx++;
            if (i == len) break;
            i += seplen;
            start = str + i;
        } else i++;
    }
    *out_count = count;
    return arr;
}


static char *pap_list_join(char **parts, int count,
                            const char *sep, size_t seplen)
{
    if (!parts || count == 0) return strdup("");
    size_t total = 0;
    for (int i = 0; i < count; i++) total += strlen(parts[i]);
    if (count > 1) total += seplen * (size_t)(count - 1);
    char *result = (char *)malloc(total + 1);
    char *p = result;
    for (int i = 0; i < count; i++) {
        size_t l = strlen(parts[i]);
        memcpy(p, parts[i], l); p += l;
        if (i + 1 < count && seplen > 0) { memcpy(p, sep, seplen); p += seplen; }
    }
    *p = '\0';
    return result;
}


static void pap_list_free(char **parts, int count)
{
    if (!parts) return;
    for (int i = 0; i < count; i++) free(parts[i]);
    free(parts);
}



static int pap_list_normalize_idx(const char *idx_str, int count)
{
    if (!idx_str || !idx_str[0] || count == 0) return -1;
    int idx = atoi(idx_str);
    if (idx < 0) idx = count + idx;
    if (idx < 0 || idx >= count) return -1;
    return idx;
}


static char *pap_process_sv(Papagaio *ctx, StrView sv)
{
    char *tmp = (char *)malloc(sv.len + 1);
    memcpy(tmp, sv.ptr, sv.len); tmp[sv.len] = '\0';
    ctx->disable_patterns++;
    char *out = papagaio_process_text(ctx, tmp, sv.len);
    ctx->disable_patterns--;
    free(tmp);
    return out ? out : strdup("");
}





static void pap_list_op(Papagaio *ctx, const Symbols *sym,
                         StrBuf *sb_out,
                         const char *name, size_t nlen,
                         const char *sep_str, size_t seplen,
                         const char *op, size_t oplen,
                         StrView *raw_blocks, int block_count)
{

    char *var_val = pap_var_lookup(ctx, sym, name, nlen);
    if (!var_val) var_val = strdup("");


    int count = 0;
    char **parts = pap_list_split(var_val, sep_str, seplen, &count);
    free(var_val);

    int mutated = 0;




    if ((oplen == sizeof("get")-1 && memcmp(op, "get", sizeof("get")-1) == 0)) {
        if (block_count >= 1 && sb_out) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int idx = pap_list_normalize_idx(idx_str, count);
            if (idx >= 0) sb_append_n(sb_out, parts[idx], strlen(parts[idx]));
            free(idx_str);
        }
    }

    else if ((oplen == sizeof("count")-1 && memcmp(op, "count", sizeof("count")-1) == 0)) {
        if (sb_out) {
            char nbuf[32];
            snprintf(nbuf, sizeof(nbuf), "%d", count);
            sb_append_n(sb_out, nbuf, strlen(nbuf));
        }
    }

    else if ((oplen == sizeof("set")-1 && memcmp(op, "set", sizeof("set")-1) == 0)) {
        if (block_count >= 2) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int idx = pap_list_normalize_idx(idx_str, count);
            free(idx_str);
            if (idx >= 0) {
                char *content = pap_process_sv(ctx, raw_blocks[1]);
                free(parts[idx]);
                parts[idx] = content;
                mutated = 1;
            }
        }
    }

    else if ((oplen == sizeof("push")-1 && memcmp(op, "push", sizeof("push")-1) == 0)) {
        for (int i = 0; i < block_count; i++) {
            char *content = pap_process_sv(ctx, raw_blocks[i]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            parts[count++] = content;
            mutated = 1;
        }
    }

    else if ((oplen == sizeof("pop")-1 && memcmp(op, "pop", sizeof("pop")-1) == 0)) {
        if (count > 0) {
            if (sb_out) sb_append_n(sb_out, parts[count-1], strlen(parts[count-1]));
            free(parts[count-1]);
            count--;
            mutated = 1;
        }
    }

    else if ((oplen == sizeof("shift")-1 && memcmp(op, "shift", sizeof("shift")-1) == 0)) {
        if (count > 0) {
            if (sb_out) sb_append_n(sb_out, parts[0], strlen(parts[0]));
            free(parts[0]);
            if (count > 1) {
                memmove(parts, parts + 1, sizeof(char *) * (size_t)(count - 1));
            }
            count--;
            mutated = 1;
        }
    }

    else if ((oplen == sizeof("unshift")-1 && memcmp(op, "unshift", sizeof("unshift")-1) == 0)) {
        for (int i = block_count - 1; i >= 0; i--) {
            char *content = pap_process_sv(ctx, raw_blocks[i]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            memmove(parts + 1, parts, sizeof(char *) * (size_t)count);
            parts[0] = content;
            count++;
            mutated = 1;
        }
    }

    else if ((oplen == sizeof("insert")-1 && memcmp(op, "insert", sizeof("insert")-1) == 0)) {
        if (block_count >= 2) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int raw_idx = atoi(idx_str); free(idx_str);
            if (raw_idx < 0) raw_idx = count + raw_idx;
            if (raw_idx < 0) raw_idx = 0;
            if (raw_idx > count) raw_idx = count;
            char *content = pap_process_sv(ctx, raw_blocks[1]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            memmove(parts + raw_idx + 1, parts + raw_idx,
                    sizeof(char *) * (size_t)(count - raw_idx));
            parts[raw_idx] = content;
            count++;
            mutated = 1;
        }
    }

    else if ((oplen == sizeof("remove")-1 && memcmp(op, "remove", sizeof("remove")-1) == 0)) {
        if (block_count >= 1 && count > 0) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int idx = pap_list_normalize_idx(idx_str, count);
            free(idx_str);
            if (idx >= 0) {
                free(parts[idx]);
                if (idx < count - 1) {
                    memmove(parts + idx, parts + idx + 1,
                            sizeof(char *) * (size_t)(count - idx - 1));
                }
                count--;
                mutated = 1;
            }
        }
    }

    else if ((oplen == sizeof("swap")-1 && memcmp(op, "swap", sizeof("swap")-1) == 0)) {
        if (block_count >= 2 && count > 1) {
            char *ia_str = pap_process_sv(ctx, raw_blocks[0]);
            char *ib_str = pap_process_sv(ctx, raw_blocks[1]);
            int ia = pap_list_normalize_idx(ia_str, count);
            int ib = pap_list_normalize_idx(ib_str, count);
            free(ia_str); free(ib_str);
            if (ia >= 0 && ib >= 0 && ia != ib) {
                char *tmp = parts[ia]; parts[ia] = parts[ib]; parts[ib] = tmp;
                mutated = 1;
            }
        }
    }

    else if ((oplen == sizeof("reverse")-1 && memcmp(op, "reverse", sizeof("reverse")-1) == 0)) {
        if (count > 1) {
            for (int lo = 0, hi = count - 1; lo < hi; lo++, hi--) {
                char *tmp = parts[lo]; parts[lo] = parts[hi]; parts[hi] = tmp;
            }
            mutated = 1;
        }
    }

    else if ((oplen == sizeof("join")-1 && memcmp(op, "join", sizeof("join")-1) == 0)) {
        if (block_count >= 1 && sb_out) {
            char *new_sep = pap_process_sv(ctx, raw_blocks[0]);
            char *joined = pap_list_join(parts, count, new_sep, strlen(new_sep));
            sb_append_n(sb_out, joined, strlen(joined));
            free(joined); free(new_sep);
        }
    }

    else if ((oplen == sizeof("find")-1 && memcmp(op, "find", sizeof("find")-1) == 0)) {
        if (block_count >= 1 && sb_out) {
            char *pat_str = pap_process_sv(ctx, raw_blocks[0]);
            Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
            for (int i = 0; i < count; i++) {
                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                for (int s = 0; parts[i][s]; s++) {
                    if (match_pattern(ctx, parts[i], (int)strlen(parts[i]), &p, s, &m)) {
                        sb_append_n(sb_out, parts[i], strlen(parts[i]));
                        free_match(&m);
                        free_pattern(&p); free(pat_str);
                        goto done;
                    }
                }
            }
            free_pattern(&p); free(pat_str);
        }
    }

    else if ((oplen == sizeof("contains")-1 && memcmp(op, "contains", sizeof("contains")-1) == 0)) {
        if (block_count >= 1 && sb_out) {
            char *pat_str = pap_process_sv(ctx, raw_blocks[0]);
            Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
            for (int i = 0; i < count; i++) {
                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                for (int s = 0; parts[i][s]; s++) {
                    if (match_pattern(ctx, parts[i], (int)strlen(parts[i]), &p, s, &m)) {
                        char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "%d", m.start);
                        sb_append_n(sb_out, nbuf, strlen(nbuf));
                        free_match(&m);
                        free_pattern(&p); free(pat_str);
                        goto done;
                    }
                }
            }
            free_pattern(&p); free(pat_str);
        }
    }

    else if ((oplen == sizeof("replace")-1 && memcmp(op, "replace", sizeof("replace")-1) == 0)) {
        if (block_count >= 2) {
            char *pat_str = pap_process_sv(ctx, raw_blocks[0]);
            char *rep_str = pap_process_sv(ctx, raw_blocks[1]);
            Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
            for (int i = 0; i < count; i++) {
                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                for (int s = 0; parts[i][s]; s++) {
                    if (match_pattern(ctx, parts[i], (int)strlen(parts[i]), &p, s, &m)) {
                        if (sb_out) sb_append_n(sb_out, parts[i] + m.start, (size_t)(m.end - m.start));
                        char *r = apply_replacement_ex(rep_str, &m, sym);
                        size_t rl = strlen(r);
                        size_t cl = strlen(parts[i]);
                        size_t new_len = (size_t)m.start + rl + (cl - (size_t)m.end);
                        char *new_parts_i = (char *)malloc(new_len + 1);
                        memcpy(new_parts_i, parts[i], (size_t)m.start);
                        memcpy(new_parts_i + m.start, r, rl);
                        strcpy(new_parts_i + m.start + rl, parts[i] + m.end);
                        free(parts[i]); parts[i] = new_parts_i;
                        mutated = 1;
                        free(r); free_match(&m);
                        free_pattern(&p); free(pat_str); free(rep_str);
                        goto done;
                    }
                }
            }
            free_pattern(&p); free(pat_str); free(rep_str);
        }
    }


    else if ((oplen == sizeof("slice")-1 && memcmp(op, "slice", sizeof("slice")-1) == 0)) {
        if (block_count >= 1 && sb_out) {
            char *s1 = pap_process_sv(ctx, raw_blocks[0]);
            int start = atoi(s1); free(s1);
            int end = count;
            if (block_count >= 2) {
                char *s2 = pap_process_sv(ctx, raw_blocks[1]);
                end = atoi(s2); free(s2);
            }
            if (start < 0) start = count + start;
            if (end < 0) end = count + end;
            if (start < 0) start = 0;
            if (end > count) end = count;
            if (start < end) {
                char *res = pap_list_join(parts + start, end - start, sep_str, seplen);
                sb_append_n(sb_out, res, strlen(res));
                free(res);
            }
        }
    }

done:

    if (mutated) {
        char *new_val = pap_list_join(parts, count, sep_str, seplen);
        pap_var_update(ctx, sym, name, nlen, new_val);
        free(new_val);
    }

    pap_list_free(parts, count);
}





static Papagaio *g_lazy_ctx = 
# 1691 "src/papagaio.c" 3
                             ((void *)0)
# 1691 "src/papagaio.c"
                                 ;
static void pap_close_lazy_ctx(void) { if (g_lazy_ctx) { papagaio_close(g_lazy_ctx); g_lazy_ctx = 
# 1692 "src/papagaio.c" 3
                                                                                                 ((void *)0)
# 1692 "src/papagaio.c"
                                                                                                     ; } }

static Papagaio *pap_get_lazy_ctx(void)
{
    if (!g_lazy_ctx) {
        g_lazy_ctx = papagaio_open();
        if (g_lazy_ctx) atexit(pap_close_lazy_ctx);
    }
    return g_lazy_ctx;
}

static char *pap_process_impl(const char *input,
                               const char *sigil, const char *open,
                               const char *close, va_list ap)
{
    Symbols sym = make_symbols(sigil, open, close);
    int rc = 0, rcap = 8;
    Rule *rules = (Rule *)malloc(sizeof(Rule) * rcap);

    while (1) {
        const char *pat = 
# 1712 "src/papagaio.c" 3
                         __builtin_va_arg(
# 1712 "src/papagaio.c"
                         ap
# 1712 "src/papagaio.c" 3
                         ,
# 1712 "src/papagaio.c"
                         const char *
# 1712 "src/papagaio.c" 3
                         )
# 1712 "src/papagaio.c"
                                                 ; if (!pat) break;
        const char *rep = 
# 1713 "src/papagaio.c" 3
                         __builtin_va_arg(
# 1713 "src/papagaio.c"
                         ap
# 1713 "src/papagaio.c" 3
                         ,
# 1713 "src/papagaio.c"
                         const char *
# 1713 "src/papagaio.c" 3
                         )
# 1713 "src/papagaio.c"
                                                 ;
        if (rc >= rcap) { rcap <<= 1; rules = (Rule *)realloc(rules, sizeof(Rule) * rcap); }
        parse_pattern_ex(pat, &rules[rc].pattern, &sym);
        rules[rc].replacement = rep; rc++;
    }

    StrBuf out; sb_init(&out);
    Papagaio *ctx = pap_get_lazy_ctx();
    char *prepared = strdup(input);
    char *preprocessed = resolve_preprocessor(ctx, prepared, &sym); free(prepared);
    char *dispatched = dispatch_commands(ctx, preprocessed, &sym); free(preprocessed);

    const char *work = dispatched;
    int len = (int)strlen(work), pos = 0;

    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < rc; i++) {
            Match m; m.ctx = ctx;
            if (match_pattern(ctx, work, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym);
                sb_append_n(&out, r, (int)strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, work[pos++]);
    }

    free(dispatched);
    for (int i = 0; i < rc; i++) free_pattern(&rules[i].pattern);
    free(rules);

    char *result = (char *)malloc(out.len + 1);
    memcpy(result, out.data, out.len + 1);
    sb_free(&out);
    return result;
}





int papagaio_register_command(Papagaio *ctx, const char *name, PapCommandHandler handler, void *ud)
{
    if (!ctx) return -1;
    if (!name || !handler) return -1;
    if (ctx->cmd_count >= ctx->cmd_cap) {
        ctx->cmd_cap = ctx->cmd_cap ? ctx->cmd_cap << 1 : 8;
        ctx->commands = (RegisteredCommand *)realloc(ctx->commands, sizeof(RegisteredCommand) * ctx->cmd_cap);
    }
    ctx->commands[ctx->cmd_count].name = strdup(name);
    ctx->commands[ctx->cmd_count].handler = handler;
    ctx->commands[ctx->cmd_count].userdata = ud;
    ctx->cmd_count++;
    return 0;
}



int papagaio_register_modifier(Papagaio *ctx, const char *name, PapModifierHandler handler, void *ud)
{
    if (!ctx || !name || !handler) return -1;
    if (ctx->mod_count >= ctx->mod_cap) {
        ctx->mod_cap = ctx->mod_cap ? ctx->mod_cap << 1 : 8;
        ctx->modifiers = (RegisteredModifier *)realloc(ctx->modifiers, sizeof(RegisteredModifier) * ctx->mod_cap);
    }
    ctx->modifiers[ctx->mod_count].name = strdup(name);
    ctx->modifiers[ctx->mod_count].handler = handler;
    ctx->modifiers[ctx->mod_count].userdata = ud;
    ctx->mod_count++;
    return 0;
}





Papagaio *papagaio_open(void)
{
    Papagaio *ctx = (Papagaio *)malloc(sizeof(Papagaio));
    if (!ctx) return 
# 1793 "src/papagaio.c" 3
                    ((void *)0)
# 1793 "src/papagaio.c"
                        ;

    ctx->commands = 
# 1795 "src/papagaio.c" 3
                     ((void *)0)
# 1795 "src/papagaio.c"
                         ; ctx->cmd_count = 0; ctx->cmd_cap = 0;
    ctx->modifiers = 
# 1796 "src/papagaio.c" 3
                     ((void *)0)
# 1796 "src/papagaio.c"
                         ; ctx->mod_count = 0; ctx->mod_cap = 0;
    ctx->finalizers = 
# 1797 "src/papagaio.c" 3
                     ((void *)0)
# 1797 "src/papagaio.c"
                         ; ctx->fin_count = 0; ctx->fin_cap = 0;
    ctx->argc = 0; ctx->argv = 
# 1798 "src/papagaio.c" 3
                                            ((void *)0)
# 1798 "src/papagaio.c"
                                                ;
    ctx->auto_export = 1;
    ctx->global_scope = (Scope *)malloc(sizeof(Scope));
    ctx->global_scope->rules = 
# 1801 "src/papagaio.c" 3
                              ((void *)0)
# 1801 "src/papagaio.c"
                                  ;
    ctx->global_scope->rule_count = 0;
    ctx->global_scope->rule_cap = 0;
    ctx->global_scope->parent = 
# 1804 "src/papagaio.c" 3
                               ((void *)0)
# 1804 "src/papagaio.c"
                                   ;
    ctx->current_scope = ctx->global_scope;
    ctx->depth = 0;
    ctx->disable_sandbox = 0;
    ctx->disable_patterns = 0;
    ctx->original_doc = 
# 1809 "src/papagaio.c" 3
                       ((void *)0)
# 1809 "src/papagaio.c"
                           ; ctx->original_len = 0;

    papagaio_register_command(ctx, "file", file_handler, 
# 1811 "src/papagaio.c" 3
                                                        ((void *)0)
# 1811 "src/papagaio.c"
                                                            );
    return ctx;
}

void papagaio_close(Papagaio *ctx)
{
    if (!ctx) return;
    for (int i = 0; i < ctx->fin_count; i++) {
        if (ctx->finalizers[i].fn)
            ctx->finalizers[i].fn(ctx->finalizers[i].userdata);
    }
    if (ctx->commands) {
        for (int i = 0; i < ctx->cmd_count; i++) {
            free((void*)ctx->commands[i].name);
        }
        free(ctx->commands);
    }
    if (ctx->modifiers) {
        for (int i = 0; i < ctx->mod_count; i++) {
            free((void*)ctx->modifiers[i].name);
        }
        free(ctx->modifiers);
    }
    free(ctx->finalizers);
    while (ctx->current_scope) {
        Scope *parent = ctx->current_scope->parent;
        clear_scope(ctx->current_scope);
        free(ctx->current_scope);
        ctx->current_scope = parent;
    }
    if (ctx->original_doc) free(ctx->original_doc);
    free(ctx);
}

void papagaio_set_args(Papagaio *ctx, int argc, char **argv)
{
    if (!ctx) return;
    ctx->argc = argc;
    ctx->argv = argv;
}

void papagaio_get_args(Papagaio *ctx, int *argc, char ***argv)
{
    if (!ctx) { if (argc) *argc = 0; if (argv) *argv = 
# 1854 "src/papagaio.c" 3
                                                      ((void *)0)
# 1854 "src/papagaio.c"
                                                          ; return; }
    if (argc) *argc = ctx->argc;
    if (argv) *argv = ctx->argv;
}

int papagaio_has_command(Papagaio *ctx, const char *name)
{
    if (!ctx) return 0;
    for (int i = 0; i < ctx->cmd_count; i++) {
        if (strcmp(ctx->commands[i].name, name) == 0) return 1;
    }
    return 0;
}

char *papagaio_process(const char *input, ...)
{
    va_list args; 
# 1870 "src/papagaio.c" 3
                 __builtin_c23_va_start(
# 1870 "src/papagaio.c"
                 args, input
# 1870 "src/papagaio.c" 3
                 )
# 1870 "src/papagaio.c"
                                      ;
    char *r = pap_process_impl(input, "$", "{", "}", args);
    
# 1872 "src/papagaio.c" 3
   __builtin_va_end(
# 1872 "src/papagaio.c"
   args
# 1872 "src/papagaio.c" 3
   )
# 1872 "src/papagaio.c"
               ; return r;
}

char *papagaio_process_ex(const char *input, const char *sigil,
                          const char *open, const char *close, ...)
{
    va_list args; 
# 1878 "src/papagaio.c" 3
                 __builtin_c23_va_start(
# 1878 "src/papagaio.c"
                 args, close
# 1878 "src/papagaio.c" 3
                 )
# 1878 "src/papagaio.c"
                                      ;
    char *r = pap_process_impl(input, sigil, open, close, args);
    
# 1880 "src/papagaio.c" 3
   __builtin_va_end(
# 1880 "src/papagaio.c"
   args
# 1880 "src/papagaio.c" 3
   )
# 1880 "src/papagaio.c"
               ; return r;
}

char *papagaio_process_pairs(Papagaio *ctx, const char *input,
                             const char **patterns, const char **repls,
                             int pair_count)
{
    (void)ctx;
    Symbols sym = make_symbols("$", "{", "}");
    Rule *rules = (Rule *)malloc(sizeof(Rule) * (pair_count ? pair_count : 1));
    if (!rules) return 
# 1890 "src/papagaio.c" 3
                      ((void *)0)
# 1890 "src/papagaio.c"
                          ;

    for (int i = 0; i < pair_count; i++) {
        parse_pattern_ex(patterns[i], &rules[i].pattern, &sym);
        rules[i].replacement = repls[i];
    }

    StrBuf out; sb_init(&out);
    int len = (int)strlen(input), pos = 0;

    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < pair_count; i++) {
            Match m; m.ctx = ctx;
            if (match_pattern(ctx, input, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym);
                sb_append_n(&out, r, strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, input[pos++]);
    }

    for (int i = 0; i < pair_count; i++) free_pattern(&rules[i].pattern);
    free(rules);

    char *result = (char *)malloc(out.len + 1);
    if (!result) { sb_free(&out); return 
# 1917 "src/papagaio.c" 3
                                        ((void *)0)
# 1917 "src/papagaio.c"
                                            ; }
    memcpy(result, out.data, out.len + 1);
    sb_free(&out); return result;
}

static char *file_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)ctx; (void)name; (void)ud; (void)argl;
    if (argc < 1) return strdup("");

    char trim_path[256]; size_t pl = strlen(argv[0]);
    size_t start = 0; while(start < pl && 
# 1927 "src/papagaio.c" 3
                                         ((*__ctype_b_loc ())[(int) ((
# 1927 "src/papagaio.c"
                                         (unsigned char)argv[0][start]
# 1927 "src/papagaio.c" 3
                                         ))] & (unsigned short int) _ISspace)
# 1927 "src/papagaio.c"
                                                                               ) start++;
    size_t end = pl; while(end > start && 
# 1928 "src/papagaio.c" 3
                                         ((*__ctype_b_loc ())[(int) ((
# 1928 "src/papagaio.c"
                                         (unsigned char)argv[0][end-1]
# 1928 "src/papagaio.c" 3
                                         ))] & (unsigned short int) _ISspace)
# 1928 "src/papagaio.c"
                                                                               ) end--;
    size_t len = end - start; if (len >= 255) len = 255;
    memcpy(trim_path, argv[0] + start, len); trim_path[len] = '\0';

    FILE *f = fopen(trim_path, "rb");
    if (!f) return strdup("");
    fseek(f, 0, 
# 1934 "src/papagaio.c" 3
               2
# 1934 "src/papagaio.c"
                       );
    long sz = ftell(f);
    fseek(f, 0, 
# 1936 "src/papagaio.c" 3
               0
# 1936 "src/papagaio.c"
                       );
    char *buf = malloc(sz + 1);
    if (buf) {
        size_t rb = fread(buf, 1, sz, f);
        buf[rb] = '\0';
    } else {
        buf = strdup("");
    }
    fclose(f);
    return buf;
}



static char *resolve_preprocessor(Papagaio *ctx, const char *src, Symbols *sym)
{
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);

    while (i < len) {
        if (src[i] == '$') {
            size_t j = i + 1;
            size_t ks = j;
            while (j < len && (
# 1959 "src/papagaio.c" 3
                              ((*__ctype_b_loc ())[(int) ((
# 1959 "src/papagaio.c"
                              (unsigned char)src[j]
# 1959 "src/papagaio.c" 3
                              ))] & (unsigned short int) _ISalnum) 
# 1959 "src/papagaio.c"
                                                             || src[j] == '_')) j++;
            size_t klen = j - ks;

            if (klen == 7 && memcmp(src + ks, "pattern", 7) == 0) {

                size_t j_pat = j;
                while (j_pat < len && 
# 1965 "src/papagaio.c" 3
                                     ((*__ctype_b_loc ())[(int) ((
# 1965 "src/papagaio.c"
                                     (unsigned char)src[j_pat]
# 1965 "src/papagaio.c" 3
                                     ))] & (unsigned short int) _ISspace)
# 1965 "src/papagaio.c"
                                                                       ) j_pat++;
                StrView so = { sym->open, strlen(sym->open) };
                StrView sc = { sym->close, strlen(sym->close) };
                if (j_pat < len && str_pfx(src + j_pat, sym->open)) {
                    StrView match_blk;
                    int next1 = extract_block(src, (int)j_pat, so, sc, &match_blk);
                    size_t k_pat = (size_t)next1;
                    while (k_pat < len && 
# 1972 "src/papagaio.c" 3
                                         ((*__ctype_b_loc ())[(int) ((
# 1972 "src/papagaio.c"
                                         (unsigned char)src[k_pat]
# 1972 "src/papagaio.c" 3
                                         ))] & (unsigned short int) _ISspace)
# 1972 "src/papagaio.c"
                                                                           ) k_pat++;
                    if (k_pat < len && str_pfx(src + k_pat, sym->open)) {
                        StrView repl_blk;
                        int next2 = extract_block(src, (int)k_pat, so, sc, &repl_blk);


                        char *match_str = (char *)malloc(match_blk.len + 1);
                        memcpy(match_str, match_blk.ptr, match_blk.len);
                        match_str[match_blk.len] = '\0';
                        char *proc_match = resolve_preprocessor(ctx, match_str, sym);
                        free(match_str);


                        sb_append_n(&out, src + i, j_pat - i);
                        sb_append_n(&out, sym->open, strlen(sym->open));
                        sb_append_n(&out, proc_match, strlen(proc_match));
                        sb_append_n(&out, sym->close, strlen(sym->close));
                        free(proc_match);

                        sb_append_n(&out, src + next1, (size_t)next2 - (size_t)next1);
                        i = (size_t)next2;
                        continue;
                    }
                }
            }


            if (klen == 0 && j < len && str_pfx(src + j, sym->open)) {
                StrView so_b = { sym->open, strlen(sym->open) };
                StrView sc_b = { sym->close, strlen(sym->close) };
                StrView blk_b;
                size_t jb = (size_t)extract_block(src, (int)j, so_b, sc_b, &blk_b);

                if (jb < len && src[jb] == '$') {
                    size_t nj = jb + 1, njs = nj;
                    while (nj < len && (
# 2007 "src/papagaio.c" 3
                                       ((*__ctype_b_loc ())[(int) ((
# 2007 "src/papagaio.c"
                                       (unsigned char)src[nj]
# 2007 "src/papagaio.c" 3
                                       ))] & (unsigned short int) _ISalnum) 
# 2007 "src/papagaio.c"
                                                                       || src[nj] == '_')) nj++;
                    size_t nfl = nj - njs;
                    if ((nfl == 7 && memcmp(src + njs, "compare", 7) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "then", 4) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "else", 4) == 0) ||
                        (nfl == 6 && memcmp(src + njs, "repeat", 6) == 0) ||
                        (nfl == 5 && memcmp(src + njs, "while", 5) == 0) ||
                        (nfl == 5 && memcmp(src + njs, "until", 5) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "byte", 4) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "find", 4) == 0) ||
                        (nfl == 8 && memcmp(src + njs, "includes", 8) == 0) ||
                        (nfl == 7 && memcmp(src + njs, "replace", 7) == 0)) {

                        char *cur_val = pap_process_sv(ctx, blk_b);
                        size_t cp = jb;
                        StrView so_f = { sym->open, strlen(sym->open) };
                        StrView sc_f = { sym->close, strlen(sym->close) };
                        while (cp < len && src[cp] == '$') {
                            size_t j2 = cp + 1, ops = j2;
                            while (j2 < len && (
# 2026 "src/papagaio.c" 3
                                               ((*__ctype_b_loc ())[(int) ((
# 2026 "src/papagaio.c"
                                               (unsigned char)src[j2]
# 2026 "src/papagaio.c" 3
                                               ))] & (unsigned short int) _ISalnum) 
# 2026 "src/papagaio.c"
                                                                               || src[j2] == '_')) j2++;
                            size_t opl = j2 - ops;
                            int is_cmp = (opl == 7 && memcmp(src + ops, "compare", 7) == 0);
                            int is_then = (opl == 4 && memcmp(src + ops, "then", 4) == 0);
                            int is_else = (opl == 4 && memcmp(src + ops, "else", 4) == 0);
                            int is_repeat = (opl == 6 && memcmp(src + ops, "repeat", 6) == 0);
                            int is_while = (opl == 5 && memcmp(src + ops, "while", 5) == 0);
                            int is_until = (opl == 5 && memcmp(src + ops, "until", 5) == 0);
                            int is_byte = (opl == 4 && memcmp(src + ops, "byte", 4) == 0);
                            int is_find = (opl == 4 && memcmp(src + ops, "find", 4) == 0);
                            int is_includes = (opl == 8 && memcmp(src + ops, "includes", 8) == 0);
                            int is_replace = (opl == 7 && memcmp(src + ops, "replace", 7) == 0);

                            if (!is_cmp && !is_then && !is_else && !is_repeat && !is_while &&
                                !is_until && !is_byte && !is_find && !is_includes && !is_replace) break;

                            size_t j3 = j2;
                            while (j3 < len && 
# 2043 "src/papagaio.c" 3
                                              ((*__ctype_b_loc ())[(int) ((
# 2043 "src/papagaio.c"
                                              (unsigned char)src[j3]
# 2043 "src/papagaio.c" 3
                                              ))] & (unsigned short int) _ISspace)
# 2043 "src/papagaio.c"
                                                                             ) j3++;
                            if (j3 >= len || !str_pfx(src + j3, sym->open)) break;
                            StrView blk; j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk);
                            char *arg = (is_repeat || is_while || is_until || is_find || is_includes || is_replace) ? 
# 2046 "src/papagaio.c" 3
                                                                                                                     ((void *)0) 
# 2046 "src/papagaio.c"
                                                                                                                          : pap_process_sv(ctx, blk);

                            if (is_cmp) {
                                if (strcmp(cur_val, arg) != 0) { free(cur_val); cur_val = strdup(""); }
                            } else if (is_then) {
                                if (cur_val[0] != '\0') { free(cur_val); cur_val = arg; arg = 
# 2051 "src/papagaio.c" 3
                                                                                             ((void *)0)
# 2051 "src/papagaio.c"
                                                                                                 ; }
                                else { free(cur_val); cur_val = strdup(""); }
                            } else if (is_else) {
                                if (cur_val[0] == '\0') { free(cur_val); cur_val = arg; arg = 
# 2054 "src/papagaio.c" 3
                                                                                             ((void *)0)
# 2054 "src/papagaio.c"
                                                                                                 ; }
                            } else if (is_byte) {
                                int code = atoi(arg);
                                char b[2] = {(char)code, '\0'};
                                free(cur_val); cur_val = strdup(b);
                            } else if (is_find) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                char *match_res = strdup("");
                                for (int s = 0; cur_val[s]; s++) {
                                    if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                        free(match_res);
                                        match_res = (char*)malloc((size_t)(m.end - m.start + 1));
                                        memcpy(match_res, cur_val + m.start, (size_t)(m.end - m.start));
                                        match_res[m.end - m.start] = '\0';
                                        free_match(&m);
                                        break;
                                    }
                                }
                                free(cur_val); cur_val = match_res;
                                free_pattern(&p); free(pat_str);
                            } else if (is_includes) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                char *idx_res = strdup("");
                                for (int s = 0; cur_val[s]; s++) {
                                    if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                        free(idx_res);
                                        char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "%d", m.start);
                                        idx_res = strdup(nbuf);
                                        free_match(&m);
                                        break;
                                    }
                                }
                                free(cur_val); cur_val = idx_res;
                                free_pattern(&p); free(pat_str);
                            } else if (is_replace) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                while (j3 < len && 
# 2094 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2094 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2094 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2094 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView rep_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &rep_blk);
                                    char *rep_str = pap_process_sv(ctx, rep_blk);
                                    Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                    Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                    int replaced = 0;
                                    for (int s = 0; cur_val[s]; s++) {
                                        if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                            char *old_match = (char*)malloc((size_t)(m.end - m.start + 1));
                                            memcpy(old_match, cur_val + m.start, (size_t)(m.end - m.start));
                                            old_match[m.end - m.start] = '\0';
                                            char *r = apply_replacement_ex(rep_str, &m, sym);
                                            size_t rl = strlen(r), cl = strlen(cur_val);
                                            size_t new_len = (size_t)m.start + rl + (cl - (size_t)m.end);
                                            char *new_val = (char *)malloc(new_len + 1);
                                            memcpy(new_val, cur_val, (size_t)m.start);
                                            memcpy(new_val + m.start, r, rl);
                                            strcpy(new_val + m.start + rl, cur_val + m.end);


                                            if (klen > 0) pap_var_update(ctx, sym, src + ks, klen, new_val);

                                            free(cur_val); cur_val = old_match;
                                            free(new_val); free(r); free_match(&m);
                                            replaced = 1; break;
                                        }
                                    }
                                    if (!replaced) {
                                        free(cur_val); cur_val = strdup("");
                                    }
                                    free_pattern(&p); free(rep_str);
                                }
                                free(pat_str);
                            } else if (is_repeat) {
                                char *times_str = pap_process_sv(ctx, blk);
                                int times = atoi(times_str); free(times_str);
                                while (j3 < len && 
# 2132 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2132 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2132 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2132 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    for (int t = 0; t < times; t++) {
                                        char *tmp = pap_process_sv(ctx, code_blk);
                                        free(tmp);
                                    }
                                }
                                free(cur_val); cur_val = strdup("");
                            } else if (is_while || is_until) {
                                StrView pat_blk = blk;
                                char *pat_str = pap_process_sv(ctx, pat_blk);
                                Pattern pat; parse_pattern_ex(pat_str, &pat, sym);
                                free(pat_str);

                                while (j3 < len && 
# 2148 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2148 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2148 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2148 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    StrBuf comb; sb_init(&comb);
                                    char *last_res = strdup("");
                                    while (1) {
                                        char *iter = pap_process_sv(ctx, code_blk);
                                        Match m; m.ctx = ctx;
                                        int matches = match_pattern(ctx, iter, (int)strlen(iter), &pat, 0, &m);
                                        if (matches) free_match(&m);

                                        if (is_while) {
                                            if (!matches) { free(iter); break; }
                                            free(last_res); last_res = iter;
                                        } else {
                                            sb_append_n(&comb, iter, strlen(iter));
                                            if (matches) { free(iter); break; }
                                            free(iter);
                                        }
                                    }
                                    free(cur_val);
                                    if (is_while) { cur_val = last_res; sb_free(&comb); }
                                    else { cur_val = strdup(comb.data); sb_free(&comb); free(last_res); }
                                }
                                free_pattern(&pat);
                            }
                            if (arg) free(arg);
                            cp = j3;
                        }
                        sb_append_n(&out, cur_val, strlen(cur_val));
                        free(cur_val);
                        i = cp;
                        goto next_char;
                    }
                }
            }

            if (klen > 0) {

                if (klen == 5 && memcmp(src + ks, "sigil", 5) == 0) {
                    sb_append_n(&out, sym->sigil, strlen(sym->sigil));
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2190 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2190 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2190 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2190 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }
                if (klen == 4 && memcmp(src + ks, "open", 4) == 0) {
                    sb_append_n(&out, sym->open, strlen(sym->open));
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2195 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2195 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2195 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2195 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }
                if (klen == 5 && memcmp(src + ks, "close", 5) == 0) {
                    sb_append_n(&out, sym->close, strlen(sym->close));
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2200 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2200 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2200 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2200 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }
                if (klen == 6 && memcmp(src + ks, "marker", 6) == 0) {
                    sb_append_n(&out, sym->optional, strlen(sym->optional));
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2205 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2205 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2205 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2205 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }


                if (klen == 5 && memcmp(src + ks, "space", 5) == 0) {
                    sb_append_char(&out, ' ');
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2212 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2212 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2212 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2212 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }
                if (klen == 7 && memcmp(src + ks, "newline", 7) == 0) {
                    sb_append_char(&out, '\n');
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2217 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2217 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2217 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2217 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }
                if (klen == 3 && memcmp(src + ks, "tab", 3) == 0) {
                    sb_append_char(&out, '\t');
                    if (j < len && src[j] == '$') { j++; while (j < len && 
# 2222 "src/papagaio.c" 3
                                                                          ((*__ctype_b_loc ())[(int) ((
# 2222 "src/papagaio.c"
                                                                          (unsigned char)src[j]
# 2222 "src/papagaio.c" 3
                                                                          ))] & (unsigned short int) _ISspace)
# 2222 "src/papagaio.c"
                                                                                                        ) j++; }
                    i = j; continue;
                }
                if (klen == 5 && memcmp(src + ks, "ascii", 5) == 0) {
                    if (j < len && src[j] == '$') {
                        size_t ns = j + 1, ne = ns;
                        while (ne < len && 
# 2228 "src/papagaio.c" 3
                                          ((*__ctype_b_loc ())[(int) ((
# 2228 "src/papagaio.c"
                                          (unsigned char)src[ne]
# 2228 "src/papagaio.c" 3
                                          ))] & (unsigned short int) _ISdigit)
# 2228 "src/papagaio.c"
                                                                         ) ne++;
                        if (ne > ns) {
                            int code = atoi(src + ns);
                            sb_append_char(&out, (char)code);
                            i = ne; continue;
                        }
                    } else {
                        while(j < len && 
# 2235 "src/papagaio.c" 3
                                        ((*__ctype_b_loc ())[(int) ((
# 2235 "src/papagaio.c"
                                        (unsigned char)src[j]
# 2235 "src/papagaio.c" 3
                                        ))] & (unsigned short int) _ISspace)
# 2235 "src/papagaio.c"
                                                                      ) j++;
                        if (j < len && str_pfx(src + j, sym->open)) {
                            StrView blk;
                            int next = extract_block(src, (int)j, (StrView){sym->open, strlen(sym->open)}, (StrView){sym->close, strlen(sym->close)}, &blk);
                            char *arg = pap_process_sv(ctx, blk);
                            int code = atoi(arg);
                            free(arg);
                            sb_append_char(&out, (char)code);
                            i = (size_t)next; continue;
                        }
                    }
                }

                if (klen == 4 && memcmp(src + ks, "math", 4) == 0) {
                    while(j < len && 
# 2249 "src/papagaio.c" 3
                                    ((*__ctype_b_loc ())[(int) ((
# 2249 "src/papagaio.c"
                                    (unsigned char)src[j]
# 2249 "src/papagaio.c" 3
                                    ))] & (unsigned short int) _ISspace)
# 2249 "src/papagaio.c"
                                                                  ) j++;
                    if (j < len && str_pfx(src + j, sym->open)) {
                        StrView blk;
                        int next = extract_block(src, (int)j, (StrView){sym->open, strlen(sym->open)}, (StrView){sym->close, strlen(sym->close)}, &blk);
                        char *arg = pap_process_sv(ctx, blk);
                        int err = 0;
                        LouroVariable scope[] = { {"+", (const void*)(lr_add), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((30) << 12), 0}, {"-", (const void*)(lr_sub), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((30) << 12), 0}, {"-", (const void*)(lr_negate), LOURO_OPERATOR | LOURO_FLAG_PREFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | ((60) << 12), 0}, {"+", (const void*)(lr_add), LOURO_OPERATOR | LOURO_FLAG_PREFIX | LOURO_FUNCTION1 | LOURO_FLAG_PURE | ((60) << 12), 0}, {"*", (const void*)(lr_mul), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((40) << 12), 0}, {"/", (const void*)(lr_divide), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((40) << 12), 0}, {"%", (const void*)(fmod), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((40) << 12), 0}, {"^", (const void*)(pow), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FLAG_RIGHT_ASSOC | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((50) << 12), 0}, {"<", (const void*)(lr_cmp_lt), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((20) << 12), 0}, {">", (const void*)(lr_cmp_gt), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((20) << 12), 0}, {"<=", (const void*)(lr_cmp_le), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((20) << 12), 0}, {">=", (const void*)(lr_cmp_ge), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((20) << 12), 0}, {"==", (const void*)(lr_cmp_eq), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((20) << 12), 0}, {"!=", (const void*)(lr_cmp_ne), LOURO_OPERATOR | LOURO_FLAG_INFIX | LOURO_FUNCTION2 | LOURO_FLAG_PURE | ((20) << 12), 0}, {"sqrt", (const void*)((double(*)(double))sqrt), _Generic(((double(*)(double))sqrt), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"sin", (const void*)((double(*)(double))sin), _Generic(((double(*)(double))sin), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"cos", (const void*)((double(*)(double))cos), _Generic(((double(*)(double))cos), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"tan", (const void*)((double(*)(double))tan), _Generic(((double(*)(double))tan), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"asin", (const void*)((double(*)(double))asin), _Generic(((double(*)(double))asin), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"acos", (const void*)((double(*)(double))acos), _Generic(((double(*)(double))acos), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"atan", (const void*)((double(*)(double))atan), _Generic(((double(*)(double))atan), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"atan2", (const void*)((double(*)(double, double))atan2), _Generic(((double(*)(double, double))atan2), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"abs", (const void*)((double(*)(double))fabs), _Generic(((double(*)(double))fabs), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"fabs", (const void*)((double(*)(double))fabs), _Generic(((double(*)(double))fabs), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"log", (const void*)((double(*)(double))log), _Generic(((double(*)(double))log), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"log10", (const void*)((double(*)(double))log10), _Generic(((double(*)(double))log10), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"exp", (const void*)((double(*)(double))exp), _Generic(((double(*)(double))exp), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"ceil", (const void*)((double(*)(double))ceil), _Generic(((double(*)(double))ceil), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"floor", (const void*)((double(*)(double))floor), _Generic(((double(*)(double))floor), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0}, {"pi", (const void*)((double(*)(void))lr_pi), _Generic(((double(*)(void))lr_pi), double (*)(void): LOURO_FUNCTION0, double (*)(double): LOURO_FUNCTION1, double (*)(double, double): LOURO_FUNCTION2, double (*)(double, double, double): LOURO_FUNCTION3, double (*)(double, double, double, double): LOURO_FUNCTION4, double (*)(double, double, double, double, double): LOURO_FUNCTION5, double (*)(double, double, double, double, double, double): LOURO_FUNCTION6, double (*)(double, double, double, double, double, double, double): LOURO_FUNCTION7 ) | LOURO_FLAG_PURE, 0} };
                        int count = sizeof(scope) / sizeof(scope[0]);
                        LouroExpression *expr = louro_compile(arg, scope, count, &err);
                        free(arg);
                        if (expr) {
                            double result = louro_evaluate(expr);
                            louro_free(expr);
                            char res_buf[64];
                            if (result == (long long)result) {
                                snprintf(res_buf, sizeof(res_buf), "%lld", (long long)result);
                            } else {
                                snprintf(res_buf, sizeof(res_buf), "%g", result);
                            }
                            sb_append_n(&out, res_buf, strlen(res_buf));
                        }
                        j = (size_t)next;
                        if (j < len && src[j] == '$') { j++; while (j < len && 
# 2271 "src/papagaio.c" 3
                                                                              ((*__ctype_b_loc ())[(int) ((
# 2271 "src/papagaio.c"
                                                                              (unsigned char)src[j]
# 2271 "src/papagaio.c" 3
                                                                              ))] & (unsigned short int) _ISspace)
# 2271 "src/papagaio.c"
                                                                                                            ) j++; }
                        i = j; continue;
                    }
                }


                if (j < len && src[j] == '$') {
                    size_t j2 = j + 1;
                    if (j2 + 4 <= len && memcmp(src + j2, "from", 4) == 0) {
                        size_t ks_name = ks;
                        size_t name_len = j - ks;
                        size_t j_next = j2 + 4;
                        while(j_next < len && 
# 2283 "src/papagaio.c" 3
                                             ((*__ctype_b_loc ())[(int) ((
# 2283 "src/papagaio.c"
                                             (unsigned char)src[j_next]
# 2283 "src/papagaio.c" 3
                                             ))] & (unsigned short int) _ISspace)
# 2283 "src/papagaio.c"
                                                                                ) j_next++;

                        StrView so = { sym->open, strlen(sym->open) };
                        StrView sc = { sym->close, strlen(sym->close) };
                        if (j_next < len && str_pfx(src + j_next, sym->open)) {
                            StrView val_blk;
                            j_next = (size_t)extract_block(src, (int)j_next, so, sc, &val_blk);
                            char *processed_val = papagaio_process_text(ctx, val_blk.ptr, val_blk.len);
                            if (processed_val) {
                                char var_name[256];
                                size_t len_to_copy = name_len < 255 ? name_len : 255;
                                strncpy(var_name, src + ks_name, len_to_copy);
                                var_name[len_to_copy] = '\0';

                                pap_var_update(ctx, sym, var_name, name_len, processed_val);
                                free(processed_val);
                            }
                            i = j_next; continue;
                        }
                    }
                }


                if (j < len && src[j] == '$') {
                    size_t j2 = j + 1;
                    size_t ms = j2;
                    while (j2 < len && (
# 2309 "src/papagaio.c" 3
                                       ((*__ctype_b_loc ())[(int) ((
# 2309 "src/papagaio.c"
                                       (unsigned char)src[j2]
# 2309 "src/papagaio.c" 3
                                       ))] & (unsigned short int) _ISalnum) 
# 2309 "src/papagaio.c"
                                                                       || src[j2] == '_')) j2++;
                    size_t mlen = j2 - ms;

                    if (mlen == 4 && memcmp(src + ms, "list", 4) == 0) {

                        size_t j3 = j2;
                        StrView so = { sym->open, strlen(sym->open) };
                        StrView sc = { sym->close, strlen(sym->close) };
                        while (j3 < len && 
# 2317 "src/papagaio.c" 3
                                          ((*__ctype_b_loc ())[(int) ((
# 2317 "src/papagaio.c"
                                          (unsigned char)src[j3]
# 2317 "src/papagaio.c" 3
                                          ))] & (unsigned short int) _ISspace)
# 2317 "src/papagaio.c"
                                                                         ) j3++;
                        if (j3 < len && str_pfx(src + j3, sym->open)) {
                            StrView sep_blk;
                            j3 = (size_t)extract_block(src, (int)j3, so, sc, &sep_blk);
                            char *sep_str = pap_process_sv(ctx, sep_blk);


                            while (j3 < len && 
# 2324 "src/papagaio.c" 3
                                              ((*__ctype_b_loc ())[(int) ((
# 2324 "src/papagaio.c"
                                              (unsigned char)src[j3]
# 2324 "src/papagaio.c" 3
                                              ))] & (unsigned short int) _ISspace)
# 2324 "src/papagaio.c"
                                                                             ) j3++;
                            if (j3 < len && src[j3] == '$') {
                                size_t ops = j3 + 1;
                                size_t j4 = ops;
                                while (j4 < len && (
# 2328 "src/papagaio.c" 3
                                                   ((*__ctype_b_loc ())[(int) ((
# 2328 "src/papagaio.c"
                                                   (unsigned char)src[j4]
# 2328 "src/papagaio.c" 3
                                                   ))] & (unsigned short int) _ISalnum) 
# 2328 "src/papagaio.c"
                                                                                   || src[j4] == '_')) j4++;
                                size_t oplen = j4 - ops;


                                StrView blocks[4];
                                int bcount = 0;
                                size_t jb = j4;
                                while (bcount < 4) {
                                    size_t jb_saved = jb;
                                    while (jb < len && 
# 2337 "src/papagaio.c" 3
                                                      ((*__ctype_b_loc ())[(int) ((
# 2337 "src/papagaio.c"
                                                      (unsigned char)src[jb]
# 2337 "src/papagaio.c" 3
                                                      ))] & (unsigned short int) _ISspace)
# 2337 "src/papagaio.c"
                                                                                     ) jb++;
                                    if (jb >= len || !str_pfx(src + jb, sym->open)) {
                                        jb = jb_saved; break;
                                    }
                                    jb = (size_t)extract_block(src, (int)jb, so, sc, &blocks[bcount++]);
                                }

                                pap_list_op(ctx, sym, &out, src + ks, klen, sep_str, strlen(sep_str), src + ops, oplen, blocks, bcount);
                                i = jb; free(sep_str);
                                continue;
                            }
                            free(sep_str);
                        }
                    }
                }
# 2365 "src/papagaio.c"
                {
                    int cur_is_then = (klen == 4 && memcmp(src + ks, "then", 4) == 0);
                    int cur_is_else = (klen == 4 && memcmp(src + ks, "else", 4) == 0);
                    int cur_is_compare = (klen == 7 && memcmp(src + ks, "compare", 7) == 0);
                    int cur_is_repeat = (klen == 6 && memcmp(src + ks, "repeat", 6) == 0);
                    int cur_is_while = (klen == 5 && memcmp(src + ks, "while", 5) == 0);
                    int cur_is_until = (klen == 5 && memcmp(src + ks, "until", 5) == 0);
                    int cur_is_byte = (klen == 4 && memcmp(src + ks, "byte", 4) == 0);
                    int cur_is_find = (klen == 4 && memcmp(src + ks, "find", 4) == 0);
                    int cur_is_contains = (klen == 8 && memcmp(src + ks, "contains", 8) == 0);
                    int cur_is_replace = (klen == 7 && memcmp(src + ks, "replace", 7) == 0);
                    int cur_is_slice = (klen == 5 && memcmp(src + ks, "slice", 5) == 0);
                    int cur_is_flow = cur_is_then || cur_is_else || cur_is_compare ||
                                          cur_is_repeat || cur_is_while || cur_is_until ||
                                          cur_is_byte || cur_is_find || cur_is_contains ||
                                          cur_is_replace || cur_is_slice;


                    int next_is_flow = 0;
                    if (!cur_is_flow && j < len && src[j] == '$') {
                        size_t nj = j + 1, njs = nj;
                        while (nj < len && (
# 2386 "src/papagaio.c" 3
                                           ((*__ctype_b_loc ())[(int) ((
# 2386 "src/papagaio.c"
                                           (unsigned char)src[nj]
# 2386 "src/papagaio.c" 3
                                           ))] & (unsigned short int) _ISalnum) 
# 2386 "src/papagaio.c"
                                                                           || src[nj] == '_')) nj++;
                        size_t nfl = nj - njs;
                        int is_flow = 0;
                        int is_conflicting = 0;
                        if ((nfl == 7 && memcmp(src + njs, "compare", 7) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "then", 4) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "else", 4) == 0) ||
                            (nfl == 6 && memcmp(src + njs, "repeat", 6) == 0) ||
                            (nfl == 5 && memcmp(src + njs, "while", 5) == 0) ||
                            (nfl == 5 && memcmp(src + njs, "until", 5) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "byte", 4) == 0)) {
                            is_flow = 1;
                        } else if ((nfl == 4 && memcmp(src + njs, "find", 4) == 0) ||
                                   (nfl == 8 && memcmp(src + njs, "contains", 8) == 0) ||
                                   (nfl == 7 && memcmp(src + njs, "replace", 7) == 0) ||
                                   (nfl == 5 && memcmp(src + njs, "slice", 5) == 0)) {
                            is_flow = 1; is_conflicting = 1;
                        }

                        if (is_flow) {
                            if (is_conflicting) {
                                char *v = pap_var_lookup(ctx, sym, src + ks, klen);
                                if (v) { next_is_flow = 1; free(v); }
                            } else {
                                next_is_flow = 1;
                            }
                        }
                    }

                    if (cur_is_flow || next_is_flow) {
                        StrView so_f = { sym->open, strlen(sym->open) };
                        StrView sc_f = { sym->close, strlen(sym->close) };


                        char *cur_val;
                        size_t cp;

                        if (cur_is_flow) {
                            cur_val = strdup("");
                            cp = i;
                        } else {
                            cur_val = pap_var_lookup(ctx, sym, src + ks, klen);
                            if (!cur_val) cur_val = strdup("");
                            cp = j;
                        }



                        while (cp < len && src[cp] == '$') {
                            size_t j2 = cp + 1, ops = j2;
                            while (j2 < len && (
# 2436 "src/papagaio.c" 3
                                               ((*__ctype_b_loc ())[(int) ((
# 2436 "src/papagaio.c"
                                               (unsigned char)src[j2]
# 2436 "src/papagaio.c" 3
                                               ))] & (unsigned short int) _ISalnum) 
# 2436 "src/papagaio.c"
                                                                               || src[j2] == '_')) j2++;
                            size_t opl = j2 - ops;

                            int is_cmp = (opl == 7 && memcmp(src + ops, "compare", 7) == 0);
                            int is_then = (opl == 4 && memcmp(src + ops, "then", 4) == 0);
                            int is_else = (opl == 4 && memcmp(src + ops, "else", 4) == 0);
                            int is_repeat = (opl == 6 && memcmp(src + ops, "repeat", 6) == 0);
                            int is_while = (opl == 5 && memcmp(src + ops, "while", 5) == 0);
                            int is_until = (opl == 5 && memcmp(src + ops, "until", 5) == 0);
                            int is_byte = (opl == 4 && memcmp(src + ops, "byte", 4) == 0);
                            int is_find = (opl == 4 && memcmp(src + ops, "find", 4) == 0);
                            int is_contains = (opl == 8 && memcmp(src + ops, "contains", 8) == 0);
                            int is_replace = (opl == 7 && memcmp(src + ops, "replace", 7) == 0);
                            int is_slice = (opl == 5 && memcmp(src + ops, "slice", 5) == 0);

                            if (!is_cmp && !is_then && !is_else && !is_repeat && !is_while &&
                                !is_until && !is_byte && !is_find && !is_contains && !is_replace && !is_slice) break;


                            size_t j3 = j2;
                            while (j3 < len && 
# 2456 "src/papagaio.c" 3
                                              ((*__ctype_b_loc ())[(int) ((
# 2456 "src/papagaio.c"
                                              (unsigned char)src[j3]
# 2456 "src/papagaio.c" 3
                                              ))] & (unsigned short int) _ISspace)
# 2456 "src/papagaio.c"
                                                                             ) j3++;
                            if (j3 >= len || !str_pfx(src + j3, sym->open)) break;

                            StrView blk;
                            j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk);
                            char *arg = (is_repeat || is_while || is_until || is_find || is_contains || is_replace || is_slice) ? 
# 2461 "src/papagaio.c" 3
                                                                                                                                 ((void *)0) 
# 2461 "src/papagaio.c"
                                                                                                                                      : pap_process_sv(ctx, blk);

                            if (is_cmp) {
                                if (strcmp(cur_val, arg) != 0) { free(cur_val); cur_val = strdup(""); }
                            } else if (is_then) {
                                if (cur_val[0] != '\0') { free(cur_val); cur_val = arg; arg = 
# 2466 "src/papagaio.c" 3
                                                                                             ((void *)0)
# 2466 "src/papagaio.c"
                                                                                                 ; }
                                else { free(cur_val); cur_val = strdup(""); }
                            } else if (is_else) {
                                if (cur_val[0] == '\0') { free(cur_val); cur_val = arg; arg = 
# 2469 "src/papagaio.c" 3
                                                                                             ((void *)0)
# 2469 "src/papagaio.c"
                                                                                                 ; }
                            } else if (is_byte) {
                                int code = atoi(arg);
                                size_t cl = strlen(cur_val);
                                char *nv = (char*)realloc(cur_val, cl + 2);
                                if (nv) {
                                    cur_val = nv; cur_val[cl] = (char)code; cur_val[cl+1] = '\0';

                                    if (klen > 0) {
                                        pap_var_update(ctx, sym, src + ks, klen, cur_val);
                                    }
                                }
                            } else if (is_find) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                char *match_res = strdup("");
                                for (int s = 0; cur_val[s]; s++) {
                                    if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                        free(match_res);
                                        match_res = (char*)malloc((size_t)(m.end - m.start + 1));
                                        memcpy(match_res, cur_val + m.start, (size_t)(m.end - m.start));
                                        match_res[m.end - m.start] = '\0';
                                        free_match(&m);
                                        break;
                                    }
                                }
                                free(cur_val); cur_val = match_res;
                                free_pattern(&p); free(pat_str);
                            } else if (is_slice) {
                                char *s1 = pap_process_sv(ctx, blk);
                                int start = atoi(s1); free(s1);
                                int end = (int)strlen(cur_val);


                                size_t j2_sl = j3;
                                while (j2_sl < len && 
# 2505 "src/papagaio.c" 3
                                                     ((*__ctype_b_loc ())[(int) ((
# 2505 "src/papagaio.c"
                                                     (unsigned char)src[j2_sl]
# 2505 "src/papagaio.c" 3
                                                     ))] & (unsigned short int) _ISspace)
# 2505 "src/papagaio.c"
                                                                                       ) j2_sl++;
                                if (j2_sl < len && str_pfx(src + j2_sl, sym->open)) {
                                    StrView blk2;
                                    j3 = (size_t)extract_block(src, (int)j2_sl, so_f, sc_f, &blk2);
                                    char *s2 = pap_process_sv(ctx, blk2);
                                    end = atoi(s2); free(s2);
                                }

                                int len_cv = (int)strlen(cur_val);
                                if (start < 0) start = len_cv + start;
                                if (end < 0) end = len_cv + end;
                                if (start < 0) start = 0;
                                if (end > len_cv) end = len_cv;
                                char *slice_res = strdup("");
                                if (start < end) {
                                    slice_res = (char*)malloc((size_t)(end - start + 1));
                                    memcpy(slice_res, cur_val + start, (size_t)(end - start));
                                    slice_res[end - start] = '\0';
                                }
                                free(cur_val); cur_val = slice_res;
                            } else if (is_contains) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                char *idx_res = strdup("");
                                for (int s = 0; cur_val[s]; s++) {
                                    if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                        free(idx_res);
                                        char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "%d", m.start);
                                        idx_res = strdup(nbuf);
                                        free_match(&m);
                                        break;
                                    }
                                }
                                free(cur_val); cur_val = idx_res;
                                free_pattern(&p); free(pat_str);
                            } else if (is_replace) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                while (j3 < len && 
# 2543 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2543 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2543 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2543 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView rep_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &rep_blk);
                                    char *rep_str = pap_process_sv(ctx, rep_blk);
                                    Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                    Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                    int replaced = 0;
                                    for (int s = 0; cur_val[s]; s++) {
                                        if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                            char *old_match = (char*)malloc((size_t)(m.end - m.start + 1));
                                            memcpy(old_match, cur_val + m.start, (size_t)(m.end - m.start));
                                            old_match[m.end - m.start] = '\0';
                                            char *r = apply_replacement_ex(rep_str, &m, sym);
                                            size_t rl = strlen(r), cl = strlen(cur_val);
                                            size_t new_len = (size_t)m.start + rl + (cl - (size_t)m.end);
                                            char *new_val = (char *)malloc(new_len + 1);
                                            memcpy(new_val, cur_val, (size_t)m.start);
                                            memcpy(new_val + m.start, r, rl);
                                            strcpy(new_val + m.start + rl, cur_val + m.end);


                                            if (klen > 0) pap_var_update(ctx, sym, src + ks, klen, new_val);

                                            free(cur_val); cur_val = old_match;
                                            free(new_val); free(r); free_match(&m);
                                            replaced = 1; break;
                                        }
                                    }
                                    if (!replaced) {
                                        free(cur_val); cur_val = strdup("");
                                    }
                                    free_pattern(&p); free(rep_str);
                                }
                                free(pat_str);
                            } else if (is_repeat) {
                                char *times_str = pap_process_sv(ctx, blk);
                                int times = atoi(times_str); free(times_str);
                                while (j3 < len && 
# 2581 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2581 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2581 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2581 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    for (int t = 0; t < times; t++) {
                                        char *tmp = pap_process_sv(ctx, code_blk);
                                        free(tmp);
                                    }
                                }
                                free(cur_val); cur_val = strdup("");
                            } else if (is_while) {
                                StrView pat_blk = blk;
                                char *pat_str = pap_process_sv(ctx, pat_blk);
                                Pattern pat; parse_pattern_ex(pat_str, &pat, sym);

                                while (j3 < len && 
# 2596 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2596 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2596 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2596 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    char *last_res = strdup("");
                                    while (1) {
                                        char *iter = pap_process_sv(ctx, code_blk);
                                        Match m; m.ctx = ctx;
                                        int matches = match_pattern(ctx, iter, (int)strlen(iter), &pat, 0, &m);
                                        if (matches) free_match(&m);

                                        if (!matches) { free(iter); break; }
                                        free(last_res); last_res = iter;
                                    }
                                    free(cur_val);
                                    cur_val = last_res;
                                }
                                free_pattern(&pat);
                                free(pat_str);
                            } else if (is_until) {
                                StrView pat_blk = blk;
                                char *pat_str = pap_process_sv(ctx, pat_blk);
                                Pattern pat; parse_pattern_ex(pat_str, &pat, sym);

                                while (j3 < len && 
# 2620 "src/papagaio.c" 3
                                                  ((*__ctype_b_loc ())[(int) ((
# 2620 "src/papagaio.c"
                                                  (unsigned char)src[j3]
# 2620 "src/papagaio.c" 3
                                                  ))] & (unsigned short int) _ISspace)
# 2620 "src/papagaio.c"
                                                                                 ) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    char *last_res = strdup("");
                                    while (1) {
                                        char *iter = pap_process_sv(ctx, code_blk);
                                        Match m; m.ctx = ctx;
                                        int matches = match_pattern(ctx, iter, (int)strlen(iter), &pat, 0, &m);
                                        if (matches) free_match(&m);

                                        if (matches) {
                                            free(last_res); last_res = iter;
                                            break;
                                        }
                                        free(last_res); last_res = iter;
                                    }
                                    free(cur_val);
                                    cur_val = last_res;
                                }
                                free_pattern(&pat);
                                free(pat_str);
                            }
                             if (arg) free(arg);
                            cp = j3;
                        }

                        sb_append_n(&out, cur_val, strlen(cur_val));
                        free(cur_val);
                        i = cp;
                        goto next_char;
                    }
                }

                int is_sym = (klen == 13 && memcmp(src + ks, "changesymbols", 13) == 0);

                if (is_sym) {
                    size_t saved_j = j;
                    while (j < len && 
# 2658 "src/papagaio.c" 3
                                     ((*__ctype_b_loc ())[(int) ((
# 2658 "src/papagaio.c"
                                     (unsigned char)src[j]
# 2658 "src/papagaio.c" 3
                                     ))] & (unsigned short int) _ISspace)
# 2658 "src/papagaio.c"
                                                                   ) j++;
                    if (j < len && src[j] == '{') {
                        StrView b1, b2, b3, b4;
                        if (is_sym) {
                            int next1 = extract_block(src, (int)j, (StrView){"{",1}, (StrView){"}",1}, &b1);
                            size_t j2 = (size_t)next1; while (j2 < len && 
# 2663 "src/papagaio.c" 3
                                                                         ((*__ctype_b_loc ())[(int) ((
# 2663 "src/papagaio.c"
                                                                         (unsigned char)src[j2]
# 2663 "src/papagaio.c" 3
                                                                         ))] & (unsigned short int) _ISspace)
# 2663 "src/papagaio.c"
                                                                                                        ) j2++;
                            if (j2 < len && src[j2] == '{') {
                                int next2 = extract_block(src, (int)j2, (StrView){"{",1}, (StrView){"}",1}, &b2);
                                size_t j3 = (size_t)next2; while (j3 < len && 
# 2666 "src/papagaio.c" 3
                                                                             ((*__ctype_b_loc ())[(int) ((
# 2666 "src/papagaio.c"
                                                                             (unsigned char)src[j3]
# 2666 "src/papagaio.c" 3
                                                                             ))] & (unsigned short int) _ISspace)
# 2666 "src/papagaio.c"
                                                                                                            ) j3++;
                                if (j3 < len && src[j3] == '{') {
                                    int next3 = extract_block(src, (int)j3, (StrView){"{",1}, (StrView){"}",1}, &b3);
                                    size_t j4 = (size_t)next3; while (j4 < len && 
# 2669 "src/papagaio.c" 3
                                                                                 ((*__ctype_b_loc ())[(int) ((
# 2669 "src/papagaio.c"
                                                                                 (unsigned char)src[j4]
# 2669 "src/papagaio.c" 3
                                                                                 ))] & (unsigned short int) _ISspace)
# 2669 "src/papagaio.c"
                                                                                                                ) j4++;
                                    if (j4 < len && src[j4] == '{') {
                                        int next4 = extract_block(src, (int)j4, (StrView){"{",1}, (StrView){"}",1}, &b4);
                                        StrView t1 = trim_sv(b1), t2 = trim_sv(b2), t3 = trim_sv(b3), t4 = trim_sv(b4);

                                        if (t1.len > 0 && t1.len < 16) {
                                            memcpy(sym->sigil, t1.ptr, t1.len); sym->sigil[t1.len] = '\0';
                                        }
                                        if (t2.len > 0 && t2.len < 16 && t3.len > 0 && t3.len < 16) {
                                            memcpy(sym->open, t2.ptr, t2.len); sym->open[t2.len] = '\0';
                                            memcpy(sym->close, t3.ptr, t3.len); sym->close[t3.len] = '\0';
                                        }
                                        if (t4.len > 0 && t4.len < 16) {
                                            memcpy(sym->optional, t4.ptr, t4.len);
                                            sym->optional[t4.len] = '\0';
                                        }
                                        i = (size_t)next4; continue;
                                    }
                                }
                            }
                        }
                    }
                    j = saved_j;
                }

                if (klen == 4 && memcmp(src + ks, "args", 4) == 0 && j < len && src[j] == '$') {
                    j++;
                    size_t start = j;
                    while (j < len && (
# 2697 "src/papagaio.c" 3
                                      ((*__ctype_b_loc ())[(int) ((
# 2697 "src/papagaio.c"
                                      (unsigned char)src[j]
# 2697 "src/papagaio.c" 3
                                      ))] & (unsigned short int) _ISalnum) 
# 2697 "src/papagaio.c"
                                                                     || src[j] == '_')) j++;
                    size_t vlen = j - start;
                    if (vlen > 0) {
                        int resolved = 0;
                        if (
# 2701 "src/papagaio.c" 3
                           ((*__ctype_b_loc ())[(int) ((
# 2701 "src/papagaio.c"
                           (unsigned char)src[start]
# 2701 "src/papagaio.c" 3
                           ))] & (unsigned short int) _ISdigit)
# 2701 "src/papagaio.c"
                                                             ) {

                            int idx = atoi(src + start);
                            if (ctx && idx >= 0 && (idx + 1) < ctx->argc) {
                                sb_append_n(&out, ctx->argv[idx + 1], strlen(ctx->argv[idx + 1]));
                                resolved = 1;
                            }
                        } else if (vlen == 5 && memcmp(src + start, "count", 5) == 0) {

                            char nbuf[32];
                            int count = ctx ? (ctx->argc > 1 ? ctx->argc - 1 : 0) : 0;
                            snprintf(nbuf, sizeof(nbuf), "%d", count);
                            sb_append_n(&out, nbuf, strlen(nbuf));
                            resolved = 1;
                        } else if (vlen == 3 && memcmp(src + start, "all", 3) == 0) {

                            if (ctx) {
                                for (int k = 2; k < ctx->argc; k++) {
                                    sb_append_n(&out, ctx->argv[k], strlen(ctx->argv[k]));
                                    if (k + 1 < ctx->argc) sb_append_char(&out, ' ');
                                }
                                resolved = 1;
                            }
                        } else {

                            if (ctx && ctx->argv) {
                                for (int k = ctx->argc - 1; k >= 0; k--) {
                                    const char *arg = ctx->argv[k];
                                    if (strncmp(arg, src + start, vlen) == 0 && arg[vlen] == '=') {

                                        const char *val = arg + vlen + 1;
                                        sb_append_n(&out, val, strlen(val));
                                        resolved = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        if (resolved) { i = j; continue; }

                        sb_append_n(&out, src + i, j - i);
                        i = j; goto next_char;
                    }
                }


                if (klen == 4 && memcmp(src + ks, "args", 4) == 0 &&
                    (j >= len || src[j] != '$')) {
                    if (ctx && ctx->argc > 2) {
                        for (int k = 2; k < ctx->argc; k++) {
                            sb_append_n(&out, ctx->argv[k], strlen(ctx->argv[k]));
                            if (k + 1 < ctx->argc) sb_append_char(&out, ' ');
                        }
                    }
                    i = j; goto next_char;
                }



                int is_cmd = 0;
                if (ctx) {
                    for (int ci = 0; ci < ctx->cmd_count; ci++) {
                        if (strlen(ctx->commands[ci].name) == klen &&
                            memcmp(ctx->commands[ci].name, src + ks, klen) == 0) {
                            is_cmd = 1; break;
                        }
                    }
                    if (!is_cmd && klen == 8 && memcmp(src + ks, "document", 8) == 0) is_cmd = 1;
                }

                if (!is_cmd && ctx && ctx->argv) {
                    for (int k = ctx->argc - 1; k >= 0; k--) {
                        const char *arg = ctx->argv[k];
                        if (strncmp(arg, src + ks, klen) == 0 && arg[klen] == '=') {

                            const char *val = arg + klen + 1;
                            sb_append_n(&out, val, strlen(val));
                            i = j; goto next_char;
                        }
                    }
                }
            }
        }
        sb_append_char(&out, src[i++]);
    next_char:;
    }
    return out.data;
}

static char *dispatch_commands(Papagaio *ctx, const char *src, const Symbols *sym)
{
    if (!ctx || !src) return src ? strdup(src) : 
# 2792 "src/papagaio.c" 3
                                                ((void *)0)
# 2792 "src/papagaio.c"
                                                    ;
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);
    size_t sl = strlen(sym->sigil);
    char sigil = sym->sigil[0];
    StrView so = { sym->open, strlen(sym->open) };
    StrView sc = { sym->close, strlen(sym->close) };

    while (i < len) {
        if (src[i] == sigil) {
            size_t j = i + sl;
            size_t ks = j;
            while (j < len && (
# 2804 "src/papagaio.c" 3
                              ((*__ctype_b_loc ())[(int) ((
# 2804 "src/papagaio.c"
                              (unsigned char)src[j]
# 2804 "src/papagaio.c" 3
                              ))] & (unsigned short int) _ISalnum) 
# 2804 "src/papagaio.c"
                                                             || src[j] == '_')) j++;
            size_t klen = j - ks;

            if (klen > 0) {
                int found = -1;
                for (int ci = 0; ci < ctx->cmd_count; ci++) {
                    if (strlen(ctx->commands[ci].name) == klen &&
                        memcmp(ctx->commands[ci].name, src + ks, klen) == 0) {
                        found = ci; break;
                    }
                }

                if (found >= 0) {
                    char *vargv[32]; size_t vargl[32]; int vargc = 0;
                    while (vargc < 32) {
                        size_t sj = j;
                        while(j < len && 
# 2820 "src/papagaio.c" 3
                                        ((*__ctype_b_loc ())[(int) ((
# 2820 "src/papagaio.c"
                                        (unsigned char)src[j]
# 2820 "src/papagaio.c" 3
                                        ))] & (unsigned short int) _ISspace)
# 2820 "src/papagaio.c"
                                                                      ) j++;
                        if (j < len && sv_pfx(src + j, so)) {
                            StrView blk; j = (size_t)extract_block(src, (int)j, so, sc, &blk);
                            char *arg = (char*)malloc(blk.len + 1);
                            if (arg) { memcpy(arg, blk.ptr, blk.len); arg[blk.len] = '\0'; }
                            vargv[vargc] = arg; vargl[vargc] = blk.len; vargc++;
                        } else { j = sj; break; }
                    }
                    RegisteredCommand *cmd = &ctx->commands[found];
                    char *res = cmd->handler(ctx, cmd->name, vargc, (const char **)vargv, vargl, cmd->userdata);
                    if (res) { sb_append_n(&out, res, strlen(res)); free(res); }
                    for (int ci = 0; ci < vargc; ci++) if (vargv[ci]) free(vargv[ci]);
                    i = j; continue;
                }


                if (klen == 8 && memcmp(src + ks, "document", 8) == 0) {
                    if (j < len && src[j] == '$') {
                        j++;
                        size_t s2 = j;
                        while (j < len && (
# 2840 "src/papagaio.c" 3
                                          ((*__ctype_b_loc ())[(int) ((
# 2840 "src/papagaio.c"
                                          (unsigned char)src[j]
# 2840 "src/papagaio.c" 3
                                          ))] & (unsigned short int) _ISalnum) 
# 2840 "src/papagaio.c"
                                                                         || src[j] == '_')) j++;
                        size_t vlen = j - s2;
                        if (vlen == 8 && memcmp(src + s2, "original", 8) == 0) {
                            if (ctx && ctx->original_doc) sb_append_n(&out, ctx->original_doc, strlen(ctx->original_doc));
                        } else if (vlen == 7 && memcmp(src + s2, "current", 7) == 0) {
                            sb_append_n(&out, src, strlen(src));
                        } else {
                            sb_append_n(&out, src, strlen(src));
                        }
                    } else {
                        sb_append_n(&out, src, strlen(src));
                    }
                    i = j; continue;
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }

    char *result = strdup(out.data);
    sb_free(&out);




    if (ctx && ctx->auto_export) {
        StrBuf out2; sb_init(&out2);
        size_t rl = strlen(result);
        for (size_t k = 0; k < rl; k++) {
            if (k + 9 < rl && memcmp(result + k, "papagaio_", 9) == 0) {


                int is_def = (k == 0 || result[k-1] == ' ' || result[k-1] == '\t' || result[k-1] == '\n' || result[k-1] == '*' || result[k-1] == '}');
                if (is_def) {


                    int looks_like_func = 0;
                    for (size_t m = k + 9; m < rl && m < k + 128; m++) {
                        if (result[m] == '(') { looks_like_func = 1; break; }
                        if (result[m] == ';' || result[m] == '=' || result[m] == '{') break;
                    }
                    if (looks_like_func) {
                        sb_append_n(&out2, "__attribute__((visibility(\"default\"))) ", 39);
                    }
                }
            }
            sb_append_char(&out2, result[k]);
        }
        free(result);
        result = strdup(out2.data);
        sb_free(&out2);
    }

    return result;
}

typedef struct {
    int priority;
    size_t start;
    size_t end;
    char *content;
    char *result;
    int is_priority_block;
    int original_index;
} PChunk;

static int compare_pchunks_priority(const void *a, const void *b) {
    const PChunk *ca = (const PChunk *)a;
    const PChunk *cb = (const PChunk *)b;
    if (ca->priority < cb->priority) return -1;
    if (ca->priority > cb->priority) return 1;
    return ca->original_index - cb->original_index;
}

static int compare_pchunks_original(const void *a, const void *b) {
    const PChunk *ca = (const PChunk *)a;
    const PChunk *cb = (const PChunk *)b;
    return (int)ca->original_index - (int)cb->original_index;
}

static char *handle_priorities(Papagaio *ctx, const char *src, size_t len, const Symbols *sym) {
    size_t i = 0;
    size_t last_pos = 0;
    size_t sl = strlen(sym->sigil);

    PChunk *chunks = 
# 2925 "src/papagaio.c" 3
                    ((void *)0)
# 2925 "src/papagaio.c"
                        ;
    int chunk_count = 0;
    int chunk_cap = 0;
    int found_any = 0;

    while (i < len) {
        if (i + sl + 8 + sl < len &&
            memcmp(src + i, sym->sigil, sl) == 0 &&
            memcmp(src + i + sl, "priority", 8) == 0 &&
            memcmp(src + i + sl + 8, sym->sigil, sl) == 0) {

            size_t ps = i;
            size_t j = i + sl + 8 + sl;
            int prio = 0;
            int has_prio_val = 0;
            if (j + 3 <= len && memcmp(src + j, "max", 3) == 0) {
                prio = 
# 2941 "src/papagaio.c" 3
                      (-0x7fffffff - 1) 
# 2941 "src/papagaio.c"
                              + 1;
                j += 3;
                has_prio_val = 1;
            } else if (j + 3 <= len && memcmp(src + j, "min", 3) == 0) {
                prio = 0x7fffffff 
# 2945 "src/papagaio.c"
                              - 1;
                j += 3;
                has_prio_val = 1;
            } else {
                int sign = 1;
                if (j < len && src[j] == '-') {
                    sign = -1;
                    j++;
                }
                while (j < len && 
# 2954 "src/papagaio.c" 3
                                 ((*__ctype_b_loc ())[(int) ((
# 2954 "src/papagaio.c"
                                 (unsigned char)src[j]
# 2954 "src/papagaio.c" 3
                                 ))] & (unsigned short int) _ISdigit)
# 2954 "src/papagaio.c"
                                                               ) {
                    prio = prio * 10 + (src[j] - '0');
                    j++;
                    has_prio_val = 1;
                }
                prio *= sign;
            }
            if (has_prio_val) {
                while (j < len && 
# 2962 "src/papagaio.c" 3
                                 ((*__ctype_b_loc ())[(int) ((
# 2962 "src/papagaio.c"
                                 (unsigned char)src[j]
# 2962 "src/papagaio.c" 3
                                 ))] & (unsigned short int) _ISspace)
# 2962 "src/papagaio.c"
                                                               ) j++;
                if (j < len && str_pfx(src + j, sym->open)) {
                    found_any = 1;


                    if (ps > last_pos) {
                        if (chunk_count >= chunk_cap) {
                            chunk_cap = chunk_cap ? chunk_cap * 2 : 8;
                            chunks = (PChunk *)realloc(chunks, sizeof(PChunk) * chunk_cap);
                        }
                        chunks[chunk_count].priority = 0x7fffffff 
# 2972 "src/papagaio.c"
                                                              - 1;
                        chunks[chunk_count].start = last_pos;
                        chunks[chunk_count].end = ps;
                        chunks[chunk_count].content = 
# 2975 "src/papagaio.c" 3
                                                     ((void *)0)
# 2975 "src/papagaio.c"
                                                         ;
                        chunks[chunk_count].result = 
# 2976 "src/papagaio.c" 3
                                                    ((void *)0)
# 2976 "src/papagaio.c"
                                                        ;
                        chunks[chunk_count].is_priority_block = 0;
                        chunks[chunk_count].original_index = chunk_count;
                        chunk_count++;
                    }

                    StrView v;
                    StrView so = { sym->open, strlen(sym->open) };
                    StrView sc = { sym->close, strlen(sym->close) };
                    int next = extract_block(src, (int)j, so, sc, &v);

                    if (chunk_count >= chunk_cap) {
                        chunk_cap = chunk_cap ? chunk_cap * 2 : 8;
                        chunks = (PChunk *)realloc(chunks, sizeof(PChunk) * chunk_cap);
                    }
                    chunks[chunk_count].priority = prio;
                    chunks[chunk_count].start = ps;
                    chunks[chunk_count].end = (size_t)next;
                    chunks[chunk_count].content = (char *)malloc(v.len + 1);
                    memcpy(chunks[chunk_count].content, v.ptr, v.len);
                    chunks[chunk_count].content[v.len] = '\0';
                    chunks[chunk_count].result = 
# 2997 "src/papagaio.c" 3
                                                ((void *)0)
# 2997 "src/papagaio.c"
                                                    ;
                    chunks[chunk_count].is_priority_block = 1;
                    chunks[chunk_count].original_index = chunk_count;
                    chunk_count++;

                    i = (size_t)next;
                    last_pos = i;
                    continue;
                }
            }
        }
        i++;
    }

    if (!found_any) {
        if (chunks) free(chunks);
        return 
# 3013 "src/papagaio.c" 3
              ((void *)0)
# 3013 "src/papagaio.c"
                  ;
    }


    if (last_pos < len) {
        if (chunk_count >= chunk_cap) {
            chunk_cap = chunk_cap ? chunk_cap * 2 : 8;
            chunks = (PChunk *)realloc(chunks, sizeof(PChunk) * chunk_cap);
        }
        chunks[chunk_count].priority = 0x7fffffff 
# 3022 "src/papagaio.c"
                                              - 1;
        chunks[chunk_count].start = last_pos;
        chunks[chunk_count].end = len;
        chunks[chunk_count].content = 
# 3025 "src/papagaio.c" 3
                                     ((void *)0)
# 3025 "src/papagaio.c"
                                         ;
        chunks[chunk_count].result = 
# 3026 "src/papagaio.c" 3
                                    ((void *)0)
# 3026 "src/papagaio.c"
                                        ;
        chunks[chunk_count].is_priority_block = 0;
        chunks[chunk_count].original_index = chunk_count;
        chunk_count++;
    }


    qsort(chunks, chunk_count, sizeof(PChunk), compare_pchunks_priority);


    ctx->disable_sandbox++;
    for (int j = 0; j < chunk_count; j++) {
        if (chunks[j].is_priority_block) {
            chunks[j].result = papagaio_process_text(ctx, chunks[j].content, strlen(chunks[j].content));
        } else {
            const char *chunk_text = src + chunks[j].start;
            size_t chunk_len = chunks[j].end - chunks[j].start;
            chunks[j].result = papagaio_process_text(ctx, chunk_text, chunk_len);
        }
    }
    ctx->disable_sandbox--;


    qsort(chunks, chunk_count, sizeof(PChunk), compare_pchunks_original);

    StrBuf out; sb_init(&out);
    for (int j = 0; j < chunk_count; j++) {
        if (chunks[j].result) {
            sb_append_n(&out, chunks[j].result, strlen(chunks[j].result));
            free(chunks[j].result);
        }
        if (chunks[j].content) free(chunks[j].content);
    }
    free(chunks);

    char *final_res = strdup(out.data);
    sb_free(&out);
    return final_res;
}

char *papagaio_process_text(Papagaio *ctx, const char *input, size_t len)
{
    if (!ctx || !input) return 
# 3068 "src/papagaio.c" 3
                              ((void *)0)
# 3068 "src/papagaio.c"
                                  ;
    Symbols sym = make_symbols("$", "{", "}");

    ctx->depth++;
    if (ctx->depth == 1) {
        clear_scope(ctx->current_scope);
        if (ctx->original_doc) free(ctx->original_doc);
        ctx->original_doc = (char*)malloc(len + 1);
        if (ctx->original_doc) {
            memcpy(ctx->original_doc, input, len);
            ctx->original_doc[len] = '\0';
            ctx->original_len = len;
        } else {
            ctx->original_len = 0;
        }
    } else if (!ctx->disable_sandbox) {
        push_scope(ctx);
    }


    char *prio_res = handle_priorities(ctx, input, len, &sym);
    if (prio_res) {
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return prio_res;
    }

    char *buf = (char *)malloc(len + 1);
    if (!buf) {
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return 
# 3099 "src/papagaio.c" 3
              ((void *)0)
# 3099 "src/papagaio.c"
                  ;
    }
    memcpy(buf, input, len); buf[len] = '\0';

    char *preprocessed = resolve_preprocessor(ctx, buf, &sym); free(buf);
    if (!preprocessed) {
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return 
# 3107 "src/papagaio.c" 3
              ((void *)0)
# 3107 "src/papagaio.c"
                  ;
    }


    PatternPair *new_pairs = 
# 3111 "src/papagaio.c" 3
                            ((void *)0)
# 3111 "src/papagaio.c"
                                ; int new_pc = 0;
    char *text_no_patterns = extract_nested(preprocessed, &sym, &new_pairs, &new_pc);
    free(preprocessed);
    if (!text_no_patterns) {
        free_pairs(new_pairs, new_pc);
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return 
# 3118 "src/papagaio.c" 3
              ((void *)0)
# 3118 "src/papagaio.c"
                  ;
    }

    if (new_pc > 0) {
        if (ctx->current_scope->rule_count + new_pc > ctx->current_scope->rule_cap) {
            ctx->current_scope->rule_cap = ctx->current_scope->rule_cap ? (ctx->current_scope->rule_cap + new_pc) * 2 : (new_pc + 8);
            ctx->current_scope->rules = (PatternPair *)realloc(ctx->current_scope->rules, sizeof(PatternPair) * ctx->current_scope->rule_cap);
        }
        for (int i = 0; i < new_pc; i++) {
            ctx->current_scope->rules[ctx->current_scope->rule_count++] = new_pairs[i];
        }
        free(new_pairs);
    }

    char *cur = text_no_patterns;

    int scope_count = 0;
    for (Scope *s = ctx->current_scope; s; s = s->parent) scope_count++;
    Scope **scopes = (Scope **)malloc(sizeof(Scope*) * scope_count);
    int s_idx = scope_count - 1;
    for (Scope *s = ctx->current_scope; s; s = s->parent) scopes[s_idx--] = s;


    for (int si = 0; si < scope_count; si++) {
        Scope *s = scopes[si];

        for (int i = 0; i < s->rule_count; i++) {

            if (ctx->disable_patterns) {
                int is_aliases_rule = (s->rules[i].m &&
                                       s->rules[i].m[0] == '$' &&
                                       s->rules[i].m[1] == '$' &&
                                       
# 3150 "src/papagaio.c" 3
                                      _Generic (0 ? (
# 3150 "src/papagaio.c"
                                      s->rules[i].m
# 3150 "src/papagaio.c" 3
                                      ) : (void *) 1, const void *: (const char *) (strstr (
# 3150 "src/papagaio.c"
                                      s->rules[i].m
# 3150 "src/papagaio.c" 3
                                      , 
# 3150 "src/papagaio.c"
                                      "$aliases{"
# 3150 "src/papagaio.c" 3
                                      )), default: strstr (
# 3150 "src/papagaio.c"
                                      s->rules[i].m
# 3150 "src/papagaio.c" 3
                                      , 
# 3150 "src/papagaio.c"
                                      "$aliases{"
# 3150 "src/papagaio.c" 3
                                      )) 
# 3150 "src/papagaio.c"
                                                                         != 
# 3150 "src/papagaio.c" 3
                                                                            ((void *)0)
# 3150 "src/papagaio.c"
                                                                                );
                if (!is_aliases_rule) continue;
            }

            StrBuf out; sb_init(&out);
            size_t clen = strlen(cur), pos = 0;
            Pattern pat;
            parse_pattern_ex(s->rules[i].m, &pat, &sym);

            while (pos < clen) {
                Match m; m.ctx = ctx;
                if (match_pattern(ctx, cur, (int)clen, &pat, (int)pos, &m)) {
                    char *r = apply_replacement_ex(s->rules[i].r, &m, &sym);
                    int is_aliases_rule = (s->rules[i].m &&
                                           s->rules[i].m[0] == '$' &&
                                           s->rules[i].m[1] == '$' &&
                                           
# 3166 "src/papagaio.c" 3
                                          _Generic (0 ? (
# 3166 "src/papagaio.c"
                                          s->rules[i].m
# 3166 "src/papagaio.c" 3
                                          ) : (void *) 1, const void *: (const char *) (strstr (
# 3166 "src/papagaio.c"
                                          s->rules[i].m
# 3166 "src/papagaio.c" 3
                                          , 
# 3166 "src/papagaio.c"
                                          "$aliases{"
# 3166 "src/papagaio.c" 3
                                          )), default: strstr (
# 3166 "src/papagaio.c"
                                          s->rules[i].m
# 3166 "src/papagaio.c" 3
                                          , 
# 3166 "src/papagaio.c"
                                          "$aliases{"
# 3166 "src/papagaio.c" 3
                                          )) 
# 3166 "src/papagaio.c"
                                                                             != 
# 3166 "src/papagaio.c" 3
                                                                                ((void *)0)
# 3166 "src/papagaio.c"
                                                                                    );
                    if (is_aliases_rule) {
                        sb_append_n(&out, r, strlen(r));
                    } else {
                        ctx->disable_patterns++;
                        char *evaluated_r = papagaio_process_text(ctx, r, strlen(r));
                        ctx->disable_patterns--;
                        sb_append_n(&out, evaluated_r, strlen(evaluated_r));
                        free(evaluated_r);
                    }
                    free(r);
                    pos = (size_t)m.end;
                    free_match(&m);
                } else {
                    sb_append_char(&out, cur[pos++]);
                }
            }
            free_pattern(&pat);
            char *next_cur = strdup(out.data);
            sb_free(&out);
            free(cur);
            cur = next_cur;
        }
    }
    free(scopes);

    char *final = dispatch_commands(ctx, cur, &sym);
    free(cur);

    if (ctx->depth > 1 && !ctx->disable_sandbox) {
        pop_scope(ctx);
    }
    ctx->depth--;
    return final;
}
