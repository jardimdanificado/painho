const i32 __stack_size = 4096;

at(0x1000) u32 framebuffer[64 * 64];
at(0x5000) i32 global_counter = 0;

entrypoint void init_system() {
    global_counter = 1;
}

export void draw_pixel(i32 x, i32 y, u32 color) {
    framebuffer[y * 64 + x] = color;
}
