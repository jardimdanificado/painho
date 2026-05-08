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
- **Trailing Sigil (whitespace collapse)**: appending a bare `$` (or the current sigil) directly after any variable or literal causes the matcher to **consume all following whitespace** in the input — making the adjacent `TOK_WS` optional. This is useful when the number of spaces between tokens is variable:
  ```text
  $pattern {$a$ $b} {$a/$b}
  hello   world   → hello/world
  ```
  The trailing `$` after `$a` collapses any run of spaces/tabs/newlines between `$a` and `$b`.

### Braced Variables

When a captured variable name needs to be immediately followed by literal text (e.g., a suffix), wrap the name in `${...}` to prevent ambiguity:

```text
$pattern {$id$word} {${id}x}
foo
```
*Output: `foox`* — without braces, `$idx` would be parsed as a single variable named `idx`.

Braced syntax can be used in any replacement string:
```text
$pattern {$first $last} {Hello, ${first}!
}
John Doe
```
*Output: `Hello, John!`*

### Nesting
Modifiers support full recursive nesting:
```text
$pattern {$n$aliases{$x$int}{abc}} {VALUE: $n}
```

---

## Extensibility (Wasm Plugins)

Papagaio follows a Wasm-first plugin architecture. Core features are limited to pattern matching and transformation, while custom text processing capabilities are provided by WebAssembly plugins.

### Built-in Operators
- **`$document`**: Injects the current state of the document (alias for `$document$current`).
- **`$document$original`**: Injects the initial, unprocessed input text. Useful for referencing the source even after multiple transformations.
- **`$document$current`**: Injects the current state of the document during the pre-processing pass.
- **`$wasm{path}`**: Loads a WebAssembly plugin from the file system (CLI only).
- **$file{path}**: Injects the content of a file from the file system (CLI only).
- **`$wat{source}`**: Compiles a WebAssembly Text Format (WAT) source string inline and registers all exported `papagaio_*` functions as commands. Useful for embedding lightweight plugins without an external `.wasm` file.
- **`$NAME$from{value}`**: Dynamically assigns a processed `value` to `$NAME`. The assignment itself is suppressed from the output, and the variable becomes available for exact-match replacement in the remaining document.

  ```text
  $NAME$from{Alice}
  Hello, $NAME!
  ```
  *Output: `Hello, Alice!`*

  ```text
  $wat{
    (module
      (func (export "papagaio_hello") (result i32)
        i32.const 42))
  }
  $hello
  ```

---

## CLI Argument Expansion

Papagaio can resolve command-line arguments directly within your source files. This is useful for passing configuration, flags, or metadata into the processing pipeline.

### Positional Arguments
The `argv` array maps as follows (where `argv[0]` is the binary name, invisible to Papagaio):

| Variable | Value |
|---|---|
| `$args$0` | `argv[1]` — the input file/script name |
| `$args$1`, `$args$2`, … | Subsequent positional arguments |
| `$args$count` | Total number of arguments (excludes the binary name, `argv[0]`) |
| `$args$all` | All extra arguments from index 1 onwards (after the script), joined with spaces |

If a `$args$NAME` variable is not found, it is emitted **literally** (e.g. `$args$missing` stays as-is).

### Named Variables (Overrides)
Arguments in the format `key=value` are automatically parsed and can be accessed in two ways:
1. **Explicit**: `$args$key`
2. **Direct**: `$key` (shorthand for `$args$key`)

Direct access (`$key`) will only resolve if `key` does not conflict with a registered command (like `$wasm`) or a built-in directive.

#### Example:
```sh
./papagaio input.c version=1.2.3 target=wasm -O3 > ready.c
# Then compile ready.c with clang to WASM
```
Inside `input.c`:
```c
const char *v = "$version"; // "1.2.3"
const char *t = "$target";  // "wasm"
const char *f = "$args$1";  // "-O3"
```

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

## Recursive Priority System

Papagaio allows you to control the order of execution and side-effects (such as pattern definitions or WASM loading) using the **`$priority$N`** directive.

- **`$priority$0{...}`**: Maximum priority.
- **`$priority$max{...}`**: Alias for `INT_MIN + 1` (Absolute highest priority).
- **`$priority$min{...}`**: Alias for `INT_MAX - 1` (Absolute lowest priority).
- **`$priority$1`, `$priority$2`, ...**: Successively lower priorities.
- **Recursive Evaluation**: Blocks with higher numerical priority (lower value) are fully processed — including their own nested patterns and commands — before any lower-priority blocks, regardless of their physical position in the file.
- **Unspecified Priority**: Any text not wrapped in a `$priority` block is treated as priority `INT_MAX - 1` (processed last).

#### Example:
```text
$priority$1{ Result: A }
$priority$max{ $pattern{A}{OK} }
```
*Output: `Result: OK`* — even though `A` is used before being defined in the source, the `$priority$max` block ensures the pattern definition happens first.

---

## Dynamic Variable Assignment ($from)

The **`$from`** operator allows you to capture processed content and assign it to a variable at runtime. This turns Papagaio into a stateful processor where variables can be defined, redefined, and chained.

### Syntax
`$NAME$from{...content...}`

1. **Recursive Processing**: The `content` is fully processed (patterns, WASM commands, other assignments) before being stored.
2. **Immediate Registration**: The variable is registered as an exact-match rule as soon as it is parsed. This allows for **chained assignments**.
3. **Output Suppression**: The entire `$from` directive is removed from the output text.

### Examples

#### Chained Assignments
Variables can depend on previously defined variables within the same document:
```text
$A$from{1}
$B$from{$A$A}
Count: $B
```
*Output: `Count: 11`*

#### Nested Assignments
You can define internal variables while defining a larger block:
```text
$GREET$from{
  $USER$from{Alice}
  Hello, $USER!
}
$GREET
```
*Output: `Hello, Alice!`*

#### Interaction with Patterns
Assignments can be used to dynamically generate pattern keys or replacements:
```text
$KEY$from{FOO}
$pattern{$KEY}{BAR}
Result: FOO
```
*Output: `Result: BAR`*

---

## Plugin Development

Papagaio features a modern, frictionless Wasm plugin system. You can write plugins in standard C using simple naming conventions and compile them into zero-dependency WebAssembly modules using `papagaio` as a preprocessor and `clang` as the WASM compiler.

### 1. Write your plugin
Create a file named `greet.c`:
```c
// Functions starting with 'papagaio_' are automatically registered as commands
char* papagaio_greet(int argc, char **argv)
{
    if (argc < 1) return "Hello, Stranger!";
    return argv[0]; // return first argument
}
```

To use the Papagaio Wasm SDK (`lib.c`), copy it from `examples/lib.c` into your project and include it explicitly:
```c
#include "lib.c"

char* papagaio_greet(int argc, char **argv)
{
    if (argc < 1) return "Hello, Stranger!";
    
    // lib.c provides standard C functions like malloc and sprintf
    char *res = (char*)malloc(strlen(argv[0]) + 16);
    sprintf(res, "Hello, %s!", argv[0]);
    
    return res;
}
```

### 2. Compile to WebAssembly
To compile your plugin, first process it with `papagaio` and then compile the result with `clang` using the following flags:

```sh
# 1. Preprocess with Papagaio
./papagaio greet.c > greet.ready.c

# 2. Compile to WASM with Clang
clang --target=wasm32-unknown-unknown -ffreestanding -nostdlib -fvisibility=hidden \
      -Wl,--no-entry,--export-dynamic,--allow-undefined \
      greet.ready.c -o greet.wasm
```

Or simply use the provided Makefile in the `examples/` directory:
```sh
cd examples && make
```

### 3. Use in Papagaio
Loading the Wasm file automatically registers all exported commands.
```text
$wasm{greet.wasm}
$greet{Papagaio}
```
*Output: Hello, Papagaio!*

### Wasm SDK (lib.c)
The Wasm SDK lives at `examples/lib.c` inside the repository. It provides a curated, zero-dependency C standard library for WebAssembly, including:
- **Memory Management**: `malloc`, `free`, `realloc`
- **String Processing**: `strlen`, `strcpy`, `sprintf`, `strrev`, etc.
- **Formatted I/O**: `printf`, `snprintf`, `sscanf`
- **Standard Math**: `sin`, `cos`, `pow`, etc.

---

## Building

```sh
make            # Core & CLI
make wasm       # WebAssembly build (Papagaio in the browser/node)
make test       # Run comprehensive test suite
```

## References

- [cpp](https://en.wikipedia.org/wiki/C_preprocessor)
- [m4](https://www.gnu.org/software/m4/)
- [libregexp](https://bellard.org/quickjs/)
- [quickjs](https://bellard.org/quickjs/)
- [tcc](https://bellard.org/tcc/)
- [wasm3](https://github.com/wasm3/wasm3)
- [watr](https://github.com/dy/watr)