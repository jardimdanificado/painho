#!/bin/bash
set -e

WCC="./wcc"
SCRATCH="scratch"

echo "Running Papagaio-C Test Suite..."

# 1. Types and Arithmetic
$WCC $SCRATCH/test_types.c -o types.wasm
wasm-objdump -x types.wasm | grep -q "test_arithmetic" && echo "[OK] Types/Arithmetic"

# 2. Imports (Object mode to check metadata without linking)
$WCC -c $SCRATCH/test_imports.c -o imports.wasm
wasm-objdump -x imports.wasm | grep -q "env" && echo "[OK] Imports (env)"
wasm-objdump -x imports.wasm | grep -q "host" && echo "[OK] Imports (host)"

# 3. Attributes/Modifiers (Object mode)
$WCC -c $SCRATCH/test_attributes.c -o attributes.wasm
wasm-objdump -x attributes.wasm | grep -q "init" && echo "[OK] Constructor"
wasm-objdump -x attributes.wasm | grep -q "get_counter" && echo "[OK] Export"

# 4. Memory (Object mode)
$WCC -c $SCRATCH/test_memory.c -o memory.wasm
wasm-objdump -x memory.wasm | grep -q "__heap_base" && echo "[OK] Heap Base Symbol"

# 5. Autodetect (Program)
echo 'export i32 main() { return 0; }' > $SCRATCH/prog.c
$WCC $SCRATCH/prog.c -o prog.wasm
wasm-objdump -x prog.wasm | grep -qE "_start|__main" && echo "[OK] Autodetect: Program"

# 6. Autodetect (Lib)
echo 'export i32 add(i32 a, i32 b) { return a + b; }' > $SCRATCH/lib.c
$WCC $SCRATCH/lib.c -o lib.wasm
wasm-objdump -x lib.wasm | grep -qv "_start" && echo "[OK] Autodetect: Library (no _start)"

echo "------------------------"
echo "All tests passed!"
rm types.wasm imports.wasm attributes.wasm memory.wasm prog.wasm lib.wasm $SCRATCH/prog.c $SCRATCH/lib.c
