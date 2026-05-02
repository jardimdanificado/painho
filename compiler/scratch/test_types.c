// Native types and basic arithmetic
export i32 test_arithmetic() {
    i8 a = 10;
    u32 b = 20;
    i64 c = 3000000000;
    f32 d = 1.5f;
    f64 e = 2.5;

    i32 result = (i32)a + (i32)b + (i32)(c / 100000000) + (i32)(d * e);
    return result; // 10 + 20 + 30 + 3 = 63
}

export f64 test_floats(f64 x, f64 y) {
    return x * y + 0.5;
}
