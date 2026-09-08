# Papagaio — Auditoria e Tabela de Migração (Fase 1)

Este documento cumpre os requisitos da **Fase 1 (Auditoria)** descritos no [papagaio_planejamento.md](../papagaio_planejamento.md).

---

## 1. Catalogação de Funcionalidades do Papagaio Atual

A auditoria cobriu o código em C (`src/papagaio.c`, `src/papagaio.h`), a biblioteca matemática `louro`, as diretivas de linha de comando (`src/cli.js`), exemplos e os 72 casos de testes em `tests/tests.json`.

### 1.1 Elementos de Sintaxe de Padrão (Matcher)
1. **Tokens Literais**: Sequências de caracteres fixos correspondidos exatamente (`TOK_LITERAL`).
2. **Tokens Variáveis**: Declarados por `$nome` (`TOK_VAR`). Capturam texto até o próximo delimitador/padrão.
3. **Variáveis com Chaves**: `${nome}` em replacements e interpolações para evitar ambiguidades com texto contíguo (`${id}x`).
4. **Opcionalidade (`?`)**: Pode ser anexada a literais (`foo?`), variáveis (`$x?`) ou blocos (`$x$block{...}?`).
5. **Consumo de Espaço com Trailing Sigil (`$`)**: Um sigil anexado ao final de um token (`$a$ $b` ou `$literal$ $b`) colapsa/consome qualquer quantidade de espaços em branco adjacentes.
6. **Flex-matching**: O motor pula automaticamente espaços em branco horizontais entre variáveis se não houver um literal rígido separando.
7. **Blocos balanceados (`$x$block{o}{c}`)**: Captura recursiva considerando o pareamento de delimitadores (ex: `( )`, `< >`, `[ ]`, `{ }`). Suporta delimitadores multi-caracteres e unescape.

---

### 1.2 Catálogo de Modifiers Existentes

| Modifier | Sintaxe Atual | Entrada / Restrição | Saída Capturada | Matching | Equivalente JS | Decisão no Novo Papagaio |
|---|---|---|---|---|---|---|
| `int` | `$v$int` | `[0-9]+` ou com `-` inicial | String numérica inteira | Rejeita se contiver não-dígito | `Number.isInteger(Number(v))` | **Preservar** (primitiva de pattern matching) |
| `float` | `$v$float` | Dígitos com `.` e opcional `-` | String decimal | Rejeita se formato inválido | `!isNaN(Number(v))` | **Preservar** (pattern matching) |
| `number` | `$v$number` | Dígitos, ponto decimal, `-` | String de número | Rejeita caracteres inválidos | `!isNaN(Number(v))` | **Preservar** (pattern matching) |
| `upper` | `$v$upper` | Todos maiúsculos | String em caixa alta | Rejeita caracteres em caixa baixa | `v === v.toUpperCase()` | **Preservar** (pattern matching) |
| `lower` | `$v$lower` | Todos minúsculos | String em caixa baixa | Rejeita caracteres em caixa alta | `v === v.toLowerCase()` | **Preservar** (pattern matching) |
| `capitalized` | `$v$capitalized` | Primeira maiúscula, resto minúsculas | String capitalizada | Rejeita caso contrário | Regex `/^[A-Z][a-z]*$/` | **Preservar** (pattern matching) |
| `word` | `$v$word` | Caracteres alfabéticos (`isalpha`) | Palavra | Rejeita dígitos/pontuação | Regex `/^[a-zA-Z]+$/` | **Preservar** (pattern matching) |
| `identifier` | `$v$identifier` | `[a-zA-Z_][a-zA-Z0-9_]*` | Identificador válido | Rejeita se iniciar com dígito | Regex `/^[a-zA-Z_]\w*$/` | **Preservar** (pattern matching) |
| `hex` | `$v$hex` | Dígitos hexadecimais (com ou sem `0x`) | Hex string | Rejeita não-hex | Regex `/^(0x)?[0-9a-fA-F]+$/` | **Preservar** (pattern matching) |
| `path` | `$v$path` | Qualquer caractere exceto espaço/quebra | Caminho/token sem espaço | Pára no whitespace | `!/\s/.test(v)` | **Preservar** (pattern matching) |
| `binary` | `$v$binary` | Dígitos `0`, `1` e prefixos `b`/`B` | Sequência binária | Rejeita outros caracteres | Regex `/^0?b?[01]+$/i` | **Preservar** (pattern matching) |
| `percent` | `$v$percent` | Número seguido de `%` | Percentual | Rejeita sem `%` | Regex `/^-?\d+(\.\d+)?%$/` | **Preservar** (pattern matching) |
| `block` | `$v$block{o}{c}` | Conteúdo entre `o` e `c` balanceados | Conteúdo interno | Consome blocos aninhados balanceados | Parser recursivo Papagaio | **Preservar** (essencial para DSLs/linguagens) |
| `aliases` | `$v$aliases{a}{b}...` | Conjunto de opções literais ou subpadrões | O valor correspondente | Testa lista de alternativas | regex `(a|b)` / match recursivo | **Preservar** (fundamental para uniões/alternativas) |
| `group` | `$v$group{pat}` | Subpadrão completo | Trecho casado | Avalia subpadrão recursivo | Sub-match Papagaio | **Preservar** (agrupamento recursivo) |
| `starts` / `prefix` | `$v$starts{p}` | Começa com prefixo literal ou subpadrão | Conteúdo com o prefixo | Valida prefixo do capture | `v.startsWith(p)` | **Preservar** (validação semântica de pattern) |
| `ends` / `suffix` | `$v$ends{s}` | Termina com sufixo | Conteúdo com o sufixo | Valida sufixo do capture | `v.endsWith(s)` | **Preservar** (validação semântica de pattern) |
| `infix` | `$v$infix{x}` | Contém `x` no meio (nem início nem fim) | Conteúdo com infix | Valida miolo do capture | JS custom | **Preservar** (validação interna de tokens) |
| `includes` | `$v$includes{x}` | Contém `x` em qualquer posição | Conteúdo casado | Valida presença de `x` | `v.includes(x)` | **Preservar** (restrição de match) |
| `alpha` | `$v$alpha` | Letras alfabéticas | Texto convertido para maiúsculo | Rejeita não-letras | `v.toUpperCase()` | **Preservar** (built-in histórico) |
| `alphanum` | `$v$alphanum` | Alfanumérico | Texto alfanumérico | Rejeita caracteres especiais | `v.match(/^[a-z0-9]+$/i)` | **Preservar** (built-in histórico) |
| `custom` | `$v$custom_name` | Definido por função registrada | Retorno da função | Executa callback | Callback JS | **Preservar** via API de registro de modifiers |

---

### 1.3 Diretivas de Pré-processamento e Comandos

| Diretiva / Operador | Descrição Original | Ação no Novo Papagaio | Justificativa |
|---|---|---|---|
| `$pattern{p}{r}` | Define regra de substituição de padrão | **Preservar** em `papagaio.replace` / `.compile()` | Core textual do Papagaio |
| `$from{v}` | Atribuição dinâmica de variáveis com escopo | **Remover do template** / **Usar JS Objects** | Em JS usamos objetos normais `{ nome: "..." }` ou closures |
| `$list{sep}$op` | Métodos de lista: `get`, `set`, `push`, `pop`, `shift`, `unshift`, `slice`, `join`, `reverse`, `count`, etc. | **Remover** | JavaScript nativo já possui Arrays e métodos (`map`, `filter`, `slice`, `pop`, `join`, etc.) |
| `$math{expr}` | Avaliador matemático e lógico Louro | **Remover sintaxe `$math`** | JavaScript ES2023 possui operadores `+ - * / % **`, `Math.*` e expressões embutidas |
| `$compare{target}` | Condicional de string | **Remover** | Operadores `===`, `!==`, ternário `? :` em JS |
| `$then{content}` | Executa se não-vazio | **Remover** | `val ? content : ""` ou `if (val)` |
| `$else{content}` | Executa se vazio | **Remover** | `val || default` ou `if (!val)` |
| `$repeat{N}{code}` | Repete bloco N vezes | **Remover** | `for`, `while`, ou `Array.from({length: N})` |
| `$while{pat}{code}` | Loop enquanto casa padrão | **Remover** | Loops nativos do JS `while (...)` |
| `$until{pat}{code}` | Loop até casar padrão | **Remover** | Loops nativos do JS `do ... while` |
| `$contains{pat}` | Busca índice de substring | **Remover** | `str.indexOf()` ou `str.includes()` |
| `$slice{s}{e}` | Fatiamento de string | **Remover** | `str.slice(s, e)` nativo |
| `$replace{p}{r}` | Substituição | **Adaptar**: substituir por `papagaio.replace(p, in, ctx)` | JS já tem `String.prototype.replace()`, mas a versão do Papagaio deve ser para patterns Papagaio |
| `$byte{code}` | Inserção de byte | **Remover** | `String.fromCharCode(code)` ou escapes hex/unicode |
| `$ascii{code}` / `$ascii$code` | Inserção de caractere por código ASCII | **Remover** | `String.fromCharCode(code)` |
| `$space`, `$newline`, `$tab` | Emissão de whitespace literal | **Remover sintaxe especial** | Template literals do JS suportam `\n`, `\t`, ` ` |
| `$sigil`, `$open`, `$close`, `$marker` | Emissão de símbolos especiais do motor | **Adaptar para escaping** | Escaping padrão (ex: `\$`, `\{`, etc.) |
| `$changesymbols` | Modificação em runtime dos delimitadores | **Preservar via opções de config** | Configurado via opções `{ sigil, open, close }` |
| `$include{file}` | Leitura de arquivo do disco | **Remover do core** | Responsabilidade do host / I/O da aplicação |
| `$import{so}` | Carregamento de DLL / plugin C | **Remover do core JS** | Sistema de módulos do JS (`import / require`) |
| `$args$0..N`, `$args$all` | Argumentos de CLI | **Remover do core** | Host fornece os dados no contexto |
| `$document`, `$document$original` | Auto-referência ao documento original | **Remover do core** | Passado explicitamente no contexto se necessário |
| `$once{...}` | Desativa recursão de patterns | **Não necessário** | O novo modelo tem compilação e substituição de passo único explícita |

---

## 2. Tabela de Migração Oficial

| Papagaio C / DSL Antiga | Novo Papagaio (JS ES2023) | Status | Detalhes de Migração |
|---|---|---|---|
| `"Olá $nome"` com `$nome$from{Jardel}` | `"Olá $nome".papagaio({ nome: "Jardel" })` | **Adaptado** | Contexto passado como objeto JS |
| `$pattern {rgb($r,$g,$b)} {...}` | `"rgb($r,$g,$b)".papagaio.match("rgb(255,0,0)")` | **Preservado** | Retorna `{ r: "255", g: "0", b: "0" }` |
| `$pattern {p} {r}` no texto | `"pattern".papagaio.replace(input, ctx)` | **Preservado** | Substituição com patterns, modifiers e captures |
| `$list{,}$get{0}` | `items[0]` | **Removido** | Uso de Array JS |
| `$list{,}$push{x}` | `items.push(x)` | **Removido** | Uso de Array JS |
| `$list{,}$join{ - }` | `items.join(" - ")` | **Removido** | Uso de Array JS |
| `$math{2 + 3 * 4}` | `${2 + 3 * 4}` | **Removido** | Avaliação nativa JS |
| `$math{sqrt(x)}` | `${Math.sqrt(x)}` | **Removido** | Uso do objeto `Math` padrão |
| `$compare{x}$then{A}$else{B}` | `${val === x ? "A" : "B"}` | **Removido** | Expressões ternárias JS |
| `$repeat{3}{...}` | Laço `for` em JS | **Removido** | Controle de fluxo JS |
| `$while{...}{...}` | Laço `while` em JS | **Removido** | Controle de fluxo JS |
| `$include{file.pap}` | `fs.readFileSync(file)` ou `fetch()` | **Removido** | I/O provido pelo ambiente |
| `$import{plugin.so}` | `import plugin from '...'` | **Removido** | Módulos JS padrão |
| `$changesymbols{@}{<}{>}` | `papagaio(str, ctx, { sigil: "@", open: "<", close: ">" })` | **Preservado** | Opções de configuração no engine |
| Modifiers de tipo (`$x$int`, `$x$hex`) | Suporte integral em `.match()` e `.replace()` | **Preservado** | Implementado no parser de tokens JS |
| Delimitadores balanceados (`$x$block{[}{]}`) | Suporte integral em `.match()` e `.replace()` | **Preservado** | Algoritmo de extração balanceada portado para JS |
| Alternativas e grupos (`$x$aliases{...}`) | Suporte integral em `.match()` e `.replace()` | **Preservado** | Algoritmo recursivo portado para JS |

---

## 3. Conclusão da Auditoria (Fase 1)

Com esta catalogação:
1. Identificamos as **funcionalidades centrais únicas** que justificam o Papagaio:
   - Pattern matching declarativo com flex-matching de espaços em branco;
   - Extração estruturada com captura de variáveis tipadas por modifiers (`int`, `float`, `hex`, `identifier`, etc.);
   - Captura de blocos balanceados arbitrários (`block{open}{close}`);
   - Alternativas com sub-padrões (`aliases`);
   - Grupos recursivos e modificadores posicionais (`starts`, `ends`, `prefix`, `suffix`, `infix`, `includes`);
   - Delimitadores e marcadores customizáveis.
2. Todas as redundâncias com JavaScript (matemática Louro, controle de fluxo, listas, manipulação básica de strings e I/O de disco) foram separadas para eliminação do core, permitindo uma biblioteca JS leve, rápida, portátil e sem dependências externas.
