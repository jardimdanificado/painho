Papagaio
========

Papagaio is a C-first, embeddable text processing and pattern-matching preprocessor. It exposes a small, well-documented C API, a native Lua module, a command-line executable, and a JavaScript/WASM wrapper for browser and Node.js usage. 

By embedding a Lua VM (compatible with Lua 5.1-5.4 and LuaJIT), Papagaio allows you to run dynamic evaluation blocks in-process. It provides highly flexible pattern and replacement APIs designed for rapid, scriptable source preprocessing and code generation.

What Papagaio Does
------------------

At its core, Papagaio is a preprocessor engine. It scans input text and performs pattern-based substitutions alongside scripted evaluations. You can use it to build macros, pattern-driven code generators, template engines, or any text-processing pipeline where embedding a tiny, efficient scripting VM is advantageous.

Quick Start
-----------

Command Line Interface (CLI):
```sh
papagaio input.txt > output.txt
```

C API:
```c
Papagaio *ctx = papagaio_open();
char *out = papagaio_process(ctx, "pattern", "replacement", NULL);
printf("%s", out);
free(out);
papagaio_close(ctx);
```

JavaScript / WASM (Node.js or Browser):
```javascript
import Papagaio from './papagaio.js';

const p = new Papagaio();
await p.init();
console.log(p.process('your text here'));
```

Pattern Syntax
--------------

Patterns are composed of whitespace-separated tokens. The syntax places the variable name first, followed by an optional modifier.

- Literal: Matches exactly the provided text.
- Variable: `$name` (captures one token sequence).
- Optional capture: `$name?` (capture is optional).

Modifiers allow you to specify the exact type of match you want:
- Numbers: `$var$int`, `$var$float`, `$var$number`
- Casing: `$var$upper`, `$var$lower`, `$var$capitalized`
- Special formats: `$var$word`, `$var$identifier`, `$var$hex`, `$var$path`, `$var$binary`, `$var$percent`
- Regex: `$id$regex{[0-9]+}` (captures matching a custom regular expression)
- Block: `$item$block{[}{]}` (captures everything inside the specified delimiters)
- Aliases: `$kind$aliases{cat,dog}` (matches one of the listed exact words)
- Fixed matches: `$var$optional{literal}`, `$var$starts{literal}`, `$var$ends{literal}`
- Advanced Substrings:
  - `$var$prefix{literal}`: Matches if the literal is at the beginning (and more characters follow).
  - `$var$suffix{literal}`: Matches if the literal is at the end (and characters precede it).
  - `$var$infix{literal}`: Matches if the literal is in the middle (not at start or end).
  - `$var$includes{literal}`: Matches if the literal is anywhere within the capture.

Replacement Syntax
------------------

- Variables: Use `$name` to insert a captured token.
- Evaluation Blocks: Use `$eval{ ... }` to run Lua code on the fly and insert its returned result. 

Inside an `$eval{}` block, you have access to the full match string via the `match` variable, and any captured variables in your pattern are automatically expanded inside the string prior to Lua execution, making it trivial to process matched inputs sequentially.

Examples
--------

1. Basic Data Extraction
```text
Pattern:      $name $age$int
Replacement:  Name: $name, Age: $age
Input:        Alice 42
Output:       Name: Alice, Age: 42
```

2. Mathematical Evaluation
```text
Pattern:      calc $x
Replacement:  $eval{return $x * 2}
Input:        calc 3
Output:       6
```

3. Block Matching
```text
Pattern:      $item$block{[}{]}
Replacement:  Item: $item
Input:        [a] [b]
Output:       Item: a Item: b
```

4. Aliases and Optional Captures
```text
Pattern:      $kind$aliases{cat,dog} $name?
Replacement:  Kind: $kind, Name: $name
Input:        dog Rover
Output:       Kind: dog, Name: Rover

Input:        cat
Output:       Kind: cat, Name: 
```

Customization
-------------

You can change the control sigil and delimiters dynamically during processing:

```text
$changequote{@}{<}{>} @eval<return 1+1>
```

This outputs `2`. You can also configure identical behaviors from the C extended processing API:
```c
papagaio_process_ex(ctx, "@", "<", ">", "@name", "Hello <@name>", NULL);
```

Public Interfaces
-----------------

C API Highlights (Header: src/papagaio.h):
- papagaio_open: Creates a new context and initializes a fresh Lua state.
- papagaio_close: Destroys a context and frees all resources.
- papagaio_set_args: Passes program arguments into the internal Lua state.
- papagaio_process: Convenience varargs API for generic processing.
- papagaio_process_pairs: Explicit explicit-context API handling raw arrays of patterns and replacements.

Note: Memory safety is heavily prioritized. C string results allocated during processing must be explicitly freed using `free()` by the caller avoiding leaks within embedded contexts.

Building from Source
--------------------

A C compiler (GCC or Clang) and CMake (>= 3.10) are required. A Lua development environment is optional; the library natively embeds its own VM bindings when building.

```sh
mkdir build && cd build
cmake ..
make
sudo make install
```

This generates `libpapagaio.a`, `papagaio.so`, and the `papagaio` CLI binary. 

For the WASM build:
```sh
make wasm
```

Design & Architecture Notes
---------------------------

- Small Surface Area: Papagaio favors a strictly minimal public API making it straightforward to link and distribute.
- Seamless Regex: Native Unicode-aware regular expression support is provided internally via a libregexp subcomponent.
- Lua Integration: The inclusion of an internal VM avoids massive abstraction overheads allowing lightweight but incredibly powerful on-the-fly transformations.
- Stateless Defaults: Calling convenience API methods without passing an explicitly initialized context uses a lazy, internal global VM instance. This behavior is ideal for one-shot operations, though standard explicitly-managed contexts are heavily recommended for complex, multi-threaded application pipelines.

License
-------

Please refer to the upstream repository for specific licensing information before adopting Papagaio for restricted commercial distributions.