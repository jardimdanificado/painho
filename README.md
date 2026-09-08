# Papagaio 🦜

> **JavaScript is the language. Papagaio is the textual extension.**

Papagaio is a lightweight, zero-dependency engine for **text processing, structural pattern matching, and template interpolation**. It seamlessly extends `String.prototype` in modern ECMAScript (**ES2023**), engineered specifically for strict portability across **Node.js, Bun, Deno, modern browsers, and embedded runtimes like QuickJS**.

Rather than reinventing programming language constructs like arrays, math, and control flow inside strings, Papagaio delegates general computation to native JavaScript and focuses purely on what it excels at:
- **Declarative pattern matching with automatic flex-whitespace handling**
- **Balanced nested block extraction without regular expression limitations**
- **Semantic modifiers and typed token capture (`$id$identifier`, `$val$int`, `$color$hex`)**
- **S-expression code transformation and syntax desugaring (e.g. compiling custom syntaxes to WebAssembly / WAT)**

---

## Table of Contents

1. [Architecture & Philosophy](#architecture--philosophy)
2. [Installation & Setup](#installation--setup)
3. [Public API Reference](#public-api-reference)
   - [String.prototype & Functional API](#stringprototype--functional-api)
   - [Interpolation & JS Expressions](#interpolation--js-expressions)
   - [Pattern Matching (`.match()`)](#pattern-matching-match)
   - [Pattern Replacement (`.replace()`)](#pattern-replacement-replace)
   - [Ahead-of-Time Compilation (`.compile()`)](#ahead-of-time-compilation-compile)
4. [Pattern Matching Syntax & Formal Semantics](#pattern-matching-syntax--formal-semantics)
   - [Token Kinds](#token-kinds)
   - [Flex-Matching Mechanics](#flex-matching-mechanics)
   - [Trailing Sigil (Whitespace Collapse)](#trailing-sigil-whitespace-collapse)
   - [Optionality (`?`)](#optionality-)
   - [Balanced Delimited Blocks (`$var$block{open}{close}`)](#balanced-delimited-blocks-varblockopenclose)
5. [Modifiers Reference](#modifiers-reference)
   - [Built-in Modifiers](#built-in-modifiers)
   - [Structural & Positional Modifiers](#structural--positional-modifiers)
   - [Custom Modifiers API](#custom-modifiers-api)
6. [Configuration & Custom Delimiters](#configuration--custom-delimiters)
7. [Advanced Use Case: Custom Syntax to WebAssembly (WAT)](#advanced-use-case-custom-syntax-to-webassembly-wat)
8. [Command Line Interface (CLI)](#command-line-interface-cli)
9. [Legacy Compatibility Example](#legacy-compatibility-example)
10. [Test Suite & Verification](#test-suite--verification)
11. [License](#license)

---

## Architecture & Philosophy

Earlier versions of Papagaio attempted to bundle flow control, list manipulation, math evaluators, and dynamic modules into a custom textual macro syntax. The modern reimagination establishes a clean boundary:

$$\text{JavaScript} \longrightarrow \text{Logic, expressions, flow control, arrays, math, closures}$$
$$\text{Papagaio} \longrightarrow \text{Pattern matching, structural captures, balanced blocks, template desugaring}$$

### Core Tenets
- **ES2023 Standard**: Uses standard modern JavaScript features with clean fallbacks.
- **QuickJS Ready**: Zero external `npm` dependencies. Does not rely on Node-only internals like `fs` or `process` in the library core.
- **Duality**: Every method is callable either fluently on string instances (`"hello $w".papagaio(...)`) or as pure standalone functions (`papagaio("hello $w", ...)`).

---

## Installation & Setup

Install via npm:

```sh
npm install papagaio
```

### Importing

#### 1. Idiomatic (Installs `String.prototype.papagaio`)
```javascript
import "papagaio";

// String instances now have .papagaio and submethods:
const out = "Hello $name".papagaio({ name: "World" });
```

#### 2. Functional / Modular (Without global prototype mutation)
```javascript
import { papagaio } from "papagaio";

const out = papagaio("Hello $name", { name: "World" });
const captures = papagaio.match("rgb($r,$g,$b)", "rgb(255,128,0)");
```

---

## Public API Reference

### String.prototype & Functional API

The `papagaio` property installed on `String.prototype` is a **callable function-object** that doubles as a namespace for specialized pattern operations:

| Method Signature | Prototype Equivalent | Description |
|---|---|---|
| `papagaio(template, context, options)` | `template.papagaio(context, options)` | Interpolates `$var` and `${expr}` using `context`. |
| `papagaio.match(pattern, input, options)` | `pattern.papagaio.match(input, options)` | Matches `input` against `pattern`. Returns `Record<string, string>` or `null`. |
| `papagaio.replace(pattern, input, replacement, options)` | `pattern.papagaio.replace(input, replacement, options)` | Replaces matches of `pattern` in `input` using string or replacer callback. |
| `papagaio.compile(patternOrTemplate, options)` | `patternOrTemplate.papagaio.compile(options)` | Pre-parses tokens into an optimized reusable runner. |
| `papagaio.registerModifier(name, handler)` | — | Registers custom token modifier globally. |

---

### Interpolation & JS Expressions

Papagaio distinguishes between simple variable substitution and full expression evaluation:

#### 1. Simple Variables (`$name`)
- Matches `[a-zA-Z_][a-zA-Z0-9_]*`.
- Directly looks up `context[name]`.
- Undefined variables emit an empty string `""` by default.

```javascript
import "papagaio";

const template = "User: $username (Role: $role)";
console.log(template.papagaio({ username: "alice", role: "admin" }));
// Output: "User: alice (Role: admin)"
```

#### 2. JavaScript Expressions (`${expression}`)
- Evaluates standard JavaScript expressions with context properties exposed directly in scope.
- Supports math, string operations, array methods, ternary operators, and method calls.

```javascript
const order = "Order #${id}: ${items.length} items, Total: $${items.reduce((a, b) => a + b.price, 0).toFixed(2)} (${paid ? 'PAID' : 'PENDING'})";

const result = order.papagaio({
  id: 4102,
  paid: true,
  items: [
    { name: "Mouse", price: 29.90 },
    { name: "Keyboard", price: 89.50 }
  ]
});

console.log(result);
// Output: "Order #4102: 2 items, Total: $119.40 (PAID)"
```

#### 3. Escaping
- `\$` emits a literal `$`.
- `\\` emits a literal `\`.
- `${unknown}` where `unknown` is not in `context` is preserved as-is.

---

### Pattern Matching (`.match()`)

```javascript
const captures = "pattern".papagaio.match(input, options);
// or: papagaio.match(pattern, input, options);
```

Executes pattern matching against `input`. If the input satisfies the pattern and its token constraints, returns an object mapping captured variable names to their extracted substrings. If no match is found, returns `null`.

```javascript
const route = "/users/$id$int/posts/$slug$identifier";

console.log(route.papagaio.match("/users/42/posts/announcement"));
// Output: { id: "42", slug: "announcement" }

console.log(route.papagaio.match("/users/abc/posts/announcement"));
// Output: null (since 'abc' fails the $int modifier constraint)
```

---

### Pattern Replacement (`.replace()`)

```javascript
const result = "pattern".papagaio.replace(input, replacement, options);
// or: papagaio.replace(pattern, input, replacement, options);
```

Searches `input` for occurrences of `pattern` and replaces them.

#### Replacement Options:
- **String Replacement**: Substituted using captured variables via `$name` or `${name}`.
- **Function Replacement**: Callback `(captures, matchInfo) => string`.

```javascript
const source = "calc(10, 20); calc(30, 40);";

// Function replacer with destructuring:
const evaluated = "calc($a$int, $b$int)".papagaio.replace(source, ({ a, b }) => {
  return `${Number(a) + Number(b)}`;
});
console.log(evaluated);
// Output: "30; 70;"

// String replacer:
const swapped = "$first $last".papagaio.replace("Ada Lovelace", "${last}, ${first}");
console.log(swapped);
// Output: "Lovelace, Ada"
```

#### Options:
- `options.all` *(boolean, default: `true`)*: Replaces all occurrences. Set `false` to replace only the first occurrence.

---

### Ahead-of-Time Compilation (`.compile()`)

When matching or interpolating in high-frequency loops, compile the pattern once:

```javascript
const pattern = "item: $sku$identifier qty: $count$int".papagaio.compile();

// Optimized match:
const match = pattern.match("item: CPU_5800X qty: 4");
// { sku: "CPU_5800X", count: "4" }

// Optimized template:
const text = pattern({ sku: "GPU_4090", count: 1 });
// "item: GPU_4090 qty: 1"

// Optimized replace:
const replaced = pattern.replace("item: RAM_16G qty: 2", "SKU=$sku COUNT=$count");
```

---

## Pattern Matching Syntax & Formal Semantics

A Papagaio pattern consists of whitespace-separated tokens. Tokens can be **literals**, **variables**, or **blocks**, augmented with modifiers.

### Token Kinds

| Token Syntax | Classification | Description |
|---|---|---|
| `word` | `TOK_LITERAL` | Matches exact text sequence. |
| `$name` | `TOK_VAR` | Captures variable up to the boundary of the next token. |
| `${name}` | `TOK_VAR` | Disambiguates variable name from following contiguous characters. |
| `$name$mod` | `TOK_VAR` (constrained) | Variable constrained and validated by modifier `mod`. |
| `$name$block{o}{c}` | `TOK_BLOCK` | Balanced recursive extraction between opening delimiter `o` and closing `c`. |
| `\s+` | `TOK_WS` | Matches one or more whitespace characters. |

---

### Flex-Matching Mechanics

In typical regular expressions, handling arbitrary spacing between tokens requires injecting `\s*` or `\s+` everywhere (e.g. `foo\s*=\s*bar`).

Papagaio features **flex-matching by default**:
1. Whitespace in the pattern matches **one or more** whitespace characters in the input.
2. Boundaries between literals and variables automatically permit horizontal whitespace skips (`" "` and `"\t"`).
3. Exact delimiters without whitespace (e.g. `$a,$b`) enforce contiguous match without spaces.

```javascript
// Matches: "a + b", "a   +   b", "a \t + \n b"
"result: $a + $b".papagaio.match("result: 10   +   20");
// { a: "10", b: "20" }
```

---

### Trailing Sigil (Whitespace Collapse)

Appending a trailing sigil (`$`) to a variable or literal collapses and consumes **all following whitespace characters** (including newlines).

```text
Pattern: "$a$ $b"
Input:   "hello       world"
Result:  Captures a="hello", b="world" (skipping arbitrary indentation/newlines)
```

---

### Optionality (`?`)

Any token (literal, variable, block, or group) can be flagged as optional by appending `?`:

```javascript
const greeting = "Hello $title? $name";

console.log(greeting.papagaio.match("Hello Dr. Smith"));
// { title: "Dr.", name: "Smith" }

console.log(greeting.papagaio.match("Hello Smith"));
// { title: "", name: "Smith" }
```

---

### Balanced Delimited Blocks (`$var$block{open}{close}`)

Regular expressions cannot count arbitrary nesting depth (such as nested parentheses or curly braces in code). Papagaio solves this with the `$block{open}{close}` token:

- Automatically maintains a stack counter (`depth`).
- Handles nested pairs: `{ a { b { c } } }`.
- Supports single-character (`( )`, `{ }`, `[ ]`, `< >`) and multi-character delimiters (`<< >>`, `BEGIN END`).

```javascript
const code = `
function compute(x) {
  if (x > 0) {
    return x * 2;
  }
  return 0;
}
`;

const match = "function $name($arg) $body$block{[}{]}".papagaio.match(code);
console.log(match.name); // "compute"
console.log(match.body); // "\n  if (x > 0) {\n    return x * 2;\n  }\n  return 0;\n"
```

---

## Modifiers Reference

### Built-in Modifiers

Modifiers specify lexical or type constraints. If an input candidate does not strictly satisfy the modifier, the token match is rejected.

| Modifier | Syntax | Validation Rule | Example Valid Input |
|---|---|---|---|
| **`int`** | `$x$int` | Integer digits with optional leading `-` (`/^-?\d+$/`) | `42`, `-103` |
| **`float`** | `$x$float` | Floating point number (`/^-?\d+(\.\d+)?$/`) | `3.14159`, `-0.5` |
| **`number`** | `$x$number` | Number, including scientific notation (`1e-4`) | `100`, `2.5e3` |
| **`identifier`** | `$x$identifier` | Standard language identifier (`/^[a-zA-Z_]\w*$/`) | `my_var`, `_init1` |
| **`word`** | `$x$word` | Alphabetic characters only (`/^[a-zA-Z]+$/`) | `HelloWorld` |
| **`hex`** | `$x$hex` | Hexadecimal digits, optional `0x`/`0X` prefix | `0x1a2b`, `ff00aa` |
| **`binary`** | `$x$binary` | Binary digits, optional `0b`/`0B` prefix | `0b1011`, `11001` |
| **`upper`** | `$x$upper` | Uppercase alphabetic letters only | `CONST`, `HTTP` |
| **`lower`** | `$x$lower` | Lowercase alphabetic letters only | `variable`, `id` |
| **`capitalized`** | `$x$capitalized` | Initial capital letter followed by lowercase | `Name`, `Title` |
| **`path`** | `$x$path` | Non-whitespace character sequence | `/usr/local/bin` |
| **`percent`** | `$x$percent` | Number followed by `%` | `75%`, `-12.5%` |
| **`alpha`** | `$x$alpha` | Validates alphabetic and transforms value to uppercase | `abc` $\to$ `ABC` |
| **`alphanum`** | `$x$alphanum` | Alphanumeric characters only | `item42` |

---

### Structural & Positional Modifiers

| Modifier | Syntax | Description |
|---|---|---|
| **`block`** | `$x$block{open}{close}` | Captures balanced content between `open` and `close`. |
| **`aliases`** | `$x$aliases{a}{b}{c}` | Multi-way branch matching one of the given alternatives. |
| **`group`** | `$x$group{subpattern}` | Treats `subpattern` as a single atomic unit. |
| **`starts`** | `$x$starts{prefix}` | Value must begin with `prefix`. |
| **`prefix`** | `$x$prefix{p}` | Value must begin with `p` (and have length greater than `p`). |
| **`ends`** | `$x$ends{suffix}` | Value must terminate with `suffix`. |
| **`suffix`** | `$x$suffix{s}` | Value must terminate with `s` (and have length greater than `s`). |
| **`infix`** | `$x$infix{needle}` | Value must contain `needle` strictly between its start and end. |
| **`includes`** | `$x$includes{needle}` | Value must contain substring `needle` anywhere. |

---

### Custom Modifiers API

Extend Papagaio's token engine with custom validators using `papagaio.registerModifier`:

```javascript
import { papagaio } from "papagaio";

// Register validator
papagaio.registerModifier("semver", (val) => {
  return /^\d+\.\d+\.\d+(-[0-9A-Za-z.-]+)?$/.test(val);
});

const match = "release v$ver$semver".papagaio.match("release v1.4.2-alpha.1");
console.log(match);
// { ver: "1.4.2-alpha.1" }
```

**Handler Contract:**
- Return `true` or the captured string to **accept**.
- Return a modified string to **transform** the captured value.
- Return `false` or `null` to **reject** the match.

---

## Configuration & Custom Delimiters

Delimiters and sigils are customizable via options:

```javascript
import { papagaio } from "papagaio";

const template = "Hello @name, count is @{1 + 2}";

const output = papagaio(template, { name: "Developer" }, {
  sigil: "@",   // Default: "$"
  open: "{",    // Default: "{"
  close: "}"    // Default: "}"
});

console.log(output);
// Output: "Hello Developer, count is 3"
```

---

## Advanced Use Case: Custom Syntax to WebAssembly (WAT)

Papagaio is the ideal engine for **language designers and DSL authors** who want to invent their own syntax and compile it directly into clean, optimized, runtime-agnostic **WebAssembly (WAT)**.

Because `.replace()` accepts JavaScript callback functions, AST generation, constant folding, and instruction emission happen with zero compiler framework boilerplate.

### Included Production Examples:
- [`examples/forth2wat/`](./examples/forth2wat): Full ANSI/FIG-style Forth compiler (`DUP`, `SWAP`, `DO...LOOP`, `IF...ELSE`, stack memory).
- [`examples/lisp2wat/`](./examples/lisp2wat): High-level Lisp S-expression compiler (`fib`, `fact`, linear memory, function exports).

### Live Compilation Sample:

```javascript
import { papagaio } from "papagaio";

// 1. A custom syntax invented by the developer
const userCode = `
  task double(x) {
    return x * 2;
  }
`;

// 2. Desugaring directly to WAT using Papagaio patterns
const wat = "task $fn($arg) $body$block{[}{]}".papagaio.replace(userCode, ({ fn, arg, body }) => {
  // Optimize: x * 2 converted to left bitshift (i32.shl)
  const optimizedBody = "return $x * 2".papagaio.replace(body, ({ x }) => {
    return `(i32.shl (local.get $${x}) (i32.const 1))`;
  });

  return `
(module
  (func $${fn} (param $${arg} i32) (result i32)
    ${optimizedBody}
  )
  (export "${fn}" (func $${fn}))
)`;
});

console.log(wat);
```

#### Generated WAT:
```wat
(module
  (func $double (param $x i32) (result i32)
    (i32.shl (local.get $x) (i32.const 1))
  )
  (export "double" (func $double))
)
```

---

## Command Line Interface (CLI)

Papagaio includes a command line runner for file processing and template generation:

```sh
# Basic syntax
node src/cli.js <template-file> [key=value ...]

# Example:
node src/cli.js config.template.json env=production port=8080
```

---

## Legacy Compatibility Example

For users migrating code written for the historical C version of Papagaio (`$pattern`, `$from`, `$list`, `$repeat`, `$while`, `$math`, etc.), an interpreter written entirely on top of the new JavaScript Papagaio is available in `examples/legacy`:

```sh
node examples/legacy/run_demo.js
```

This demo reproduces legacy macros, list mutations, and math expressions by using `String.prototype.papagaio` as its transformation substrate.

---

## Test Suite & Verification

Papagaio maintains a rigorous ES2023 regression test suite:

```sh
# Run official test suite
npm test

# Run WebAssembly compilers integration tests
node examples/lisp2wat/run_demo.js
node examples/forth2wat/run_demo.js
```

---

## System Limits & Performance Characteristics

| Metric | Property | Behavior |
|---|---|---|
| **Runtime Dependencies** | None (`0`) | Runs natively on Node, Bun, Deno, QuickJS, Browser. |
| **Language Target** | ES2023 | Compatible with all current JavaScript toolchains. |
| **Memory Allocation** | Linear string views | Pattern scanner avoids unnecessary string copies. |
| **Balanced Depth** | Arbitrary | Stack-based depth counter, not bounded by regular expression recursion limits. |

---

## License

MIT © [jardimdanificado](https://github.com/jardimdanificado)
