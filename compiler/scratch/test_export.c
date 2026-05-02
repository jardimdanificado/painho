export i32 add(i32 a, i32 b) {
    return a + b;
}

i32 global_var = 42;

i32 main() {
    return add(global_var, 1);
}
