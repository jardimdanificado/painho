# Papagaio

Papagaio is a C-first, embeddable text processing engine. It is designed to be highly modular and **script-agnostic**, allowing core pattern matching to be used alone or extended via third-party plugins (e.g., Lua).

## Key Features

- **Lightweight Core**: Efficient C engine for pattern matching and transformation.
- **Pattern-Matching**: Powerful capture system with built-in and custom modifiers.
- **Modular Plugins**: Extend the language with custom commands.
- **Optional Scripting**: High-level logic via `$lua` (or other script plugins) with multi-block arguments.
- **Configurable Delimiters**: Redefine sigils, delimiters, and markers at runtime.
- **Language Bindings**: Native usage in C, Lua (as a library), and Node.js/WebAssembly.

## Quick Start

### Command Line Interface (CLI)
```sh
# Process with patterns defined in the file or passed via -p
papagaio -e '$pattern {hello $w} {Hi $w}' input.txt
```

### C API
```c
Papagaio *ctx = papagaio_open();
// Load optional plugins
papagaio_load_plugin(ctx, "plugins/lua.so"); 

char *out = papagaio_process_text(ctx, input_text, strlen(input_text));
printf("%s", out);
free(out);
papagaio_close(ctx);
```

### JavaScript / WASM (Node.js)
```javascript
import Papagaio from './papagaio.js';

const p = new Papagaio();
await p.init();
p.registerCommand("mycmd", (name, ...args) => `Result: ${args[0]}`);

console.log(p.process('$mycmd{Hello}')); // Output: Result: Hello
```

---

## Pattern Syntax

Patterns are composed of whitespace-separated tokens. The engine uses a "flex-matching" strategy that automatically skips horizontal whitespace between variables.

- **Literal**: Matches exact text.
- **Variable**: `$name` (captures a sequence up to the next pattern match).
- **Optional**: `$name?` or `literal?` (marker is configurable, e.g., `MAYBE`, via `$changesymbols`).
- **Escaping**: Use `$$` to match a literal `$`.

### Modifiers
Modifiers specify the data type or constraints of a match:
- **Numbers**: `$var$int`, `$var$float`, `$var$number`
- **Casing**: `$var$upper`, `$var$lower`, `$var$capitalized`
- **Formats**: `$var$word`, `$var$identifier`, `$var$hex`, `$var$path`, `$var$binary`, `$var$percent`
- **Regex**: `$id$regex{[0-9]+}`
- **Block**: `$item$block{[}{]}` (captures everything between delimiters)
- **Aliases**: `$kind$aliases{cat}{dog}{bird}` (multi-block syntax).
- **Substrings**: `$var$starts{foo}`, `$var$ends{bar}`, `$var$prefix{p}`, `$var$suffix{s}`, `$var$infix{i}`, `$var$includes{x}`
- **Grouping**: `$item$group{subpattern}` (recursive grouping, matches as one unit)
- **Optionality**: any token (literal, variable, or group) can be made optional by adding `?` (or a custom marker like `MAYBE`).

### Nesting
Modifiers support full recursive nesting:
```text
$pattern {$n$aliases{$x$int}{abc}} {VALUE: $n}
```

---

## Scripting & Extensibility

Papagaio follows a plugin-first architecture. Core features are limited to pattern matching and transformation, while scripting capabilities like Lua are provided by modular plugins.

### Scripting Blocks (`$lua`)
The `$lua` command is provided by the **Lua Plugin**. It allows executing logic via multi-block arguments:
```text
$lua{ return params[1] .. params[2] }{HELLO}{WORLD}
```
- **Isolation**: Scripts operate in a clean environment; no automatic shared state (like legacy `match`/`content` variables).
- **Arguments**: Script blocks are passed to the `params` table (1-indexed in Lua). 

### Built-in Operators
- **$document**: Injects the current state of the document.
- **$import{path}**: Loads a plugin (`.so`) or a script (`.lua`) into the current context.

---

## Dynamic Customization

You can redefine the engine's syntax symbols at runtime using the atomic **`$changesymbols`** directive.

### `$changesymbols{sigil}{open}{close}{optional}`
Default: `$changesymbols{$}{{} }{}}{?}`

Example:
```text
$changesymbols{@}{<}{>}{!} @pattern <@n!> <ID: @n> [x] [y]
```
This changes the sigil to `@`, delimiters to `< >`, and the optional marker to `!`. Preprocessor directives (like `$changesymbols` itself) always use the stable `$` and `{}` to remain functional.

---

## Plugin Development

### Custom Registry (Lua)
You can extend Papagaio directly from script-land:
```lua
-- In a script loaded via $import or $lua
papagaio.register("square", function(x)
  local n = tonumber(x)
  return tostring(n * n)
end)
```
Usage: `$square{4}` -> `16`.

### C Plugin Interface
Plugins export a standard entry point:
```c
int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx);
```
Handlers receive the command name and a variadic list of arguments:
```c
typedef char *(*PapCommandHandler)(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);
```

---

## Creating Plugins

Papagaio can be extended native C plugins.

### Native Plugins
For maximum performance or system integration, use C plugins.

1.  **Implement the handler** (`hello.c`):
    ```c
    #include "papagaio_plugin.h"
    #include <string.h>

    static char *hello_handler(Papagaio *ctx, const char *name, int argc, 
                              const char **argv, const size_t *argl, void *ud) {
        return strdup("Hello from native C!");
    }

    int papagaio_plugin_init(PapPlugin *p, Papagaio *ctx) {
        p->register_command(p, "chello", hello_handler, NULL);
        return 0;
    }
    ```
2.  **Compile and load**:
    ```sh
    cc -shared -fPIC -I./src -o hello.so hello.c
    ```
    Usage: `$import{./hello.so} $chello{}`

### Handling Multiple Arguments
Handlers receive raw blocks of data. Since these are not guaranteed to be null-terminated (to support binary data), always use `argl`:

```c
static char *repeat_handler(Papagaio *ctx, const char *name, int argc, 
                            const char **argv, const size_t *argl, void *ud) {
    if (argc < 2) return strdup("");
    
    // Convert first argument (count)
    int count = atoi(argv[1]); // argv pointers are safe within their block size
    if (count <= 0) return strdup("");

    // Create result buffer
    size_t char_len = argl[0];
    char *res = malloc(char_len * count + 1);
    for (int i = 0; i < count; i++) {
        memcpy(res + (i * char_len), argv[0], char_len);
    }
    res[char_len * count] = '\0';
    return res;
}
```
*   `argc`: Number of blocks passed (ex: `$cmd{a}{b}` has `argc = 2`).
*   `argv`: Array of pointers to the start of each block.
*   `argl`: Array of lengths for each block.
*   **Safety**: If you need a C-string, you MUST copy `argv[i]` and null-terminate it yourself.
*   **Memory**: Always return a `malloc`'d string (or `strdup`). The engine calls `free()`.

### Lua Plugin Functions
An easier way to add new commands is using `papagaio.register` within a script in the lua plugin.

1.  **Create your plugin** (`greet.lua`):
    ```lua
    papagaio.register("hello", function(name)
      return "Hello, " .. (name or "stranger") .. "!"
    end)
    ```
2.  **Load and use it**:
    ```text
    $import{greet.lua} $hello{Papagaio}
    ```
    *Output: Hello, Papagaio!*


---

## Building

```sh
make        # Core & Plugins
make wasm   # WebAssembly build
make test   # Run comprehensive test suite (60 tests)
```

## References

- [libregexp](https://bellard.org/quickjs/)
- [QuickJS](https://bellard.org/quickjs/)
- [TCC](https://bellard.org/tcc/)
- [minilua](https://github.com/edubart/minilua)
- [Lua](https://www.lua.org/)
- [CPP](https://en.wikipedia.org/wiki/C_preprocessor)
- [m4](https://www.gnu.org/software/m4/)
- [Terra](http://terralang.org/)

---
*Papagaio: Efficient, scriptable text processing.*