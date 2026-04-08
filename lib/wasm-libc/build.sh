#!/usr/bin/env bash
# build.sh — compila wasm-libc com clang bare-metal (target: wasm32-unknown-unknown)
# Uso:
#   ./build.sh           — compila a libc (.a) + test.wasm
#   ./build.sh lib       — só a libc
#   ./build.sh test      — só o módulo de teste
#   ./build.sh clean     — limpa artefatos

set -euo pipefail

# ── configuração ──────────────────────────────────────────────────────────── #

TARGET="wasm32-unknown-unknown"
CC="${WASM_CC:-clang}"
AR="${WASM_AR:-llvm-ar}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT="$ROOT/build"
LIB="$OUT/libwasm-libc.a"

CFLAGS=(
    --target="$TARGET"
    -std=c11
    -O2
    -Wall
    -Wextra
    -ffreestanding          # sem runtime de C padrão
    -fno-builtin            # não substituir chamadas por builtins
    -fno-stack-protector
    -fno-exceptions
    -nostdlib
    -nostdinc
    -I "$ROOT/include"
    # wasm3 é MVP; desativa extensões não suportadas
    -mno-bulk-memory
    -mno-sign-ext
)

LDFLAGS=(
    --entry=_start
    --export=wasm_main
    --export=_start
    #--no-entry
    #--import-memory              # memória importada do host
    --export-dynamic
    --allow-undefined            # __host_write, __host_abort resolvidos em runtime
    -z stack-size=65536
)

# fontes da libc
LIBC_SRCS=(
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

# ── funções ───────────────────────────────────────────────────────────────── #

die() { echo "✗ $*" >&2; exit 1; }
log() { echo "  $*"; }

check_deps() {
    command -v "$CC" >/dev/null 2>&1 || die "clang não encontrado. Instale com: apt install clang"
    command -v "$AR" >/dev/null 2>&1 || die "llvm-ar não encontrado. Instale com: apt install llvm"

    # verifica suporte ao target wasm32
    "$CC" --target="$TARGET" -x c -c /dev/null -o /dev/null 2>/dev/null \
        || die "clang não suporta $TARGET. Verifique a instalação."
}

build_lib() {
    echo "▶ compilando libc..."
    mkdir -p "$OUT/obj"

    local obj_files=()
    for src in "${LIBC_SRCS[@]}"; do
        local obj="$OUT/obj/$(basename "${src%.c}").o"
        log "cc $src"
        "$CC" "${CFLAGS[@]}" -c "$ROOT/$src" -o "$obj"
        obj_files+=("$obj")
    done

    log "ar → $(basename "$LIB")"
    "$AR" rcs "$LIB" "${obj_files[@]}"
    echo "✓ $LIB"
}

build_test() {
    echo "▶ compilando test.wasm..."

    local test_src="$ROOT/test/test_main.c"
    local test_out="$OUT/test.wasm"

    [[ -f "$test_src" ]] || die "test/test_main.c não encontrado"

    # compila objeto do teste
    "$CC" "${CFLAGS[@]}" -c "$test_src" -o "$OUT/obj/test_main.o"

    # linka com wasm-ld
    wasm-ld \
        "${LDFLAGS[@]}" \
        "$OUT/obj/test_main.o" \
        "$LIB" \
        -o "$test_out"

    echo "✓ $test_out"
    echo ""
    echo "Para rodar com wasm3:"
    echo "  wasm3 --func main $test_out"
}

clean() {
    rm -rf "$OUT"
    echo "✓ limpo"
}

# ── main ──────────────────────────────────────────────────────────────────── #

check_deps

case "${1:-all}" in
    lib)   build_lib ;;
    test)  build_lib && build_test ;;
    clean) clean ;;
    all)   build_lib && build_test ;;
    *)     die "uso: $0 [lib|test|clean|all]" ;;
esac
