#!/usr/bin/env bash
# build_host.sh — compiles the wasm3 host runner

WASM3_SRC="../wasm3/source"

[[ -d "$WASM3_SRC" ]] || { echo "✗ Error: $WASM3_SRC not found"; exit 1; }

echo "▶ compiling host runner..."

gcc runtime/host_example.c runtime/wasm3_host.c \
    -I "$WASM3_SRC" \
    -L "$WASM3_SRC" \
    -lm3 -lm \
    -o host

if [[ -f "./host" ]]; then
    echo "✓ host created"
else
    echo "✗ failed to create host"
    exit 1
fi
