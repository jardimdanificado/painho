# wasm-libc

Implementação bare-metal de uma libc para WebAssembly, compilada com `clang`
e interpretada com [wasm3](https://github.com/wasm3/wasm3).

## Filosofia

- **Zero dependências** — nenhuma libc do sistema, nenhuma WASI
- **ABI explícita** — I/O e controle de fluxo são imports declarados do host
- **Auditável** — cada função tem no máximo 50–100 linhas, sem macros complexas
- **wasm3-safe** — MVP do WebAssembly apenas; sem threads, SIMD ou bulk-memory

## Estrutura

```
wasm-libc/
├── include/          headers públicos
│   ├── stddef.h      size_t, NULL, offsetof
│   ├── stdint.h      int32_t, uint64_t, ...
│   ├── stdbool.h     bool, true, false
│   ├── limits.h      INT_MAX, CHAR_BIT, ...
│   ├── string.h      mem*, str*
│   ├── stdio.h       printf, snprintf, puts
│   └── stdlib.h      malloc, free, atoi, qsort, rand
│
├── src/
│   ├── string/       memcpy, memmove, memset, strcmp, strtok_r, ...
│   ├── memory/       malloc, free, calloc, realloc (free-list + boundary tags)
│   ├── stdio/        printf, sprintf, snprintf, vsnprintf (todos os specs)
│   └── stdlib/       atoi, strtol, itoa, qsort, bsearch, rand, abs
│
├── test/
│   └── test_main.c   suite de testes unitários
│
└── build.sh          script de compilação
```

## Requisitos

```bash
# Ubuntu / Debian
apt install clang llvm lld

# Arch Linux
pacman -S clang llvm lld

# macOS (Homebrew)
brew install llvm
```

Para rodar os testes:
```bash
# instala wasm3
git clone https://github.com/wasm3/wasm3 && cd wasm3
cmake . && make -j4
sudo cp build/wasm3 /usr/local/bin/
```

## Build

```bash
chmod +x build.sh

./build.sh        # compila libc + test.wasm
./build.sh lib    # só a libc estática (.a)
./build.sh test   # compila e linka o test.wasm
./build.sh clean  # remove artefatos
```

## Rodando os testes

```bash
wasm3 --func main build/test.wasm
```

Saída esperada:
```
wasm-libc test suite
====================

[string]
  OK   strlen básico
  OK   strlen vazio
  ...

[malloc]
  OK   malloc retorna não-NULL
  ...

resultado: 42/42 testes passaram
```

## ABI de host imports

O módulo `"env"` deve exportar para o WASM:

| Função           | Assinatura C                          | Descrição          |
|------------------|---------------------------------------|--------------------|
| `__host_write`   | `(const char *buf, int len) -> void`  | stdout             |
| `__host_write_err`| `(const char *buf, int len) -> void` | stderr             |
| `__host_abort`   | `() -> void`                          | abort/panic        |

### Exemplo de binding em C (host wasm3)

```c
#include "wasm3.h"
#include "m3_env.h"

m3ApiRawFunction(host_write) {
    m3ApiGetArgMem(const char *, buf, 0);
    m3ApiGetArg(int32_t, len, 1);
    fwrite(buf, 1, (size_t)len, stdout);
    m3ApiSuccess();
}

m3ApiRawFunction(host_abort) {
    abort();
}

// na inicialização:
m3_LinkRawFunction(module, "env", "__host_write",     "v(*i)", host_write);
m3_LinkRawFunction(module, "env", "__host_write_err", "v(*i)", host_write);
m3_LinkRawFunction(module, "env", "__host_abort",     "v()",   host_abort);
```

## Alocador de memória

Implementa **free-list com boundary tags** (Knuth, 1973):

```
[ header 8B | payload ... | footer 4B ]
   size|flags              size|flags
```

- **Coalescing** em O(1) para frente e para trás usando boundary tags
- **Splitting** quando o bloco tem espaço sobrando para um bloco mínimo
- **heap_grow** usa `memory.grow` do WASM — sem syscall, sem OS
- **Alinhamento** de 8 bytes em todos os blocos

## Módulos planejados

- [ ] `math.h` — sin, cos, sqrt, pow (implementações puras em C)
- [ ] `ctype.h` — isalpha, isdigit, toupper, tolower
- [ ] `errno.h` — códigos de erro
- [ ] `setjmp.h` — longjmp bare-metal para WASM
- [ ] `time.h` — com host import para clock
