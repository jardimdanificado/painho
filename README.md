# Papagaio Preprocessor

Papagaio is a lightweight, class‑based text preprocessor designed to support pattern rewriting, macro expansion, scoped transforms, and embedded JavaScript evaluation. It is engineered to be predictable, recursion‑safe, and easily embeddable in any JavaScript runtime.

---

## Overview

Papagaio processes an input string through a deterministic multi‑stage pipeline:

1. **Scope blocks** (recursive processing)
2. **Eval blocks** (JS execution)
3. **Macro collection**
4. **Pattern collection**
5. **Pattern application**
6. **Macro expansion**

The engine runs until it reaches a fixed point or hits the recursion limit.

Papagaio supports **nested delimiters**, **custom sigils**, and **configurable keywords**.

---

## Installation

Papagaio ships as a standalone ES module.

```js
import { Papagaio } from "./papagaio.js";
```

---

## Basic Usage

```js
const p = new Papagaio();
const output = p.process("pattern {a} {b}  a a a");
console.log(output); // "b b b"
```

---

## Core Features

### 1. Pattern Blocks

Patterns rewrite text using a match → replacement structure:

```
pattern { MATCH } { REPLACEMENT }
```

Patterns are collected once per iteration and applied globally.

Example:

```
pattern {hello} {hi}
hello world
```

Output:

```
hi world
```

#### Variables

Variables use the configured **sigil** (default `$`).

```
pattern {say $x} {[$x]}
say hello
```

→ `[hello]`

#### Balanced Block Variables

Papagaio supports deep matching of balanced blocks:

```
pattern {($x)} {[BLOCK:$x]}
(do something)
```

→ `[BLOCK:do something]`

#### Spread Variables

Spread variables capture until a terminating token:

```
pattern {from $x...to} {$x}
from A B C to
```

→ `A B C`

#### Metavariables

Papagaio provides special `$keywords` inside replacements:

| Variable  | Meaning                                    |
| --------- | ------------------------------------------ |
| `$match`  | Full matched text                          |
| `$pre`    | Text before match                          |
| `$post`   | Text after match                           |
| `$unique` | Auto‑increment unique token                |
| `$$`      | Whitespace wildcard in patterns            |
| `$clear`  | Triggers clear‑rewrite of last replacement |

Example:

```
pattern {x} {$pre[${unique}]$post}
abcxdef
```

---

### 2. Macro Blocks

Macros behave like simple template functions.

```
macro name { BODY }
name(arg1, arg2)
```

Example:

```
macro wrap {[$1][$2]}
wrap(a, b)
```

→ `[a][b]`

#### Argument Mapping

Arguments map to `$1`, `$2`, … automatically. `$0` expands to the macro name.

Example:

```
macro tag {<$0 $1>}  
tag(title)
```

→ `<tag title>`

---

### 3. Eval Blocks

Executes embedded JavaScript and returns the result as a string.

```
eval { return 3 + 7 }
```

→ `10`

Eval executes inside a strict IIFE.

You may access:

* `papagaio` → the processor instance
* `ctx` → an empty object for temporary state

Example:

```
pattern {sum $a $b} {eval { return Number($a) + Number($b) }}
sum 2 8
```

→ `10`

---

### 4. Scope Blocks

Scope blocks create isolated processing regions using the same pipeline.

```
scope {
    pattern {x} {y}
    x x x
}
```

→ `y y y`

Scopes do not leak macros or patterns to the outside.

---

## Delimiters

Papagaio supports configurable opening/closing pairs.

```js
p.delimiters = [["{", "}"], ["(", ")"]];
```

Balanced variable matching works across all registered delimiter pairs.

---

## Recursion Model

Papagaio runs a fixed‑point loop:

1. Process scopes
2. Process eval blocks
3. Collect macros
4. Collect top‑level patterns
5. Apply patterns
6. Expand macros

If any Papagaio keyword remains in the output, it repeats.

You can configure the iteration cap:

```js
p.maxRecursion = 256;
```

---

## Error Handling

Eval blocks are wrapped in a try/catch. Exceptions produce empty output.

Pattern/macro recursion halts upon reaching `maxRecursion`.

---

## Advanced Examples

### Nested Macros

```
macro A {($1)}
macro B {A($1)}
B(hello)
```

→ `(hello)`

### Dynamic Rewriting with `$unique`

```
pattern {node} {id_$unique}
node node node
```

→ `id_0 id_1 id_2`

### Working with Pre/Post Context

```
pattern {x} {$pre|X|$post}
a x b
```

→ `a |X| b`

### Spread Matching

```
pattern {let $name = $v...;} {[decl $name $v]}
let x = 1 + 2 + 3;
```

→ `[decl x 1 + 2 + 3]`

---

## API Reference

### Class: `Papagaio`

#### `process(input: string): string`

Runs the full pipeline and returns the transformed text.

#### `delimiters: Array<[string, string]>`

List of opening/closing delimiter pairs.

#### `sigil: string`

Variable prefix. Default `$`.

#### `keywords`

Keyword configuration:

* `pattern`
* `macro`
* `eval`
* `scope`

#### Internal State

For debugging only:

* `content` → latest processed text
* `#matchContent`
* `#scopeContent`
* `#evalContent`

---

## Embedding

Papagaio is pure JS and safe to embed in build pipelines, CLIs, game engines, or runtime scripting layers.

Example minimal CLI wrapper:

```js
#!/usr/bin/env node
const fs = require("fs");
const { Papagaio } = require("./papagaio");

const input = fs.readFileSync(0, "utf8");
const out = new Papagaio().process(input);
process.stdout.write(out);
```

---
