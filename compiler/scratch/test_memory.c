extern i8 __heap_base;

export i32 get_heap_start() {
    return (i32)&__heap_base;
}

export void write_to_heap(i32 offset, i8 val) {
    i8 *p = (i8*)&__heap_base;
    p[offset] = val;
}
