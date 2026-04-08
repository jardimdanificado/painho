#!/usr/bin/env bash
set -euo pipefail

# bin/build_plugin.sh — compila um arquivo .c em um plugin .wasm para o Papagaio
# Uso: ./bin/build_plugin.sh entrada.c [saida.wasm]

ENTRY="${1:-}"
OUTPUT="${2:-}"

if [[ -z "$ENTRY" ]]; then
    echo "Uso: $0 <entrada.c> [saida.wasm]"
    exit 1
fi

if [[ -z "$OUTPUT" ]]; then
    BASENAME=$(basename "$ENTRY")
    OUTPUT="${BASENAME%.*}.wasm"
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK_H="$ROOT/lib/wasm-libc/include/papagaio_wasm.h"

# Garante que o SDK unificado existe
if [[ ! -f "$SDK_H" ]]; then
    echo "▶ SDK unificado não encontrado. Gerando..."
    "$ROOT/lib/wasm-libc/amalgamate.sh"
fi

echo "▶ Compilando $ENTRY (Wasm SDK Unificado)..."

# Compilação direta usando o header amalgamated
# Nota: O arquivo de entrada deve ter:
#   #define PAPAGAIO_WASM_IMPLEMENTATION
#   #include "papagaio_wasm.h"

clang --target=wasm32-unknown-unknown \
    -O2 -Wall -Wextra \
    -ffreestanding \
    -fno-builtin \
    -nostdlib \
    -nostdinc \
    -I "$(dirname "$SDK_H")" \
    -mno-bulk-memory \
    -mno-sign-ext \
    -Wl,--no-entry \
    -Wl,--export-all \
    -Wl,--allow-undefined \
    -Wl,-z,stack-size=65536 \
    "$ENTRY" \
    -o "$OUTPUT"

echo "✓ Sucesso: $OUTPUT"
