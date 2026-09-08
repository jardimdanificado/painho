# Papagaio — Especificação Técnica da Reimaginação (Fase 2)

Este documento estabelece as especificações formais de sintaxe, tipos, APIs e comportamento da nova biblioteca JavaScript **Papagaio** conforme delineado no [papagaio_planejamento.md](../papagaio_planejamento.md).

---

## 1. Princípios de Arquitetura & Runtimes Alvo

- **Linguagem**: JavaScript moderno com teto estrito em **ES2023**.
- **Runtime Mínimo**: **QuickJS** (também roda nativamente em Node.js, Bun, Deno, Browsers).
- **Sem Dependências**: Zero dependências de pacotes npm externos.
- **Isolamento de Host**: O core não importa `node:fs`, `process` nem assume DOM de browser.

---

## 2. API Pública

### 2.1 Ponto de Entrada Principal

O Papagaio exporta uma função principal `papagaio` e estende `String.prototype.papagaio`:

```js
import { papagaio } from "papagaio";
// ou
import "papagaio"; // instala String.prototype.papagaio
```

A propriedade `String.prototype.papagaio` e a função `papagaio` compartilham a mesma assinatura conceitual e funcionam como **função-objeto**:

```js
// Chamada direta (funcional)
papagaio(template, context, options);

// Via String.prototype
template.papagaio(context, options);
```

### 2.2 Namespace de Métodos Especializados

Tanto a função global quanto o método no prototype contam com os mesmos métodos:

#### 1. `match(pattern, input, options)` / `pattern.papagaio.match(input, options)`
- Executa pattern matching com flex-matching, modifiers e captures.
- **Retorno**: Um objeto com as variáveis capturadas `{ [varName]: string }`, ou `null` caso o padrão não case.

#### 2. `replace(pattern, input, replacement, options)` / `pattern.papagaio.replace(input, replacement, options)`
- Executa matching e substitui as ocorrências do padrão em `input`.
- `replacement` pode ser:
  - Uma `string` contendo `$var` ou `${var}` correspondentes às capturas do padrão;
  - Uma `function(captures, matchInfo): string` recebendo o objeto de capturas.
- `options.all`: booleano (default `true`), se deve substituir todas as ocorrências ou apenas a primeira.

#### 3. `compile(patternOrTemplate, options)` / `pattern.papagaio.compile(options)`
- Pré-analisa os tokens do padrão ou template, retornando uma função reutilizável de alta performance:
  - Se for template: `compiled(context) => string`
  - Se for padrão: `compiled.match(input) => captures | null` e `compiled.replace(input, replacement) => string`

---

## 3. Sintaxe e Semântica de Interpolação

### 3.1 `$identificador` vs `${expressao}`

1. **Variável Simples (`$identificador`)**:
   - Casa `[a-zA-Z_][a-zA-Z0-9_]*`.
   - Busca a chave diretamente no objeto de contexto: `context[identificador]`.
   - Se a chave não existir no contexto:
     - Comportamento padrão: emite string vazia `""` (ou pode ser configurado nas opções).
2. **Expressão JavaScript (`${expressão}`)**:
   - Avalia a expressão JavaScript fornecida dentro do escopo das propriedades do contexto.
   - Suporta chamadas de métodos, operadores ternários, matemática (`Math.*`), manipulação de arrays e strings.
   - Exemplo:
     ```js
     "Total: ${price * (1 + tax)} - Itens: ${items.join(', ')}".papagaio({
       price: 100,
       tax: 0.1,
       items: ["A", "B"]
     });
     ```
   - No QuickJS / ambientes compatíveis com ES2023, a avaliação utiliza `new Function(...)` com as chaves do contexto passadas como argumentos (ou escopo `with`/desestruturação segura).

### 3.2 Escaping

- `\$` emite um literal `$` sem acionar interpolação ou variável.
- `\\` emite `\`.
- Opções `{ sigil, open, close }` permitem alterar os delimitadores se desejado (default: `$`, `{`, `}`).

---

## 4. Semântica do Pattern Matching & Modifiers

### 4.1 Tokenização de Padrões
- **Flex-matching**: Espaços em branco no padrão casam 1 ou mais espaços no texto de entrada. Espaços horizontais entre variáveis e literais são automaticamente acomodados.
- **Trailing sigil (`$`)**: Em `$a$ $b`, o sigil após `$a` consome todo espaço subsequente.
- **Opcionalidade (`?`)**: `token?` marca o token como opcional. Se não encontrar correspondência, o capture do token fica vazio (`""`).

### 4.2 Modifiers Preservados
1. **Tipos e Formatos**:
   - `int`: dígitos inteiros (permite `-` no início).
   - `float` / `number`: representação numérica decimal.
   - `upper`: apenas caracteres alfabéticos em maiúsculo.
   - `lower`: apenas caracteres alfabéticos em minúsculo.
   - `capitalized`: primeira letra maiúscula e subsequentes minúsculas.
   - `word`: caracteres alfabéticos (`/^[a-zA-Z]+$/`).
   - `identifier`: identificador de código (`/^[a-zA-Z_]\w*$/`).
   - `hex`: dígitos hexadecimais (com ou sem prefixo `0x`).
   - `path`: token sem espaços em branco.
   - `binary`: dígitos binários `0`/`1` e prefixo `0b`.
   - `percent`: número acompanhado do símbolo `%`.
   - `alpha`: valida caracteres alfabéticos e converte o resultado capturado para maiúsculo (compatibilidade com built-in histórico).
   - `alphanum`: valida alfanuméricos.
2. **Estruturais**:
   - `block{open}{close}`: captura recursiva respeitando contagem de balanceamento dos delimitadores `open` e `close`.
   - `aliases{alt1}{alt2}...`: tenta casar sequencialmente cada uma das alternativas (literais ou sub-padrões).
   - `group{subpattern}`: agrupa um sub-padrão como unidade para match e opcionalidade.
   - `starts{p}` / `prefix{p}`: exige que o valor capturado inicie com `p`.
   - `ends{s}` / `suffix{s}`: exige que o valor capturado termine com `s`.
   - `infix{x}`: exige que o valor contenha `x` estritamente no seu interior.
   - `includes{x}`: exige que o valor contenha a substring `x`.

### 4.3 Captures Duplicados & Ordem
- Quando um mesmo nome de variável aparece mais de uma vez no padrão, a última ocorrência bem-sucedida define o valor ou, se especificado em `options.arrayCaptures: true`, acumula um array de strings.
