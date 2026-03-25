# Papagaio

Papagaio is a C-first, embeddable text processing and pattern-matching preprocessor. It provides a highly flexible pattern and replacement engine designed for rapid source preprocessing, code generation, and templating.

Papagaio is highly modular, supporting dynamic script-based extensions via Lua and WebAssembly/JavaScript.

## Key Features

- **Pattern-Matching Engine**: Powerful capture system with built-in and custom modifiers.
- **Modular Plugins**: Extend the language with custom commands and scripts.
- **Variadic Scripting**: Execute complex logic in-process via `$lua` (or other script plugins) with multi-block arguments.
- **Highly Configurable**: Redefine the entire syntax (sigils, delimiters, markers) at runtime.
- **Cross-Platform**: Seamless usage in C, Lua, and Node.js/WebAssembly.

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
- **Optional**: `$name?` or `literal?` (marker is configurable via `$changesymbols`).
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

### Nesting
Modifiers support full recursive nesting:
```text
$pattern {$n$aliases{$x$int}{abc}} {VALUE: $n}
```

---

## Replacement & Scripting

### Variable Interpolation
Use `$name` in the replacement string to insert captured content.

### Scripting Blocks (`$lua`)
Papagaio uses script plugins for dynamic transformations. The `$lua` command (if the Lua plugin is loaded) supports multiple blocks:
```text
$lua{ return params[1] .. params[2] }{HELLO}{WORLD}
```
- **Arguments**: Script blocks are passed to the `params` table (1-indexed in Lua). 
- **Isolation**: Scripts operate in a clean environment without automatic shared state (like legacy `match` or `content` variables).

### Built-in Operators
- **$document**: Injects the current state of the preprocessed document into the output.

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