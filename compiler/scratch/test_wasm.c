export i32 main() {
    i32 a = 10;
    i8 b = 5;
    u64 c = 100;
    
    if (a > b) {
        a = a + (i32)b;
    }
    
    for (i32 i = 0; i < 10; i = i + 1) {
        a = a + 1;
    }
    
    return a;
}
