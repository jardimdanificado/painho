i32 counter = 0;

constructor void init() {
    counter = 100;
}

noreturn void stop() {
    for(;;);
}

export i32 get_counter() {
    return counter;
}

export void do_stop(i32 x) {
    if (x > 0) stop();
}
