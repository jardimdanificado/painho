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
- **$wasmfile{path}**: Loads a WebAssembly plugin from the file system.
- **$wasm{base64_data}**: Loads a WebAssembly plugin directly from a Base64-encoded string. Useful for self-contained scripts or environments without a file system.

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

Papagaio features a modern, frictionless Wasm plugin system. With the **`papagaiocc`** standalone compiler, you can write plugins in C using a high-level syntax and compile them into zero-dependency WebAssembly modules.

### 1. Write your plugin
Create a file named `greet.c`:
```c
use "plibc";

// Export the function as a Papagaio command named "greet"
export hello as "greet"
{
    if (argc < 1) return "Hello, Stranger!";
    
    // plibc provides standard C functions like malloc and sprintf
    char *res = (char*)malloc(strlen(argv[0]) + 16);
    sprintf(res, "Hello, %s!", argv[0]);
    
    return res;
}
```

### 2. Compile with `papagaiocc`
The `papagaiocc` tool is a self-contained compiler driver that embeds its own SDK.
```sh
./papagaiocc greet.c
```
This generates `greet.wasm`.

### 3. Use in Papagaio
Loading the Wasm file automatically registers all exported commands.
```text
$wasmfile{greet.wasm}
$greet{Papagaio}
```
*Output: Hello, Papagaio!*

### Unified Wasm SDK (plibc)
The `use "plibc";` directive provides a curated, zero-dependency C standard library for WebAssembly, including:
- **Memory Management**: `malloc`, `free`, `realloc`
- **String Processing**: `strlen`, `strcpy`, `sprintf`, `strrev`, etc.
- **Formatted I/O**: `printf`, `snprintf`, `sscanf`
- **Standard Math**: `sin`, `cos`, `pow`, etc.

---

## Building

```sh
make            # Core & CLI
make papagaiocc # Standalone plugin compiler
make wasm       # WebAssembly build (Papagaio in the browser/node)
make test       # Run comprehensive test suite
```

## References

- [CPP](https://en.wikipedia.org/wiki/C_preprocessor)
- [m4](https://www.gnu.org/software/m4/)
- [libregexp](https://bellard.org/quickjs/)
- [QuickJS](https://bellard.org/quickjs/)
- [TCC](https://bellard.org/tcc/)
- [Wasm3](https://github.com/wasm3/wasm3)