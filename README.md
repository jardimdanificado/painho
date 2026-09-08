# Papagaio 🦜

> **JavaScript é a linguagem. Papagaio é a extensão textual.**

O **Papagaio** é uma biblioteca leve e sem dependências para **processamento textual, pattern matching flexível e interpolação de templates**. Ele age como uma extensão natural de `String` no ecossistema JavaScript (ES2023), projetado com foco em portabilidade estrita (Node.js, Bun, Deno, Browsers e **QuickJS**).

Em vez de reinventar arrays, matemática ou controle de fluxo, o Papagaio delega a computação ao JavaScript nativo e concentra-se no que faz de melhor: **flex-matching de espaços em branco, captura de blocos balanceados aninhados, modifiers semânticos de tokens e metaprogramação de sintaxe**.

---

## ✨ Principais Funcionalidades

- 🧵 **Extensão de `String.prototype` & API Funcional**: Use `"Olá $nome".papagaio(...)` ou `papagaio(str, ...)`.
- 🧩 **Pattern Matching Declarativo**: Sintaxe intuitiva baseada em tokens com captura automática de variáveis.
- 📐 **Captura de Blocos Balanceados**: Extraia corpos de blocos aninhados (`$body$block{[}{]}`) respeitando profundidade sem regexes complexas.
- 🔍 **Modifiers Semânticos Embutidos**: Valide ou restrinja capturas diretamente no padrão (`$id$identifier`, `$val$int`, `$cor$hex`, `$item$aliases{a}{b}`).
- ⚡ **Flex-Matching**: Tolerância nativa a variações arbitrárias de espaçamento em branco entre tokens.
- 🪶 **Zero Dependências**: Núcleo puramente ES2023 executável até mesmo no engine ultraleve **QuickJS**.
- 🛠️ **Ideal para Compiladores e DSLs**: Excelente motor de desaçucaramento sintático e geração de código (ex: WAT/WebAssembly).

---

## 📦 Instalação

```bash
npm install papagaio
```

Ou importe diretamente no seu projeto:

```javascript
import { papagaio } from "papagaio";
// Ou importe para habilitar o método no protótipo de String:
import "papagaio";
```

---

## 🚀 Guia de Uso Rápido

### 1. Interpolação Textual e Expressões JS

Substitua variáveis `$var` ou avalie expressões completas em `${expressao}`:

```javascript
import "papagaio";

// Interpolação direta com contexto
const saudacao = "Olá $nome, seja bem-vindo!".papagaio({ nome: "Jardel" });
console.log(saudacao); // "Olá Jardel, seja bem-vindo!"

// Expressões JavaScript completas
const recibo = "Item: $item | Total: R$ ${preco * qtd} (${pago ? 'PAGO' : 'PENDENTE'})".papagaio({
  item: "Teclado Mecânico",
  preco: 250,
  qtd: 2,
  pago: true
});
console.log(recibo); // "Item: Teclado Mecânico | Total: R$ 500 (PAGO)"
```

### 2. Pattern Matching (`.match()`)

Extraia dados de strings estruturadas sem escrever expressões regulares indecifráveis:

```javascript
import { papagaio } from "papagaio";

// Via método no String.prototype:
const match = "rgb($r, $g, $b)".papagaio.match("rgb(255, 128, 64)");
console.log(match);
// { r: "255", g: "128", b: "64" }

// Ou via chamada funcional:
const dados = papagaio.match("pedido #$id: $status", "pedido #4829: enviado");
console.log(dados);
// { id: "4829", status: "enviado" }
```

### 3. Substituição Baseada em Padrões (`.replace()`)

Substitua trechos casados usando strings de substituição ou callbacks computados em JavaScript:

```javascript
import "papagaio";

const codigo = "soma(10, 20); soma(30, 40);";

// Callback com destructuring das capturas:
const resultado = "soma($a, $b)".papagaio.replace(codigo, ({ a, b }) => {
  return `${Number(a) + Number(b)}`;
});
console.log(resultado); // "30; 70;"
```

### 4. Pré-Compilação de Alta Performance (`.compile()`)

Para loops ou alta frequência de execução, compile o padrão antecipadamente:

```javascript
const formatador = "log: [$nivel] $msg".papagaio.compile();

console.log(formatador.match("log: [INFO] Servidor iniciado"));
// { nivel: "INFO", msg: "Servidor iniciado" }

console.log(formatador({ nivel: "WARN", msg: "Memória alta" }));
// "log: [WARN] Memória alta"
```

---

## 🎯 Modifiers de Pattern Matching

Modifiers restringem e validam os tipos de dados casados por cada variável do padrão:

| Modifier | Sintaxe | Descrição / Restrição | Exemplo Válido |
|---|---|---|---|
| **`int`** | `$v$int` | Números inteiros (permite `-`) | `42`, `-10` |
| **`float`** | `$v$float` | Números de ponto flutuante | `3.14`, `-0.05` |
| **`number`** | `$v$number` | Qualquer formato numérico aceito | `100`, `1e5` |
| **`identifier`** | `$v$identifier` | Identificador válido de linguagem (`[a-zA-Z_]\w*`) | `minhaVariavel_1` |
| **`word`** | `$v$word` | Somente letras alfabéticas | `Papagaio` |
| **`hex`** | `$v$hex` | Dígitos hexadecimais (com ou sem `0x`) | `0xff00aa`, `c0ffee` |
| **`upper`** | `$v$upper` | Apenas letras maiúsculas | `OK`, `CONSTANTE` |
| **`lower`** | `$v$lower` | Apenas letras minúsculas | `texto`, `slug` |
| **`capitalized`** | `$v$capitalized` | Primeira letra maiúscula | `Nome`, `Titulo` |
| **`binary`** | `$v$binary` | Dígitos binários (com ou sem `0b`) | `0b1010`, `1100` |
| **`path`** | `$v$path` | Sequência sem espaços | `/usr/bin/node` |
| **`percent`** | `$v$percent` | Número com sinal de percentagem | `100%`, `-5.5%` |
| **`block`** | `$v$block{o}{c}` | Conteúdo entre delimitadores balanceados `o` e `c` | `{ if (x) { y(); } }` |
| **`aliases`** | `$v$aliases{a}{b}` | Alternativas literais ou subpadrões | `gato` ou `cachorro` |
| **`starts`** | `$v$starts{pref}` | Valor que se inicia com o prefixo | `pre-requisito` |
| **`ends`** | `$v$ends{suf}` | Valor que termina com o sufixo | `arquivo.txt` |
| **`includes`** | `$v$includes{x}` | Valor que contenha o trecho `x` | `chave_teste_id` |

### Modifiers Customizados

Registre seus próprios modificadores de validação e transformação:

```javascript
import { papagaio } from "papagaio";

papagaio.registerModifier("email", (val) => {
  return val.includes("@") && val.includes(".");
});

const res = "$contato$email".papagaio.match("suporte@papagaio.dev");
console.log(res); // { contato: "suporte@papagaio.dev" }
```

---

## 🛠️ Exemplo Avançado: Criando Compiladores para WAT (WebAssembly)

O Papagaio é a ferramenta perfeita para quem deseja **inventar uma sintaxe de programação própria** e transformá-la diretamente em **WAT (WebAssembly Text)** validado e executável.

Confira na pasta [`examples/`](./examples) implementações completas e funcionais:
- [`examples/forth2wat/`](./examples/forth2wat): Compilador completo de **Forth clássico** (`DUP`, `SWAP`, `DO...LOOP`, `IF...ELSE`) para WAT.
- [`examples/lisp2wat/`](./examples/lisp2wat): Compilador de dialeto **Lisp / S-Expressions** com suporte a loops, funções recursivas e manipulação de memória linear WASM.

Exemplo de desaçucaramento sintático:

```javascript
import { papagaio } from "papagaio";

const codigoUsuario = "rotina dobro(n) { retorne n * 2 }";

const wat = "rotina $fn($arg) $corpo$block{[}{]}".papagaio.replace(codigoUsuario, ({ fn, arg, corpo }) => {
  // Desaçucara o corpo e aplica otimizações algébricas:
  const corpoOtimizado = "retorne $x * 2".papagaio.replace(corpo, ({ x }) => {
    return `(i32.shl (local.get $${x}) (i32.const 1))`; // shift left mais rápido que multiplicação
  });

  return `
(func $${fn} (param $${arg} i32) (result i32)
  ${corpoOtimizado}
)
(export "${fn}" (func $${fn}))`;
});

console.log(wat);
```

---

## 💻 Linha de Comando (CLI)

O Papagaio inclui um utilitário de CLI simples para processamento rápido de arquivos de texto ou templates:

```bash
# Processando template passando variáveis no contexto
node src/cli.js template.txt nome=Jardel versao=0.48.1
```

---

## 🧪 Testes

Execute a suíte de testes de regressão:

```bash
npm test
```

Para rodar os testes de compilação WebAssembly dos exemplos:

```bash
node examples/lisp2wat/run_demo.js
node examples/forth2wat/run_demo.js
```

---

## 📜 Licença

MIT © [jardimdanificado](https://github.com/jardimdanificado)
