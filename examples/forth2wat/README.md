# Forth to WebAssembly (WAT) Compiler Tutorial

This tutorial demonstrates how to build a **complete Forth compiler targeting WebAssembly Text (WAT)** using **Papagaio** as the primary code transformation and templating engine.

---

## 🎯 What You Will Learn

1. How to use Papagaio's `String.prototype.papagaio.replace()` to match and desugar high-level constructs.
2. How to use Papagaio's balanced block capture (`$body$block{[}{]}`) to compile structured control flow (`IF ... ELSE ... THEN` and `DO ... LOOP`).
3. How to use Papagaio templates to construct valid, clean WebAssembly S-expressions without a complex AST generator.
4. How to instantiate the resulting WASM binary directly in the Node.js / browser runtime.

---

## 💡 Why Forth & WebAssembly?

Forth is a minimalist, stack-based programming language. WebAssembly (Wasm) is also fundamentally a stack machine (plus linear memory and typed locals). 

However, Forth words can leave an open number of values on the data stack across branch boundaries. To achieve 100% compliant Forth semantics without compiler stack analysis, we map Forth's data stack to **WASM linear memory**:

- Pointer `$sp` (Stack Pointer in global mutable memory).
- **Pushing** a value:
  ```wat
  (global.get $sp) (i32.const 4) i32.add (global.set $sp)
  (global.get $sp) (i32.const <value>) i32.store
  ```
- **Popping** a value:
  ```wat
  (global.get $sp) i32.load
  (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  ```

---

## 🔍 How Papagaio Does the Heavy Lifting

### 1. Extracting Word Definitions (`: WORD ... ;`)

In Forth, new functions are called "words", declared as `: NAME ... ;`.
Using Papagaio, we extract the name and the entire body using balanced delimiters:

```javascript
import { papagaio } from "../../src/index.js";

const wordPattern = ": $name$identifier $body ;";

// Extract words from the source:
forthCode.papagaio.replace(wordPattern, ({ name, body }) => {
  // Compile word body and emit WAT function
  return emitWatFunction(name, body);
});
```

### 2. Compiling Control Flow (`IF ... ELSE ... THEN`)

When compiling `IF`, Papagaio templates inject the generated then/else instruction blocks cleanly:

```javascript
const watIf = `
  (global.get $sp) i32.load
  (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  (if
    (then
      $thenBranch
    )
    (else
      $elseBranch
    )
  )
`.papagaio({
  thenBranch: thenInstructions.join("\n"),
  elseBranch: elseInstructions.join("\n")
});
```

### 3. Stack Combinators (`DUP`, `SWAP`, `OVER`, `ROT`)

Stack manipulation words are desugared directly to simple, highly optimizable memory copies using Papagaio patterns:

```javascript
// DUP ( a -- a a )
const DUP_WAT = `
  (global.get $sp) (i32.const 4) i32.add
  (global.get $sp) i32.load
  i32.store
  (global.get $sp) (i32.const 4) i32.add (global.set $sp)
`;
```

---

## 🚀 Running the Demo

Execute the demo script to compile Forth, assemble it with `wat2wasm`, and run it:

```sh
node examples/forth2wat/run_demo.js
```

### Example Forth Code:
```forth
\ Square function (x -> x^2)
: SQUARE
  DUP * ;

\ Pythagorean theorem: a^2 + b^2
: PYTHAGORAS
  SQUARE SWAP SQUARE + ;

\ Iterative factorial using DO ... LOOP
: FACTORIAL
  DUP 1 <= IF
    DROP 1
  ELSE
    1 SWAP 1 + 2 DO
      I *
    LOOP
  THEN ;
```

### Verified Output:
```text
• 7 SQUARE        = 49 (expected: 49) -> CORRECT ✅
• 3 4 PYTHAGORAS  = 25 (expected: 25) -> CORRECT ✅
• 5 FACTORIAL     = 120 (expected: 120) -> CORRECT ✅
• 6 FACTORIAL     = 720 (expected: 720) -> CORRECT ✅
• 42 99 MAX_VAL   = 99 (expected: 99) -> CORRECT ✅
```
