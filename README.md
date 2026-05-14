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

### Built-in Operators
- **`$document`**: Injects the current state of the document (alias for `$document$current`).
- **`$document$original`**: Injects the initial, unprocessed input text. Useful for referencing the source even after multiple transformations.
- **`$document$current`**: Injects the current state of the document during the pre-processing pass.
- **$file{path}**: Injects the content of a file from the file system.
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

Papagaio allows you to control the order of execution and side-effects (such as pattern definitions) using the **`$priority$N`** directive.

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

1. **Recursive Processing**: The `content` is fully processed (patterns, other assignments) before being stored.
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

## List Operations (`$list`)

Any variable can be treated as a list by accessing it through the `$list` modifier chain. The **separator** can be any string (single character or multi-character) and is itself **processed** before use, allowing dynamic separators.

### Syntax

```
$VARNAME$list$OPERATION{separator}{...arguments}
```

### Operations

| Operation | Signature | Emits | Mutates |
|---|---|---|---|
| `get` | `$V$list$get{sep}{idx}` | Element at index | No |
| `set` | `$V$list$set{sep}{idx}{content}` | Nothing | Yes |
| `push` | `$V$list$push{sep}{content}` | Nothing | Yes |
| `pop` | `$V$list$pop{sep}` | Last element | Yes |
| `shift` | `$V$list$shift{sep}` | First element | Yes |
| `unshift` | `$V$list$unshift{sep}{content}` | Nothing | Yes |
| `insert` | `$V$list$insert{sep}{idx}{content}` | Nothing | Yes |
| `remove` | `$V$list$remove{sep}{idx}` | Nothing | Yes |
| `swap` | `$V$list$swap{sep}{idx_a}{idx_b}` | Nothing | Yes |
| `reverse` | `$V$list$reverse{sep}` | Nothing | Yes |
| `count` | `$V$list$count{sep}` | Number of elements | No |
| `join` | `$V$list$join{sep_orig}{sep_new}` | List with new separator | No |

**Index rules**: zero-based; negative indices count from the end (`-1` = last); out-of-range access emits `""` silently.

### Examples

```text
$FRUITS$from{apple,banana,orange}

$FRUITS$list$get{,}{0}    → apple
$FRUITS$list$get{,}{-1}   → orange
$FRUITS$list$count{,}     → 3
```

```text
$L$from{a,b,c}
$L$list$push{,}{d}
$L$list$set{,}{1}{B}
$L                        → a,B,c,d
```

```text
$STACK$from{x,y,z}
Popped: $STACK$list$pop{,}
Rest: $STACK              → Popped: z  /  Rest: x,y
```

```text
$CSV$from{one,two,three}
$CSV$list$join{,}{ | }    → one | two | three
```

```text
$PATH$from{/usr/local/bin}
$PATH$list$get{/}{-1}     → bin
```

```text
/* Dynamic separator from variable */
$SEP$from{,}
$L$from{x,y,z}
$L$list$get{$SEP}{2}      → z
```

---

## Flow Control Operators

Papagaio provides operators for conditional logic and value chaining. These are treated as **suffix modifiers** that can be appended to any variable, list operation, or expression.

### Syntax

```
$VAL$compare{target}
$VAL$then{content}
$VAL$else{content}
```

| Operator | Behavior |
|---|---|
| **`compare`** | If `$VAL` matches `target`, emits `$VAL`. Otherwise, emits `""`. |
| **`then`** | If `$VAL` is **not empty**, processes and emits `content`. Otherwise, emits `""`. |
| **`else`** | If `$VAL` **is empty**, processes and emits `content`. Otherwise, passes `$VAL` through. |

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
```

### Standalone and Braced Usage

- **Standalone**: If used without a preceding variable (e.g., `$else{default}`), the input is assumed to be an empty string.
- **Braced**: You can pipe arbitrary braced expressions into flow operators: `${some content}$then{has content!}`.

#### Example:
```text
$L$from{a,b,c}
$R$from{$L$list$get{,}{0}}
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
- [tcc](https://bellard.org/tcc/)