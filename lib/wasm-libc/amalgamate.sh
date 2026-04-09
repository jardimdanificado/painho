#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT="$ROOT/include/lib.c"

HEADERS=(
    stdarg.h
    stddef.h stdint.h stdbool.h limits.h
    string.h stdio.h stdlib.h ctype.h
    setjmp.h time.h errno.h math.h
    locale.h assert.h
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
 * Papagaio Wasm SDK — lib.c
 * Unified Single-File Library (Headers + Implementation)
 * Generated automatically from wasm-libc.
 */

#ifndef PAPAGAIO_LIBC_H
#define PAPAGAIO_LIBC_H

/* ── Headers ─────────────────────────────────────────────────────────────── */
EOF

for h in "${HEADERS[@]}"; do
    echo "" >> "$OUTPUT"
    echo "/* --- $h --- */" >> "$OUTPUT"
    # Remove guards: #ifndef _FILENAME_H, #define _FILENAME_H
    sed -E '/^#(ifndef|define)[[:space:]]+_?[A-Z0-9]+_H([[:space:]]|$)/d' "$ROOT/include/$h" | \
    sed -E '/^#endif[[:space:]]+\/\*[[:space:]]+_?[A-Z0-9]+_H[[:space:]]+\*\//d' | \
    grep -Ev "#include[[:space:]]+[\"<]($HEADER_REGEX)[\">]" >> "$OUTPUT"
done

cat <<EOF >> "$OUTPUT"

#endif /* PAPAGAIO_LIBC_H */

/* ── Implementation ──────────────────────────────────────────────────────── */

EOF

for s in "${SOURCES[@]}"; do
    echo "" >> "$OUTPUT"
    echo "/* --- $s --- */" >> "$OUTPUT"
    grep -Ev "#include[[:space:]]+[\"<]($HEADER_REGEX)[\">]" "$ROOT/$s" >> "$OUTPUT"
done

echo "" >> "$OUTPUT"

echo "✓ $OUTPUT successfully generated."
