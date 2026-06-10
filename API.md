# Papagaio & Louro API Documentation

Papagaio is built as a highly extensible text-processing engine written in C, fully exportable to JavaScript/Node.js and browsers via WebAssembly. It integrates the Louro mathematical and logical evaluation engine.

This document describes the API and CLI usage for both the C core and the JavaScript/Wasm integration.

---

## 1. C API (libpapagaio)

The C API allows you to embed Papagaio and Louro into any native application. It is primarily defined in `papagaio.h`.

### Lifecycle Management

```c
#include "papagaio.h"

// Create a new isolated execution context.
Papagaio *ctx = papagaio_open();

// Pass CLI arguments to the engine (makes them available via $args).
char *args[] = {"--mode", "release"};
papagaio_set_args(ctx, 2, args);

// Process a block of text. Returns a heap-allocated string that you must free.
const char *code = "$pattern{MACRO}{10} MACRO";
char *result = papagaio_process_text(ctx, code, strlen(code));
printf("Result: %s\n", result);
free(result);

// Destroy the context and free all associated memory.
papagaio_close(ctx);
```

### High-Level One-Shot Evaluation

If you don't need persistent context or custom extensions, you can use the quick one-shot functions:

```c
// Process text with inline pattern registration (NULL-terminated list of strings).
// Signature: papagaio_process(const char *input, const char *pat1, const char *rep1, ..., NULL);
char *res = papagaio_process("Hello $USER!", "$USER", "Alice", NULL);

// Process text with custom delimiters.
char *res2 = papagaio_process_ex("Hello ?USER!", "?", "[", "]", "?USER", "Bob", NULL);
```

### Extending Papagaio (Commands, Modifiers & Plugins)

You can register custom text-processing commands (e.g. `$mycmd{...}`) and modifiers (e.g. `$VAR$mymod{...}`).

#### Commands

```c
// Command Handler Signature
char *my_command(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata) {
    if (argc > 0) return strdup(argv[0]); // Echo first argument
    return strdup("");
}

// Registering the command
papagaio_register_command(ctx, "mycmd", my_command, NULL);
```

#### Modifiers

```c
// Modifier Handler Signature
// Return a heap-allocated string on success, NULL to reject the match.
char *my_modifier(const char *match, const char *modifier,
                   size_t match_len, size_t mod_len, void *userdata);

// Registering the modifier — usable in patterns as $var$mymod
papagaio_register_modifier(ctx, "mymod", my_modifier, NULL);

// Query and cleanup
papagaio_has_modifier(ctx, "mymod");   // → 1 (true)
papagaio_clear_modifiers(ctx);         // Remove all modifiers
```

#### Plugins

Papagaio supports loading native shared libraries at runtime via the `$import{path}` directive (**CLI-only**). The library must export:

```c
int papagaio_plugin_init(Papagaio *ctx);
```

Inside `init`, the plugin can register commands, modifiers, and finalizers:

```c
int papagaio_plugin_init(Papagaio *ctx) {
    papagaio_register_command(ctx, "mycmd", my_command, NULL);
    papagaio_register_modifier(ctx, "mymod", my_modifier, NULL);
    papagaio_add_finalizer(ctx, my_cleanup, NULL);
    return 0;
}
```

Build with:
```sh
cc -shared -fPIC -I/path/to/papagaio/src -o my_plugin.so my_plugin.c
```

Use in a template:
```text
$import{./my_plugin.so}
$pattern {$v$mymod} {[$v]}
Hello
```

### Extending Louro (Mathematical and Logical Operations)

The Louro engine supports registering custom functions, operators, and logical blocks directly into the mathematical evaluator. The macros automatically detect arity (0-16 arguments) and handle closure pointers.

```c
#include <math.h>

double my_custom_add(double a, double b) {
    return a + b;
}

// Register a standard function: $math{ my_add(10, 20) }
louro_register(ctx, "my_add", my_custom_add);

// Register an infix operator: $math{ 10 # 20 }
louro_register_infix(ctx, "#", my_custom_add, 15); // Precedence 15
```

---

## 2. JavaScript & WebAssembly API

The Wasm wrapper `papagaio.js` exposes an asynchronous class interface to interact with the engine from Node.js or browsers.

### Initialization and Execution

```javascript
import Papagaio from './dist/wasm/papagaio.js';

async function main() {
    const p = new Papagaio();
    await p.init(); // Wait for WebAssembly compilation

    // Inject command-line arguments
    p.setArgs(['--dev', '--verbose']);

    // Evaluate Papagaio code
    const code = `$pattern{NAME}{Alice} Hello $NAME!`;
    const result = p.process(code);
    
    console.log(result); // Outputs: Hello Alice!
}

main();
```

> **Note:** The current JS wrapper focuses on evaluation and argument passing. Native JS callback registration (to build custom `$commands` in JS) requires adding additional Emscripten glue code.

---

## 3. Command Line Interface (CLI)

The standard Papagaio binary (built via `make`) provides a fast native CLI for evaluating files.

### Basic Usage
```bash
# Evaluate a file and print to stdout
./papagaio template.pap

# Pass arguments to the script
./papagaio build.pap --release v1.0.0
```

### Accessing CLI Arguments Inside Papagaio
When you pass arguments to the CLI, they become available globally within the engine through the `$args` built-in variable list.

```text
// build.pap
$args$compare{}
$then{ No arguments provided! }
$else{
  Build mode: $args$list{ }$get{0}
  Version:    $args$list{ }$get{1}
}
```

Running `./papagaio build.pap release v1.0.0` will output:
```
  Build mode: release
  Version:    v1.0.0
```

### Real-time REPL
If you run `./papagaio` without any files or arguments (and without piping stdin), it drops into a lightweight interactive REPL (Read-Eval-Print Loop) for testing expressions.

---

## System Integration Details

- **Memory Safety:** Every text processing operation returns a newly allocated string (`char*`). It is the caller's absolute responsibility to `free()` it.
- **Thread Safety:** The engine `Papagaio *ctx` is deeply stateful. You should **not** share a single context across multiple threads concurrently. Create a separate context per thread.
- **Transpilation Security:** The engine prevents infinite macro expansion through deterministic passes. You can use `$normalize` if you explicitly require continuous recursive expansion.
