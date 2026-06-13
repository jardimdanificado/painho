# Papagaio

Papagaio is an embeddable text processing engine.

## Key Features

- **Lightweight Core**: Efficient C engine for pattern matching and transformation.
- **Pattern-Matching**: Powerful capture system with built-in and custom modifiers.
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

---

## Pattern Syntax

Patterns are composed of whitespace-separated tokens. The engine uses a "flex-matching" strategy that automatically skips horizontal whitespace between variables.

- **Literal**: Matches exact text.
- **Variable**: `$name` (captures a sequence up to the next pattern match).
- **Optional**: `$name?` or `literal?` (marker is configurable, e.g., `MAYBE`, via `$changesymbols`).

### Modifiers
Modifiers specify the data type or constraints of a match:
- **Numbers**: `$var$int`, `$var$float`, `$var$number`
- **Casing**: `$var$upper`, `$var$lower`, `$var$capitalized`
- **Formats**: `$var$word`, `$var$identifier`, `$var$hex`, `$var$path`, `$var$binary`, `$var$percent`
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

### Custom Modifiers

Custom modifiers extend the pattern engine with user-defined validation and transformation logic. They are registered via `papagaio_register_modifier()`:

```c
typedef char *(*PapModifierHandler)(const char *match, const char *modifier,
                                     size_t match_len, size_t mod_len,
                                     void *userdata);

int papagaio_register_modifier(Papagaio *ctx, const char *name,
                                PapModifierHandler handler, void *ud);
```

**Handler contract:**
- `match` — the raw captured text; return `NULL` to reject the match
- `modifier` — the full modifier name as written in the pattern (e.g. `"len_3_8"`)
- Return a **heap-allocated** string to store as the capture value (freed by the engine)
- Return `NULL` or an empty string to fail the match (like a typed modifier mismatch)

**Example plugin** (`examples/modifiers/papagaio_mod_alpha.c`):
```c
#include "papagaio.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *alpha_modifier(const char *match, const char *modifier,
                            size_t match_len, size_t mod_len, void *ud)
{
    (void)modifier; (void)mod_len; (void)ud;
    if (!match || match_len == 0) return NULL;
    for (size_t i = 0; i < match_len; i++)
        if (!isalpha((unsigned char)match[i])) return NULL;
    char *res = malloc(match_len + 1);
    for (size_t i = 0; i < match_len; i++)
        res[i] = toupper((unsigned char)match[i]);
    res[match_len] = '\0';
    return res;
}

int papagaio_plugin_init(Papagaio *ctx)
{
    return papagaio_register_modifier(ctx, "alpha", alpha_modifier, NULL);
}
```

Build as a shared library and load via `$import`:
```sh
cc -shared -fPIC -I/path/to/papagaio/src -o alpha.so alpha.c
```
```text
$import{./alpha.so}
$pattern {$v$alpha} {[$v]}
Hello
```
*Output: `[HELLO]`*

Modifier names support alphanumeric characters and underscores, enabling argument-passing through the name itself. The length-modifier plugin (`examples/modifiers/papagaio_mod_len.c`) uses this to encode min/max bounds:
```text
$import{./mod_len.so}
$pattern {$p$len_3_8} {[$p]}
abcdefg
```
*Output: `[abcdefg]`* (length 7 is within bounds)

Related C API functions:
- `papagaio_has_modifier(ctx, "name")` — check if a modifier is registered
- `papagaio_clear_modifiers(ctx)` — remove all registered modifiers

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

### Built-in Operators
- **`$document`**: Injects the current state of the document (alias for `$document$current`).
- **`$document$original`**: Injects the initial, unprocessed input text. Useful for referencing the source even after multiple transformations.
- **`$document$current`**: Injects the current state of the document during the pre-processing pass.
- **`$include{path}`**: Injects the content of a file from the file system. Path is relative to the working directory.
- **`$NAME$from{value}`**: Dynamically assigns a processed `value` to `$NAME`. The assignment itself is suppressed from the output, and the variable becomes available for exact-match replacement in the remaining document.

  ```text
  $NAME$from{Alice}
  Hello, $NAME!
  ```
  *Output: `Hello, Alice!`*

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

Direct access (`$key`) will only resolve if `key` does not conflict with a registered command or a built-in directive.

#### Example:
```sh
# Then compile ready.c with clang
```
Inside `input.c`:
```c
const char *v = "$version"; // "1.2.3"
const char *t = "$target";  // "wasm"
const char *f = "$args$1";  // "-O3"
```

---

## Plugin System (`$import`)

Papagaio supports native shared-library plugins loaded at runtime via the **`$import{path}`** directive. This is a **CLI-only** feature (requires `./papagaio` or a program that calls `papagaio_set_cli_mode(ctx, 1)`). When used outside of CLI mode, `$import` emits literally.

### How it works

1. The engine calls `dlopen()` (POSIX) or `LoadLibrary()` (Windows) to load the specified `.so`/`.dll`.
2. It looks up the symbol `papagaio_plugin_init` and calls it with the current `Papagaio *ctx`.
3. The plugin can then register custom commands, modifiers, and finalizers using the C API.

### Plugin entry point

```c
#include "papagaio.h"

int papagaio_plugin_init(Papagaio *ctx);
```

### Building a plugin

```sh
cc -shared -fPIC -I/path/to/papagaio/src -o my_plugin.so my_plugin.c
```

### Usage in a template

```text
$import{./examples/modifiers/alpha.so}
$pattern {$v$alpha} {[$v]}
Hello
```
*Output: `[HELLO]`*

The path argument is **processed recursively** before loading, so you can use variables:

```text
$PLUGIN_DIR$from{./examples}
$import{$PLUGIN_DIR/modifiers/alpha.so}
```

### Plugin capabilities

Plugins can call any API function on the provided `ctx`:

| Function | Purpose |
|---|---|
| `papagaio_register_command(ctx, name, handler, ud)` | Register a `$name{args}` command |
| `papagaio_register_modifier(ctx, name, handler, ud)` | Register a `$var$name` modifier |
| `papagaio_register_math_generic(ctx, name, func, arity, ...)` | Register a `$math` function |
| `papagaio_add_finalizer(ctx, fn, ud)` | Register cleanup called at `papagaio_close` |
| `papagaio_clear_commands(ctx)` | Remove all registered commands |
| `papagaio_clear_modifiers(ctx)` | Remove all registered modifiers |

### Built-in plugins

Example plugins live in `examples/`:

| Plugin | Description |
|---|---|
| `examples/modifiers/alpha.so` | Matches only alphabetic text, uppercases |
| `examples/modifiers/alphanum.so` | Matches only alphanumeric text |
| `examples/modifiers/email.so` | Validates and extracts email addresses |
| `examples/modifiers/len.so` | Validates text length with min/max bounds |
| `examples/tcc/papagaio_tcc.so` | JIT-compile C code at runtime via TCC |
| `examples/math/papagaio_math.so` | Advanced mathematical and logical evaluation |
| `examples/wasm/papagaio_wasm.so` | Execute WebAssembly plugins inside Papagaio |

Build all modifier plugins:
```sh
make -C examples/modifiers clean all
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

## Dynamic Built-in Operators

Papagaio provides several built-in operators that give you access to the engine's internal syntax configuration and allow you to precisely inject unrepresentable characters (such as whitespace and binary ASCII) directly into patterns or output text. 

> [!NOTE]
> All built-in operators support the **trailing sigil** syntax (e.g., `$sigil$`). This allows the operator to immediately consume all following whitespace or safely concatenate with adjacent alphanumeric text, exactly like standard variables.

### Reserved Symbol Emitters
When generating macros or writing complex rules, you may need to emit a literal sigil without it being evaluated. Instead of complex double-escaping, you can use:
- **`$sigil`**: Emits the current sigil (e.g., `$`)
- **`$open`**: Emits the current open delimiter (e.g., `{`)
- **`$close`**: Emits the current close delimiter (e.g., `}`)
- **`$marker`**: Emits the current optional marker (e.g., `?`)

**Solving the Infinite Interpretation Problem:**
```text
$pattern {hello} {world $sigil$A$sigil$from{X}}
hello
```
*Output: `world $A$from{X}`* — Since `$sigil` is processed during the single initial pass, it emits a literal `$` that safely bypasses any subsequent evaluation.

### Formatting and Binary Control
For precise layout control, especially inside flex-matched `$pattern` rules that normally skip extra whitespace:
- **`$space`**: Emits a literal space character (`' '`)
- **`$newline`**: Emits a literal newline (`\n`)
- **`$tab`**: Emits a literal tab character (`\t`)

To generate specific binary characters (such as null bytes) or handle complex ASCII injection:
- **`$ascii$code`** (Inline): E.g., `$ascii$65` outputs `A`
- **`$ascii{code}`** (Block): E.g., `$ascii{0}` outputs a binary null byte (`\0`)


## Dynamic Variable Assignment ($from)

The **`$from`** operator allows you to capture processed content and assign it to a variable at runtime. This turns Papagaio into a stateful processor where variables can be defined, redefined, and chained.

### Syntax
`$NAME$from{...content...}`

1. **Recursive Processing**: The `content` is fully processed (patterns, other assignments) before being stored.
2. **Immediate Registration**: The variable is registered as an exact-match rule as soon as it is parsed. This allows for **chained assignments**.
3. **Output Suppression**: The entire `$from` directive is removed from the output text.

### Lexical Scopes and Sandboxing

Every block operator or preprocessing field evaluated dynamically (such as inside `$from`, `$then`, `$else`, `$while`, `$repeat`, and `$until`) is executed in its own **local nested scope**.

- **Sandboxing**: Any variable declared inside a local scope (e.g. `$B$from{local_val}` inside `$A$from{...}`) that does *not* exist in a parent scope is treated as a local variable. It will be completely destroyed and freed when the block finishes evaluating (sandboxed).
- **Shadowing & Upward Updates**: If a variable updated inside a local scope already exists in a parent/ancestor scope, Papagaio avoids shadowing it. Instead, it propagates the update upwards, modifying the existing variable in the parent scope.
- **Recursive Nesting**: Local scopes can be nested arbitrarily. Each child scope has full read/write access to variables in parent scopes, but parent or sibling scopes do not have access to variables declared exclusively in child scopes.

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

## List Operations (`$list`)

Any variable can be treated as a list by accessing it through the `$list` modifier chain. The **separator** can be any string (single character or multi-character) and is itself **processed** before use, allowing dynamic separators.

### Syntax

```
$VARNAME$list{separator}$OPERATION{...arguments}
```

### Operations

| Operation | Signature | Emits | Mutates |
|---|---|---|---|
| `get` | `$V$list{sep}$get{idx}` | Element at index | No |
| `set` | `$V$list{sep}$set{idx}{content}` | Nothing | Yes |
| `push` | `$V$list{sep}$push{content}` | Nothing | Yes |
| `pop` | `$V$list{sep}$pop` | Last element | Yes |
| `shift` | `$V$list{sep}$shift` | First element | Yes |
| `unshift` | `$V$list{sep}$unshift{content}` | Nothing | Yes |
| `insert` | `$V$list{sep}$insert{idx}{content}` | Nothing | Yes |
| `remove` | `$V$list{sep}$remove{idx}` | Nothing | Yes |
| `swap` | `$V$list{sep}$swap{idx_a}{idx_b}` | Nothing | Yes |
| `reverse` | `$V$list{sep}$reverse` | Nothing | Yes |
| `count` | `$V$list{sep}$count` | Number of elements | No |
| `join` | `$V$list{sep_orig}$join{sep_new}` | List with new separator | No |
| `slice` | `$V$list{sep}$slice{start}{end}` | Sub-list from start to end | No |
| `find` | `$V$list{sep}$find{pat}` | First **whole element** matching `pat` | No |
| `contains` | `$V$list{sep}$contains{pat}` | Index of `pat` **within** matching element | No |
| `replace` | `$V$list{sep}$replace{pat}{rep}` | First match found; updates the **whole element** | Yes |

**Index rules**: zero-based; negative indices count from the end (`-1` = last); out-of-range access emits `""` silently.

### Examples

```text
$FRUITS$from{apple,banana,orange}

$FRUITS$list{,}$get{0}    → apple
$FRUITS$list{,}$get{-1}   → orange
$FRUITS$list{,}$count     → 3
```

```text
$L$from{a,b,c}
$L$list{,}$push{d}
$L$list{,}$set{1}{B}
$L                        → a,B,c,d
```

```text
$STACK$from{x,y,z}
Popped: $STACK$list{,}$pop
Rest: $STACK              → Popped: z  /  Rest: x,y
```

```text
$CSV$from{one,two,three}
$CSV$list{,}$join{ | }    → one | two | three
```

```text
$PATH$from{/usr/local/bin}
$PATH$list{/}$get{-1}     → bin
```

```text
/* Dynamic separator from variable */
$SEP$from{,}
$L$from{x,y,z}
$L$list{$SEP}$get{2}      → z

$L$from{a,b,c,d,e}
$L$list{,}$slice{1}{4}    → b,c,d
```

---

---

## Mathematical and Logical Evaluation Plugin (`$math`)

Papagaio provides an optional plugin powered by the **Louro Engine** for deterministic, AOT-capable mathematical and logical evaluation. Load it via `$import{./examples/math/papagaio_math.so}`.

### Syntax
```
$math{ expression }
```

### Features
- **Arithmetic Operators:** `+`, `-`, `*`, `/`, `%` (modulo), `^` (exponentiation)
- **Logical Operators:** `&&` (AND), `||` (OR), `!` (NOT)
- **Comparisons:** `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Conditionals:** `if X then Y else Z end` (lazy evaluation)
- **Variables:** Variables can be injected into math blocks by name or via `$pattern` substitution.
- **Functions:** Includes a full standard math library (`sqrt`, `sin`, `cos`, `tan`, `log`, `exp`, `ceil`, `floor`, `abs`, `pi`, etc.)

### Examples
```text
$math{ 2 + 3 * 4 }             → 14
$math{ if 10 > 5 then 1 else 0 end } → 1
$math{ sqrt(16) * pi() }       → 12.56637...
```

---


## Flow Control Operators

Papagaio provides operators for conditional logic and value chaining. These are treated as **suffix modifiers** that can be appended to any variable, list operation, or expression.

### Syntax

```
$VAL$compare{target}
$VAL$find{pattern}
$VAL$contains{pattern}
$VAL$replace{pattern}{replacement}
$VAL$slice{start}{end}
$VAL$then{content}
$VAL$else{content}
```

| Operator | Behavior |
|---|---|
| **`compare`** | If `$VAL` matches `target`, emits `$VAL`. Otherwise, emits `""`. |
| **`find`** | Performs a non-anchored search for `pattern` in `$VAL`. Emits the matched substring. |
| **`contains`** | Performs a non-anchored search. Emits the character index of the first match (or `""`). |
| **`replace`** | Replaces the first match of `pattern` with `replacement`. Emits the OLD match. |
| **`slice`** | Returns a substring from `start` to `end`. Supports negative indices. |
| **`then`** | If `$VAL` is **not empty**, processes and emits `content`. Otherwise, emits `""`. |
| **`else`** | If `$VAL` **is empty**, processes and emits `content`. Otherwise, passes `$VAL` through. |
| **`repeat`** | `$repeat{N}{code}` | Executes `code` N times. Emits nothing; used for side effects. |
| **`while`** | `$while{pat}{code}` | Executes `code` while its result matches `pat`. Emits the last successful result. |
| **`until`** | `$until{pat}{code}` | Executes `code` until its result matches `pat`. Emits the match that caused the break. |
| **`byte`** | `$byte{code}` | Appends a byte (0-255) to the variable or current stream. |

### Chaining (If-Then-Else)

Operators can be chained to create complex conditional logic. The output of one operator becomes the input for the next.

#### Basic If-Then:
```text
$A$from{hello}
$A$compare{hello}$then{Matched!}   → Matched!
$A$compare{world}$then{Matched!}   → (empty)
```

#### If-Then-Else Pattern:
```text
$A$from{abc}
$A$compare{abc}$then{YES}$else{NO} → YES
$A$compare{xyz}$then{YES}$else{NO} → NO

#### Search and Extract:
```text
$A$from{user_id: 12345}
$A$find{$d+}$                → 12345
$A$contains{id}             → 5
$A$replace{$d+}{HIDDEN} $A  → 12345 user_id: HIDDEN

#### Slicing:
```text
$A$from{hello world}
$A$slice{0}{5}              → hello
$A$slice{-5}                → world
```
```
```

### Standalone and Braced Usage

- **Standalone**: If used without a preceding variable (e.g., `$else{default}`), the input is assumed to be an empty string.
- **Braced**: You can pipe arbitrary braced expressions into flow operators: `${some content}$then{has content!}`.

#### Example:
```text
$L$from{a,b,c}
$R$from{$L$list{,}$get{0}}
$R$compare{a}$then{Is A}$else{Not A} → Is A
```

---

---

## Building

```sh
make            # Core & CLI
make wasm       # WebAssembly build (via Emscripten)
make test       # Run comprehensive test suite
```

---

## System Limits

| Feature | Limit | Rationale / Detail |
|---|---|---|
| **Symbol Length** | 15 characters | Sigils, delimiters (`{`, `}`), and markers are stored in fixed 16-byte buffers. |
| **String Size** | Unlimited | All internal buffers (`StrBuf`) use dynamic `realloc`. Limited only by available RAM. |
| **Pattern Count** | Unlimited | Registered rules are stored in a dynamic array. |
| **Priority Range** | `INT_MIN` to `INT_MAX` | Priorities are handled as standard signed integers. |
| **Recursion Depth** | Stack-limited | Deeply nested patterns or priority blocks are processed recursively. |

## References

- [cpp](https://en.wikipedia.org/wiki/C_preprocessor)
- [m4](https://www.gnu.org/software/m4/)