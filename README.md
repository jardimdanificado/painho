# Papagaio

Papagaio is a C-first, embeddable text processing engine. It is designed to be highly modular and **script-agnostic**, allowing core pattern matching to be used alone or extended via WebAssembly (Wasm) plugins.

## Key Features

- **Lightweight Core**: Efficient C engine for pattern matching and transformation.
- **Pattern-Matching**: Powerful capture system with built-in and custom modifiers.
- **WebAssembly Plugins**: Highly secure, zero-dependency plugin architecture via an embedded `wasm3` runtime.
- **Configurable Delimiters**: Redefine sigils, delimiters, and markers at runtime.
- **Language Bindings**: Native usage in C and Node.js/WebAssembly.

## Quick Start

### Command Line Interface (CLI)
```sh
# Process with patterns defined in the file or passed via -p
papagaio -e '$pattern {hello $w} {Hi $w}' input.txt
```

### C API
```c
Papagaio *ctx = papagaio_open();

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

## Extensibility (Wasm Plugins)

Papagaio follows a Wasm-first plugin architecture. Core features are limited to pattern matching and transformation, while custom text processing capabilities are provided by WebAssembly plugins.

### Built-in Operators
- **$document**: Injects the current state of the document.
- **$wasmfile{path}**: Loads a WebAssembly plugin into the current context. All exported functions prefixed with `pap_cmd_` are automatically registered as commands.

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

Papagaio includes an embedded `wasm3` runtime for highly secure, zero-dependency plugins (Bare Metal Wasm). Wasm plugins are dynamically loaded and executed in pure isolation, communicating with the host entirely through memory serialization.

1. **Write the plugin using the generator**:
    Save this as `my_plugin.c.pap`:
    ```c
    $wasm_plugin{greet}{
        if (argc < 1) return "";
        const char *name = argv[0];
        
        size_t sz = strlen(name) + 8;
        char *res = (char*)malloc(sz);
        res[0] = '\0';
        strcat(res, "Hello, ");
        strcat(res, name);
        return res;
    }
    ```

2. **Generate the C code**:
    ```sh
    ./papagaio my_plugin.c.pap > plugin.c
    ```

3. **Compile the Bare Metal Wasm module**:
    No standard library or runtime imports are needed. Use standard `clang` targeting `wasm32`:
    ```sh
    clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export-all -o plugin.wasm plugin.c
    ```

4. **Load and Use in Papagaio**:
    Loading the Wasm file automatically registers exported commands starting with `pap_cmd_`.
    ```text
    $wasmfile{plugin.wasm} $greet{World}
    ```
    *Output: Hello, World*

---

## Building

```sh
make        # Core & CLI
make wasm   # WebAssembly build
make test   # Run comprehensive test suite
```

## References

- [CPP](https://en.wikipedia.org/wiki/C_preprocessor)
- [m4](https://www.gnu.org/software/m4/)
- [libregexp](https://bellard.org/quickjs/)
- [QuickJS](https://bellard.org/quickjs/)
- [TCC](https://bellard.org/tcc/)
- [Wasm3](https://github.com/wasm3/wasm3)
- [minilua](https://github.com/edubart/minilua)
- [Lua](https://www.lua.org/)
- [Terra](http://terralang.org/)