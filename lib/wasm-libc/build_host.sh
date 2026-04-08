#!/usr/bin/env bash
# build_host.sh — compila o runner host do wasm3

WASM3_SRC="../wasm3/source"

[[ -d "$WASM3_SRC" ]] || { echo "✗ Erro: $WASM3_SRC não encontrado"; exit 1; }

echo "▶ compilando host runner..."

gcc runtime/host_example.c runtime/wasm3_host.c \
    -I "$WASM3_SRC" \
    -L "$WASM3_SRC" \
    -lm3 -lm \
    -o host

if [[ -f "./host" ]]; then
    echo "✓ host criado"
else
    echo "✗ falha ao criar host"
    exit 1
fi
