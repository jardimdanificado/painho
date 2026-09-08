# Lisp to WebAssembly (WAT) Compiler Tutorial

This tutorial guides you through building an **optimizing S-expression / Lisp-like language compiler** that generates native WebAssembly Text (WAT) using **Papagaio** as the core desugaring engine.

---

## 🎯 What You Will Learn

1. How to use Papagaio's **recursive pattern matching** to parse nested S-expressions without writing a hand-rolled lexer or parser.
2. How to use **semantic modifiers** (`$n$identifier`, `$v$int`) to separate variable names from integer literals.
3. How to implement **compiler passes** (function desugaring, local variable hoisting, expression flattening) using `.papagaio.replace()`.
4. How to compile and execute recursive functions (e.g. `fibonacci`), loops with branches (`factorial`), and raw linear memory operations (`store` / `load`).

---

## 💡 Architecture of the Compiler

The compiler transforms high-level Lisp into standard WASM 1.0 folded S-expressions:

```text
Lisp Source Code (S-Expressions)
              │
              ▼
   [ Papagaio Top-Level Scanner ] ── Splits (func ...), (memory ...), (export ...)
              │
              ▼
   [ Papagaio Recursive Desugaring ]
     - Translates (if cond then else)
     - Translates (loop label ...) and (br_if label cond)
     - Maps (+ a b), (* x y) to (i32.add a b), (i32.mul x y)
     - Hoists (local $var i32) declarations to function headers
              │
              ▼
   [ Papagaio Module Assembler ] ── Emits full (module ...) text
              │
              ▼
   WebAssembly Binary Execution (wat2wasm -> WebAssembly.instantiate)
```

---

## 🔍 How Papagaio Transforms Lisp to WAT

### 1. Matching Function Signatures
A function definition in our Lisp dialect looks like:
```lisp
(func $fib (param $n i32) (result i32)
  ...body...
)
```

Using Papagaio's pattern matching with balanced parenthesized blocks:

```javascript
import { papagaio } from "../../src/index.js";

// Extracts function signature, params, return type and body:
const funcPattern = "func $name$identifier (param $param$identifier $ptype) (result $rtype) $body$block{(}{)}";

const match = funcPattern.papagaio.match(rawFuncContent);
console.log(match.name);  // "$fib"
console.log(match.param); // "$n"
console.log(match.body);  // "(if (<= $n 1) $n (+ ($fib (- $n 1)) ($fib (- $n 2))))"
```

### 2. Translating Expressions & Operators

Papagaio replaces binary arithmetic operators into WASM stack instructions:

```javascript
// Operands can be nested sub-expressions or literal tokens
const exprPattern = "$op $arg1 $arg2";

// Translates (+ a b) -> (local.get $a) (local.get $b) i32.add
```

### 3. Constant Folding & Literal Boxing

Using Papagaio modifiers like `$v$int`, numeric literals are identified and wrapped in `(i32.const $v)` automatically:

```javascript
// Matches literal integer:
"($op $x $num$int)".papagaio.replace(source, ({ op, x, num }) => {
  return `(${WASM_OPS[op]} (local.get $${x}) (i32.const ${num}))`;
});
```

---

## 🚀 Running the Demo

Execute the demo script to compile the Lisp source, assemble it with `wat2wasm`, and execute all functions:

```sh
node examples/lisp2wat/run_demo.js
```

### Example Lisp Code:
```lisp
;; Linear memory of 1 page (64KB)
(memory $mem 1)

;; Recursive Fibonacci
(func $fib (param $n i32) (result i32)
  (if (<= $n 1)
    $n
    (+ ($fib (- $n 1)) ($fib (- $n 2)))
  )
)

;; Iterative Factorial with Loop and Locals
(func $fact (param $n i32) (result i32)
  (local $res i32)
  (set $res 1)
  (loop $l
    (if (> $n 1)
      (then
        (set $res (* $res $n))
        (set $n (- $n 1))
        (br_if $l 1)
      )
    )
  )
  $res
)

;; Direct Linear Memory Store and Load
(func $mem_test (param $addr i32) (param $val i32) (result i32)
  (i32.store $addr $val)
  (i32.load $addr)
)
```

### Verified Runtime Output:
```text
Results:
• fib(10)          = 55   (expected: 55)   -> CORRECT ✅
• fact(5)          = 120  (expected: 120)  -> CORRECT ✅
• fact(7)          = 5040 (expected: 5040) -> CORRECT ✅
• mem_test(64, 42) = 42   (expected: 42)   -> CORRECT ✅
```
