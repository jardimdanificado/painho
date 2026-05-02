# Papagaio-C Language Specification

Papagaio-C is a modern, WebAssembly-native dialect of the C programming language. It is designed to be lean, fast, and directly integrated with the WASM ecosystem, providing high-level keywords for features that traditionally require non-standard compiler attributes.

## 1. Native Type System

Papagaio-C replaces or supplements standard C types with explicit-width integer and floating-point types, matching the WebAssembly specification.

### Integer Types
| Type | Bits | Sign | WASM Mapping |
| :--- | :--- | :--- | :--- |
| `i8` | 8 | Signed | `i32` (ext) |
| `u8` | 8 | Unsigned | `i32` (ext) |
| `i16` | 16 | Signed | `i32` (ext) |
| `u16` | 16 | Unsigned | `i32` (ext) |
| `i32` | 32 | Signed | `i32` |
| `u32` | 32 | Unsigned | `i32` |
| `i64` | 64 | Signed | `i64` |
| `u64` | 64 | Unsigned | `i64` |

### Floating-Point Types
| Type | Bits | WASM Mapping |
| :--- | :--- | :--- |
| `f32` | 32 | `f32` |
| `f64` | 64 | `f64` |

---

## 2. Modern Keywords and Modifiers

Unlike traditional C compilers that use `__attribute__((...))`, Papagaio-C promotes critical metadata to first-class keywords. Modifiers can be placed before or after the type (e.g., `export void f()` or `void export f()`).

### Storage & Visibility
- `export`: Marks a function or variable to be exported in the WASM `Export` section.
- `entrypoint`: Marks a function as the specific entry point of the binary (replaces `main` as the default).
- `weak`: Marks a symbol as weak for the linker.
- `static`: Standard C visibility (local to the translation unit).

### Function Attributes
- `noreturn`: Indicates the function does not return to the caller.
- `noinline`: Prevents the compiler from inlining the function.
- `constructor`: Marks a function to be executed during WASM initialization.
- `destructor`: Marks a function to be executed during cleanup.

### Data Layout
- `packed`: Used with `struct` to ensure no padding is added between members.
- `aligned(N)`: Ensures a variable or struct is aligned to an $N$-byte boundary.
- `at(address)`: Places a global variable at a specific memory address.

---

## 3. WebAssembly Specifics

### Stack Size Configuration
Instead of a compiler flag, the stack size is defined directly in the source code using a special global constant:
```c
const i32 __stack_size = 4096; // Sets stack to 4KB
```
If not defined, the compiler uses a default value (usually 64KB).

### Memory Mapping
You can map variables directly to hardware or specific memory offsets:
```c
at(0x1000) u32 framebuffer[64 * 64];
```

### Importing Functions
Use `import_module` and `import_name` modifiers to declare external requirements.
```c
import_module("env") import_name("print") 
void log_message(const i8 *msg);
```

---

## 4. Dialect Restrictions

Papagaio-C simplifies the C language by removing legacy and complex features that hinder WebAssembly efficiency.

### No Preprocessor
Papagaio-C performs **direct source compilation**. There is no preprocessor stage:
- No `#include` (use modular compilation and linking).
- No `#define` or macro expansion.
- No conditional compilation (`#ifdef`, etc.).

### Removed Control Flow
- **No `goto`**: Arbitrary jumps are disallowed to maintain structured WASM output.
- **No `setjmp`/`longjmp`**: Non-local jumps are not supported.

### Binary Type Auto-Detection
The compiler (`wcc`) determines if the output is an **Executable** or a **Library**:
- **Executable**: If a function is marked as `entrypoint` or if a function named `main` is present and exported.
- **Library**: If no explicit entry point is found.

---

## 5. Built-in Symbols

- `__heap_base`: Constant address where dynamic memory begins (after data and stack).
- `__stack_pointer`: Global variable managing the current stack top.

---

## 6. Examples

### Complete Application Example
```c
const i32 __stack_size = 1024;

import_module("sys") import_name("putc") void putc(i32 c);

at(0x0) i8 buffer[128];

entrypoint void start() {
    buffer[0] = 'H';
    putc(buffer[0]);
}
```
