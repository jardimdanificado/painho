#!/usr/bin/env bash
# build.sh — compiles wasm-libc with bare-metal clang (target: wasm32-unknown-unknown)
# Usage:
#   ./build.sh           — compiles libc (.a) + test.wasm
#   ./build.sh lib       — only libc
#   ./build.sh test      — only test module
#   ./build.sh clean     — clean artifacts

set -euo pipefail

# ── configuration ──────────────────────────────────────────────────────────── #

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
    -ffreestanding          # no standard C runtime
    -fno-builtin            # do not replace calls with builtins
    -fno-stack-protector
    -fno-exceptions
    -nostdlib
    -nostdinc
    -I "$ROOT/include"
    # wasm3 is MVP; deactivate unsupported extensions
    -mno-bulk-memory
    -mno-sign-ext
)

LDFLAGS=(
    --entry=_start
    --export=wasm_main
    --export=_start
    #--no-entry
    #--import-memory              # imported memory from host
    --export-dynamic
    --allow-undefined            # __host_write, __host_abort resolved at runtime
    -z stack-size=65536
)

# libc sources
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

# ── functions ───────────────────────────────────────────────────────────────── #

die() { echo "✗ $*" >&2; exit 1; }
log() { echo "  $*"; }

check_deps() {
    command -v "$CC" >/dev/null 2>&1 || die "clang not found. Install with: apt install clang"
    command -v "$AR" >/dev/null 2>&1 || die "llvm-ar not found. Install with: apt install llvm"

    # check support for wasm32 target
    "$CC" --target="$TARGET" -x c -c /dev/null -o /dev/null 2>/dev/null \
        || die "clang does not support $TARGET. Check your installation."
}

build_lib() {
    echo "▶ compiling libc..."
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
    echo "▶ compiling test.wasm..."

    local test_src="$ROOT/test/test_main.c"
    local test_out="$OUT/test.wasm"

    [[ -f "$test_src" ]] || die "test/test_main.c not found"

    # compila objeto do teste
    "$CC" "${CFLAGS[@]}" -c "$test_src" -o "$OUT/obj/test_main.o"

    # link with wasm-ld
    wasm-ld \
        "${LDFLAGS[@]}" \
        "$OUT/obj/test_main.o" \
        "$LIB" \
        -o "$test_out"

    echo "✓ $test_out"
    echo ""
    echo "To run with wasm3:"
    echo "  wasm3 --func main $test_out"
}

clean() {
    rm -rf "$OUT"
    echo "✓ clean"
}

# ── main ──────────────────────────────────────────────────────────────────── #

check_deps

case "${1:-all}" in
    lib)   build_lib ;;
    test)  build_lib && build_test ;;
    clean) clean ;;
    all)   build_lib && build_test ;;
    *)     die "usage: $0 [lib|test|clean|all]" ;;
esac
