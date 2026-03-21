Papagaio
========

Papagaio is a C-first, embeddable text processing and pattern-matching preprocessor that exposes a small, well-documented C API, a native Lua module, a command-line executable, and a JavaScript/WASM wrapper for browser/node usage. It embeds a Lua VM (supports Lua 5.1–5.4 and LuaJIT) to run dynamic $eval{} blocks and provides flexible pattern/replacement APIs intended for rapid, scriptable source preprocessing and text generation.

Key Features
------------

- C library (libpapagaio): static library intended for embedding in C projects. Includes Unicode-aware regexp support via an internal libregexp component.
- Embedded Lua VM: Papagaio embeds a Lua VM so replacements or evaluation blocks can run arbitrary Lua code securely in-process.
- Native Lua module: Builds a papagaio.so so Lua projects can require("papagaio") and use the library directly from Lua.
- Command-line tool: A small CLI executable (built from src/main.c) that reads a file, processes it and writes the output to stdout.
- JavaScript/WASM wrapper: papagaio.js exposes a JS-friendly Papagaio class that uses an Emscripten-produced WASM module (papagaio_wasm.js) for browser/node usage.
- Flexible APIs: Several C APIs for various use cases — convenience varargs API, explicit context API, pair-based processing, and raw buffer processing.
- Safe ownership / memory model: Processing functions return malloc'd C strings. Callers are responsible for freeing the returned memory.

What Papagaio Does
------------------

Papagaio is a preprocessor engine: it scans input text and performs pattern-based substitutions and scripted evaluations. It is suitable to implement macros, pattern-driven code generation, template-like transformations, and any text-processing pipeline where embedding a tiny scripting VM (Lua) is desirable.

Public Interfaces
-----------------

C API (header: src/papagaio.h)
- Papagaio *papagaio_open(void);
  Create a new context. This also creates/attaches a fresh Lua state for $eval{} execution.

- void papagaio_close(Papagaio *ctx);
  Destroy a context and free all associated resources.

- void papagaio_cleanup(void);
  Cleanup lazy/global VM state (the library registers this to run at exit too).

- struct lua_State *papagaio_L(Papagaio *ctx);
  Borrow the inner lua_State* if the embedding application needs direct access to the Lua VM.

- void papagaio_set_args(Papagaio *ctx, int argc, char **argv);
  Pass program arguments into the internal Lua state (useful when running scripts relying on arg/argv).

- char *papagaio_process(const char *input, ...);
  Convenience varargs API accepting NULL-terminated pairs of pattern/replacement strings.

- char *papagaio_process_ex(const char *input,
                            const char *sigil,
                            const char *open,
                            const char *close, ...);
  Extended varargs API that lets the caller customize the sigil and opening/closing delimiters (for example, customizing $eval{} markers).

- char *papagaio_process_pairs(Papagaio *ctx,
                               const char *input,
                               const char **patterns,
                               const char **repls,
                               int pair_count);
  Explicit pairs API to pass arrays of patterns and replacements along with an explicit context.

- char *papagaio_process_text(Papagaio *ctx,
                              const char *input,
                              size_t len);
  Low-level API that processes a raw text buffer of known length and returns a newly allocated C string with the processed output.

Notes about return values: All processing functions return malloc()'d C strings. The caller MUST free() the returned pointer when finished.

Lua module entry points
- LUALIB_API int luaopen_papagaio(struct lua_State *L);
  Called by Lua when require("papagaio") is used. Exposes Papagaio functionality to Lua scripts.

- LUALIB_API int luaopen_memory(struct lua_State *L);
  Auxiliary module (exposed as memory) shipped alongside papagaio for convenience in Lua environments.

Command-line usage
------------------

After building and installing the project, the "papagaio" executable can be installed to a system bin directory. A simple usage example (the C example in src/main.c):

  papagaio <input-file>

The CLI reads the file, processes it with an explicit Papagaio context and writes the result to stdout. The C main prints localized error messages when files cannot be read or processing fails.

JavaScript / WASM usage
----------------------

The repository includes a papagaio.js wrapper that loads a WASM module and exposes a small Papagaio class. Example:

  import Papagaio from './papagaio.js';

  const p = new Papagaio();
  await p.init();
  const out = p.process('text with $eval{ return 1 + 2 }');
  console.log(out);

Important notes:
- Call await papagaio.init() before using process() because the wrapper must initialize the WASM module first.
- The JS wrapper calls the C API papagaio_process_text under the hood and frees the returned pointer after converting it to a JS string.

Building from source
--------------------

Requirements:
- A C compiler (gcc/clang) and CMake (>= 3.10).
- A Lua dev environment if you plan to build the native Lua module (optional; the library embeds its own VM bindings when building).

Typical steps:

  mkdir build && cd build
  cmake ..
  make
  sudo make install

This will build the static library libpapagaio and the papagaio executable and install the header (papagaio.h) in the system include path if using the provided CMake install rules.

Node / npm

The package.json shows a minimal Node integration (papagaio.js as the JS wrapper and a bin/cli.mjs for the npm binary). Run the included tests with:

  npm test

(That runs node tests/test.js as defined in package.json.)

Examples
--------

C minimal example (from src/main.c):

  Papagaio *ctx = papagaio_open();
  papagaio_set_args(ctx, argc, argv);
  char *out = papagaio_process_text(ctx, input_buf, input_len);
  if (out) { puts(out); free(out); }
  papagaio_close(ctx);

JS example (see papagaio.js):

  const pap = new Papagaio();
  await pap.init();
  const result = pap.process('hello');

Design notes and behavior
-------------------------

- Papagaio intentionally favors a small, C-first API surface so it can be embedded easily in native projects.
- The embedded Lua VM is chosen to provide expressive, runtime-evaluated replacements. The library is compatible with Lua 5.1–5.4 and LuaJIT when linked appropriately.
- Regex and Unicode handling are implemented through an internal libregexp subcomponent (see CMakeLists and lib/libregexp/).
- By default, if you pass NULL as the Papagaio context to the convenience APIs, a lazy internal VM/context is used: this is convenient for quick one-shot usage but less suitable for multithreaded workloads.

Contributing
------------

- Report bugs at: https://github.com/jardimdanificado/papagaio/issues
- The repository URL is declared in package.json for the upstream project.
- To contribute: fork, create a topic branch, and make small, focused changes. Open a pull request describing the motivation and the testing performed.

License
-------

No license file is present in this checkout. Check the upstream repository for license information before using Papagaio in projects that require specific licensing terms.

Authors and contact
-------------------

Author: jardimdanificado (see package.json). For bug reports, use the GitHub issues link above.

Acknowledgements & internals
----------------------------

- The CMake build links the math library when not on MSVC and installs header, library and executable using standard CMake install rules.
- The library bundles a small regular expression and unicode helper code under lib/libregexp/ to provide cross-platform pattern matching.

Usage, Syntax, and Semantics
============================

Quick Start
-----------

**CLI:**
```sh
papagaio input.txt > output.txt
```

**C API:**
```c
Papagaio *ctx = papagaio_open();
char *out = papagaio_process(ctx, "pattern", "replacement", NULL);
printf("%s", out);
free(out);
papagaio_close(ctx);
```

**JavaScript (WASM):**
```js
import Papagaio from './papagaio.js';
const p = new Papagaio();
await p.init();
console.log(p.process('your text here'));
```

Pattern Syntax
--------------
- Patterns are whitespace-separated tokens. Each token can be:
  - A literal (matches exactly)
  - A variable: `$name` (captures one token)
  - Optional: `$name?` (capture is optional)
  - With type: `$age$int`, `$word$upper` (see modifiers below)
  - Regex: `$regex$id {\d+}` (captures using regex)
  - Block: `$block{[}{]}item` (captures inside delimiters)
  - Aliases: `$kind$aliases{cat,dog}` (matches one of the listed words)

Change Delimiters
-----------------
- `$changequote{sigil}{open}{close}`: Changes the sigil and delimiters for the rest of the text.
  Example: `$changequote{@}{<}{>} @eval<return 1+1>` outputs `2`.

Replacement Syntax
------------------
- Use `$name` to insert a capture.
- Use `$eval{ ... }` to run Lua code and insert the result. The variable `match` is set to the matched text.

Examples
--------
**Simple:**
```txt
Pattern:      $name $age$int
Replacement:  Name: $name, Age: $age
Input:        Alice 42
Output:       Name: Alice, Age: 42
```

**Regex + Eval:**
```txt
Pattern:      $regex$n {\d+}
Replacement:  Number: $n, doubled: $eval{ return tonumber(match) * 2 }
Input:        123
Output:       Number: 123, doubled: 246
```

**Block Sequence:**
```txt
Pattern:      $blockseq{[}{]}items
Replacement:  Items: $items
Input:        [a][b][c]
Output:       Items: a b c
```

**Aliases and Optional:**
```txt
Pattern:      $kind$aliases{cat,dog} $name?
Replacement:  Kind: $kind, Name: $name
Input:        dog Rover
Output:       Kind: dog, Name: Rover
Input:        cat
Output:       Kind: cat, Name: 
```

Modifiers
---------
- `$var$int`, `$var$float`, `$var$number`: restricts to numbers
- `$var$upper`, `$var$lower`, `$var$capitalized`: restricts case
- `$var$word`, `$var$identifier`, `$var$hex`, `$var$path`, `$var$binary`, `$var$percent`
- `$var$aliases{a,b,c}`: matches one of the listed
- `$var$optional{literal}`: matches literal if present, else empty
- `$var$starts{literal}`, `$var$ends{literal}`: must start/end with literal

Escaping
--------
- Use `\$` to insert a literal `$`.

Custom Delimiters
-----------------
- Use the extended API to change sigil and delimiters:
```c
papagaio_process_ex(ctx, "@", "<", ">", "@name", "Hello <@name>", NULL);
```

How It Works
------------
- Patterns are applied in order. Each match replaces the matched text with the replacement, then continues scanning.
- `$eval{}` blocks are evaluated after replacements are inserted.
- All output strings are malloc'd; free them after use (C API).

For more, see the code or open an issue with your use case.