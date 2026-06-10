# Louco Transpiler Example

This example demonstrates how to use Papagaio's plugin system to build an Ahead-Of-Time (AOT) transpiler for Louro math expressions directly into C code, without modifying Papagaio's core.

## Files
- `aot_plugin.c`: The C plugin that registers `$aot_thunks` and `$aot_c` commands. It uses Papagaio's internal Louro engine to parse the expression and walk the AST to generate C code.
- `aot_template.pap`: The Papagaio template that loads the plugin and formats the generated C code into a complete C file.
- `my_env.h`: A sample environment header that defines variables (`player_x`, `speed`, etc.) used in the script.
- `script.txt`: A sample Louro expression.

## How to Compile and Run

1. Compile the plugin into a shared library:
```bash
gcc -shared -fPIC -I../../src -DENV_HEADER='"my_env.h"' aot_plugin.c -o aot_plugin.so
```

2. Run the transpiler using Papagaio:
```bash
../../papagaio aot_template.pap "$(cat script.txt)"
```

The output will be the complete, transpiled C code of your script!
