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

$wasm_plugin{test_mul}{
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
