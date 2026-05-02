// Function modifiers and pointer usage
export noinline i32 add(i32 a, i32 b) {
    return a + b;
}

export void pointer_test() {
    i32 x = 10;
    i32* p = &x; // pointers use standard C syntax
    *p = 20;
}

constructor void my_init() {
    // This runs automatically at startup
}
