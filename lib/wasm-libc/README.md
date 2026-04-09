# wasm-libc

Bare-metal implementation of a libc for WebAssembly, compiled with `clang`
and interpreted with [wasm3](https://github.com/wasm3/wasm3).

## Philosophy

- **Zero dependencies** — no system libc, no WASI
- **Explicit ABI** — I/O and flow control are declared host imports
- **Auditable** — each function has at most 50–100 lines, no complex macros
- **wasm3-safe** — WebAssembly MVP only; no threads, SIMD, or bulk-memory

## Structure

```
wasm-libc/
├── include/          public headers
│   ├── stddef.h      size_t, NULL, offsetof
│   ├── stdint.h      int32_t, uint64_t, ...
│   ├── stdbool.h     bool, true, false
│   ├── limits.h      INT_MAX, CHAR_BIT, ...
│   ├── string.h      mem*, str*
│   ├── stdio.h       printf, snprintf, puts
│   └── stdlib.h      malloc, free, atoi, qsort, rand
│
├── src/
│   ├── string/       memcpy, memmove, memset, strcmp, strtok_r, ...
│   ├── memory/       malloc, free, calloc, realloc (free-list + boundary tags)
│   ├── stdio/        printf, sprintf, snprintf, vsnprintf (all specs)
│   └── stdlib/       atoi, strtol, itoa, qsort, bsearch, rand, abs
│
├── test/
│   └── test_main.c   unit test suite
│
└── build.sh          build script
```

## Requirements

```bash
# Ubuntu / Debian
apt install clang llvm lld

# Arch Linux
pacman -S clang llvm lld

# macOS (Homebrew)
brew install llvm
```

To run tests:
```bash
# install wasm3
git clone https://github.com/wasm3/wasm3 && cd wasm3
cmake . && make -j4
sudo cp build/wasm3 /usr/local/bin/
```

## Build

```bash
chmod +x build.sh

./build.sh        # compiles libc + test.wasm
./build.sh lib    # only static libc (.a)
./build.sh test   # compiles and links test.wasm
./build.sh clean  # removes artifacts
```

## Running tests

```bash
wasm3 --func main build/test.wasm
```

Expected output:
```
wasm-libc test suite
====================

[string]
  OK   basic strlen
  OK   empty strlen
  ...

[malloc]
  OK   malloc returns non-NULL
  ...

result: 42/42 tests passed
```

## Host imports ABI

The `"env"` module must export to WASM:

| Function           | C Signature                           | Description        |
|------------------|---------------------------------------|--------------------|
| `__host_write`   | `(const char *buf, int len) -> void`  | stdout             |
| `__host_write_err`| `(const char *buf, int len) -> void` | stderr             |
| `__host_abort`   | `() -> void`                          | abort/panic        |

### C binding example (wasm3 host)

```c
#include "wasm3.h"
#include "m3_env.h"

m3ApiRawFunction(host_write) {
    m3ApiGetArgMem(const char *, buf, 0);
    m3ApiGetArg(int32_t, len, 1);
    fwrite(buf, 1, (size_t)len, stdout);
    m3ApiSuccess();
}

m3ApiRawFunction(host_abort) {
    abort();
}

// during initialization:
m3_LinkRawFunction(module, "env", "__host_write",     "v(*i)", host_write);
m3_LinkRawFunction(module, "env", "__host_write_err", "v(*i)", host_write);
m3_LinkRawFunction(module, "env", "__host_abort",     "v()",   host_abort);
```

## Memory Allocator

Implements **free-list with boundary tags** (Knuth, 1973):

```
[ header 8B | payload ... | footer 4B ]
   size|flags              size|flags
```

- **Coalescing** in O(1) forwards and backwards using boundary tags
- **Splitting** when the block has enough space for a minimum block
- **heap_grow** uses WASM `memory.grow` — no syscall, no OS
- **Alignment** of 8 bytes on all blocks

## Planned modules

- [ ] `math.h` — sin, cos, sqrt, pow (pure C implementations)
- [ ] `ctype.h` — isalpha, isdigit, toupper, tolower
- [ ] `errno.h` — error codes
- [ ] `setjmp.h` — bare-metal longjmp for WASM
- [ ] `time.h` — with host import for clock
