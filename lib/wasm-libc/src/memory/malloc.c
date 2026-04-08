#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* 
 * Um alocador simples (First-Fit, Coalescência Imediata) para WebAssembly.
 * Memória layout: [header (8b)] [payload] [footer (4b)]
 * Alinhamento: 8 bytes.
 */

/* Entry point do heap (vindo do linker ou definido aqui) */
extern char __heap_base;

/* Configurações de alinhamento */
#define ALIGN           8U
#define ALIGN_MASK      (ALIGN - 1)
#define ALIGN_UP(x)     (((size_t)(x) + ALIGN_MASK) & ~ALIGN_MASK)

#define HDR_SIZE        8U      /* size_flags, next_free */
#define FTR_SIZE        4U      /* sizeof(uint32_t) footer */
#define OVERHEAD        (HDR_SIZE + FTR_SIZE)
#define MIN_PAYLOAD     8U
#define MIN_BLOCK_SIZE  ALIGN_UP(OVERHEAD + MIN_PAYLOAD)

#define SIZE_MASK       (~(uint32_t)0x7U)
#define FLAG_USED       0x1U

/* Macros de acesso */
#define BLOCK_USED(h)   ((h)->size_flags & FLAG_USED)
#define BLOCK_SIZE(h)   ((h)->size_flags & SIZE_MASK)
#define BLOCK_PAYLOAD(h) ((void *)((char *)(h) + HDR_SIZE))
#define PAYLOAD_BLOCK(p) ((block_hdr_t *)((char *)(p) - HDR_SIZE))
#define BLOCK_FOOTER(h) ((uint32_t *)((char *)(h) + BLOCK_SIZE(h) - FTR_SIZE))
#define NEXT_BLOCK(h)   ((block_hdr_t *)((char *)(h) + BLOCK_SIZE(h)))
#define PREV_BLOCK(h)   ((block_hdr_t *)((char *)(h) - (*((uint32_t *)((char *)(h) - FTR_SIZE)) & SIZE_MASK)))

/* ── estrutura de bloco ──────────────────────────────────────────────────── */

typedef struct block_hdr {
    uint32_t           size_flags;  /* size | flags nos 3 bits baixos (alinhado a 8) */
    struct block_hdr  *next_free;   /* só válido em blocos livres */
} block_hdr_t;

/* ── Estado Global ───────────────────────────────────────────────────────── */

static block_hdr_t *free_list  = (void *)0;
static char        *heap_end   = (void *)0;
static int          initialized = 0;

/* ── helpers ─────────────────────────────────────────────────────────────── */

__attribute__((import_module("env"), import_name("__host_abort")))
extern void __host_abort(const char *msg);

#define WASM_PAGE_SIZE 65536

static inline uint32_t wasm_memory_size(void) {
    return __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
}

static inline uint32_t wasm_memory_grow(uint32_t pages) {
    return __builtin_wasm_memory_grow(0, pages);
}

static void block_set(block_hdr_t *h, size_t size, int used) {
    h->size_flags = (uint32_t)size | (used ? FLAG_USED : 0);
    *BLOCK_FOOTER(h) = h->size_flags;
}

static void free_list_insert(block_hdr_t *b) {
    b->next_free = free_list;
    free_list = b;
}

static void free_list_remove(block_hdr_t *b) {
    block_hdr_t **cur = &free_list;
    while (*cur) {
        if (*cur == b) {
            *cur = b->next_free;
            return;
        }
        cur = &((*cur)->next_free);
    }
}

static block_hdr_t *coalesce(block_hdr_t *b) {
    size_t size = BLOCK_SIZE(b);
    
    /* tenta fundir com o próximo se ele estiver no heap e livre */
    block_hdr_t *next = NEXT_BLOCK(b);
    if ((char *)next < heap_end && !BLOCK_USED(next)) {
        size += BLOCK_SIZE(next);
        free_list_remove(next);
    }

    /* tenta fundir com o anterior */
    /* can_prev: se b não é o primeiro bloco do heap */
    char *heap_start = (char *)ALIGN_UP(&__heap_base);
    if ((char *)b > heap_start) {
        block_hdr_t *prev = PREV_BLOCK(b);
        if (!BLOCK_USED(prev)) {
            size += BLOCK_SIZE(prev);
            free_list_remove(prev);
            b = prev;
        }
    }

    block_set(b, size, 0);
    return b;
}

static block_hdr_t *heap_grow(size_t size) {
    size_t pages = (size + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE;
    uint32_t old_pages = wasm_memory_grow((uint32_t)pages);
    if (old_pages == (uint32_t)-1) return (void *)0;

    char *new_mem = (char *)(uintptr_t)(old_pages * WASM_PAGE_SIZE);
    size_t new_size = pages * WASM_PAGE_SIZE;
    
    block_hdr_t *b = (block_hdr_t *)new_mem;
    block_set(b, new_size, 0);
    heap_end = new_mem + new_size;

    return b;
}

static void heap_init(void) {
    char *base = (char *)ALIGN_UP(&__heap_base);
    uint32_t cur_size = wasm_memory_size();
    
    if ((size_t)cur_size > (size_t)base + MIN_BLOCK_SIZE) {
        size_t available = (size_t)cur_size - (size_t)base;
        block_hdr_t *b = (block_hdr_t *)base;
        block_set(b, available & ~ALIGN_MASK, 0);
        free_list_insert(b);
        heap_end = base + (available & ~ALIGN_MASK);
    } else {
        /* heap inicial é muito pequeno, cresce */
        block_hdr_t *b = heap_grow(MIN_BLOCK_SIZE);
        if (b) free_list_insert(b);
    }
    initialized = 1;
}

/* ── API pública ─────────────────────────────────────────────────────────── */

void *malloc(size_t size) {
    if (size == 0) return (void *)0;
    if (!initialized) heap_init();

    size_t block_size = ALIGN_UP(size + OVERHEAD);
    if (block_size < MIN_BLOCK_SIZE) block_size = MIN_BLOCK_SIZE;

    block_hdr_t **cur = &free_list;
    while (*cur) {
        block_hdr_t *b = *cur;
        size_t bsz = BLOCK_SIZE(b);

        if (bsz >= block_size) {
            /* encontrou! desvincula da lista */
            *cur = b->next_free;

            /* divide o bloco se sobrar espaço suficiente */
            if (bsz - block_size >= MIN_BLOCK_SIZE) {
                block_hdr_t *split = (block_hdr_t *)((char *)b + block_size);
                block_set(split, bsz - block_size, 0);
                free_list_insert(split);
                block_set(b, block_size, 1);
            } else {
                block_set(b, bsz, 1);
            }

            return BLOCK_PAYLOAD(b);
        }
        cur = &b->next_free;
    }

    /* OOM na free_list, tenta crescer heap */
    block_hdr_t *b = heap_grow(block_size);
    if (!b) return (void *)0;
    
    /* tenta alocar novamente do novo espaço */
    free_list_insert(b);
    return malloc(size);
}

void free(void *ptr) {
    if (!ptr) return;
    block_hdr_t *b = PAYLOAD_BLOCK(ptr);
    if (!BLOCK_USED(b)) return; /* evitar double-free simples */

    b = coalesce(b);
    free_list_insert(b);
}

void *calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) return (void *)0;
    size_t total = nmemb * size;
    if (total / nmemb != size) return (void *)0; /* overflow */

    void *ptr = malloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return (void *)0; }

    block_hdr_t *b = PAYLOAD_BLOCK(ptr);
    size_t bsz = BLOCK_SIZE(b);
    size_t old_pay = bsz - OVERHEAD;

    if (size <= old_pay) return ptr;

    void *new_ptr = malloc(size);
    if (!new_ptr) return (void *)0;

    memcpy(new_ptr, ptr, old_pay);
    free(ptr);
    return new_ptr;
}
