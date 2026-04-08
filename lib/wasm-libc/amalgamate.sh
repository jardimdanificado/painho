#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT="$ROOT/include/papagaio_wasm.h"

HEADERS=(
    stddef.h stdint.h stdbool.h limits.h
    string.h stdio.h stdlib.h ctype.h
    setjmp.h time.h errno.h math.h
)

SOURCES=(
    src/string/string.c
    src/memory/malloc.c
    src/stdio/printf.c
    src/stdlib/stdlib.c
    src/stdlib/ctype.c
    src/stdlib/setjmp.c
    src/stdlib/time.c
    src/stdlib/errno.c
    src/string/strerror.c
    src/math/math.c
    src/stdlib/locale.c
    src/stdio/scanf.c
)

HEADER_REGEX=$(echo "${HEADERS[@]}" | sed 's/ /|/g' | sed 's/\./\\./g')

cat <<EOF > "$OUTPUT"
/* 
 * Papagaio Wasm SDK — Unified Single-Header Library
 * Generated automatically from wasm-libc.
 */

#ifndef PAPAGAIO_WASM_H
#define PAPAGAIO_WASM_H

#include <stdarg.h>

/* ── SDK Macros ──────────────────────────────────────────────────────────── */

#define PAP_COMMAND(name) \\
    static char *intern_cmd_##name(int argc, const char **argv); \\
    __attribute__((export_name("pap_cmd_" #name))) \\
    char* pap_bridge_##name(int argc, uint32_t argv_ptr) { \\
        const char **argv = (const char **)malloc(argc * sizeof(const char *)); \\
        if (!argv) return NULL; \\
        uint32_t *table = (uint32_t *)argv_ptr; \\
        for (int i = 0; i < argc; i++) argv[i] = (const char *)table[i]; \\
        char *res = intern_cmd_##name(argc, argv); \\
        free((void*)argv); \\
        return res; \\
    } \\
    static char *intern_cmd_##name(int argc, const char **argv)

/* ── Headers ─────────────────────────────────────────────────────────────── */
EOF

for h in "${HEADERS[@]}"; do
    echo "" >> "$OUTPUT"
    echo "/* --- $h --- */" >> "$OUTPUT"
    # Remove guards: #ifndef _FILENAME_H, #define _FILENAME_H
    # Precisamos de âncoras (boundary) para não casar _CT_HX quando procuramos _CTYPE_H
    sed -E '/^#(ifndef|define)[[:space:]]+_?[A-Z0-9]+_H([[:space:]]|$)/d' "$ROOT/include/$h" | \
    sed -E '/^#endif[[:space:]]+\/\*[[:space:]]+_?[A-Z0-9]+_H[[:space:]]+\*\//d' | \
    grep -Ev "#include[[:space:]]+[\"<]($HEADER_REGEX)[\">]" >> "$OUTPUT"
done

cat <<EOF >> "$OUTPUT"

#endif /* PAPAGAIO_WASM_H */

/* ── Implementation ──────────────────────────────────────────────────────── */

#ifdef PAPAGAIO_WASM_IMPLEMENTATION
EOF

for s in "${SOURCES[@]}"; do
    echo "" >> "$OUTPUT"
    echo "/* --- $s --- */" >> "$OUTPUT"
    grep -Ev "#include[[:space:]]+[\"<]($HEADER_REGEX)[\">]" "$ROOT/$s" >> "$OUTPUT"
done

echo "" >> "$OUTPUT"
echo "#endif /* PAPAGAIO_WASM_IMPLEMENTATION */" >> "$OUTPUT"

echo "✓ $OUTPUT gerado com sucesso."
