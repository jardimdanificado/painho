# Papagaio Examples & Tutorials 📚

This directory contains complete, verified, and self-contained tutorials and production examples built with **Papagaio**.

Each example demonstrates how to leverage Papagaio's pattern matching, balanced block captures, and template interpolation to solve real-world language transformation challenges without heavy compiler frameworks.

---

## 📂 Available Tutorials

### 1. [Forth to WebAssembly (WAT) Compiler](./forth2wat/)
- **Concepts**: Stack machine semantics, memory pointer manipulation, balanced branch compilation (`IF ... ELSE ... THEN`, `DO ... LOOP`).
- **Demonstrates**: Translating pure ANSI/FIG Forth words directly to WAT S-expressions, assembling with `wat2wasm`, and executing native WASM in Node.js.
- **Run**:
  ```sh
  node examples/forth2wat/run_demo.js
  ```

---

### 2. [Lisp / S-Expression to WebAssembly (WAT) Compiler](./lisp2wat/)
- **Concepts**: Recursive S-expression pattern matching, identifier vs literal modifier validation (`$v$int`, `$id$identifier`), linear memory store/load, loop label branching.
- **Demonstrates**: Compiling high-level recursive functions (`fibonacci`), iterative loops (`factorial`), and raw linear memory manipulation into standard WebAssembly binary format.
- **Run**:
  ```sh
  node examples/lisp2wat/run_demo.js
  ```

---

### 3. [Legacy Papagaio Compatibility Interpreter](./legacy/)
- **Concepts**: Meta-programming, implementing an entire language runtime with scoped variables (`$from`), dynamic list operations (`$list`), loops (`$while`, `$repeat`), and math (`$math`) purely using `String.prototype.papagaio`.
- **Demonstrates**: How modern Papagaio can serve as an engine to execute older DSLs and historical code without C compilers or native DLL plugins.
- **Run**:
  ```sh
  node examples/legacy/run_demo.js
  ```

---

## 🧪 Testing All Examples

All examples are fully automated and verified against the active codebase:

```sh
node examples/forth2wat/run_demo.js
node examples/lisp2wat/run_demo.js
node examples/legacy/run_demo.js
```
