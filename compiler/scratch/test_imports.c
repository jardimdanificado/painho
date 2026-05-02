// Importing functions from a WASM host module
import_module("env") import_name("js_print")
void print(const i8* s);

import_module("graphics") import_name("clear_screen")
void clear(u32 color);

export void run_test() {
    clear(0xFF0000FF);
    print("Hello from Papagaio-C!");
}
