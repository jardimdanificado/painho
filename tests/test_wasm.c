typedef unsigned int  uint32_t;
typedef unsigned long size_t;
extern unsigned char __heap_base;
static unsigned char *hptr;
void *malloc(size_t n) {
    if (!hptr) hptr = &__heap_base;
    void *p = hptr; hptr += (n + 7) & ~7; return p;
}
void free(void *p) { (void)p; }

int atoi(const char *s) {
    int n = 0, sign = 1;
    if (*s == '-') { sign = -1; s++; }
    while (*s >= '0' && *s <= '9') { n = n * 10 + (*s - '0'); s++; }
    return n * sign;
}

/* argv_ptr is a Wasm offset to an i32 table of string offsets */
static const char *_pap_get_arg(uint32_t argv_ptr, int i) {
    uint32_t *table = (uint32_t *)argv_ptr;
    return (const char *)table[i];
}

/* Implementation */
char* intern_cmd_test_mul(int argc, const char **argv) {
    if (argc < 2) return "";
    int a = atoi(argv[0]);
    int b = atoi(argv[1]);
    int res = a * b;
    
    char *out = (char*)malloc(32);
    out[0] = '\0';
    
    /* Simple integer to string */
    char buf[32];
    int i = 0, is_neg = 0;
    if (res == 0) { buf[i++] = '0'; }
    if (res < 0) { is_neg = 1; res = -res; }
    while (res > 0) { buf[i++] = (res % 10) + '0'; res /= 10; }
    if (is_neg) { buf[i++] = '-'; }
    
    int len = i;
    for (int j = 0; j < len; j++) out[j] = buf[len - 1 - j];
    out[len] = '\0';
    return out;
}

/* Bridge - called by host with (argc, argv_table_ptr) */
__attribute__((export_name("papagaio_test_mul")))
char* pap_bridge_test_mul(int argc, uint32_t argv_ptr) {
    const char **argv = (const char **)malloc(argc * sizeof(const char *));
    for (int i = 0; i < argc; i++) argv[i] = _pap_get_arg(argv_ptr, i);
    char *res = intern_cmd_test_mul(argc, argv);
    free((void*)argv);
    return res;
}
