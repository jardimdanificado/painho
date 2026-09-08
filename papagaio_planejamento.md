# Papagaio — Planejamento de Reimaginação

## 1. Visão

Reimaginar o Papagaio como uma **biblioteca JavaScript de processamento textual**, e não como uma linguagem independente.

A linguagem JavaScript passa a ser responsável por:

- expressões;
- operadores;
- controle de fluxo;
- arrays;
- objetos;
- matemática;
- funções;
- módulos;
- estruturas de dados;
- APIs do runtime.

O Papagaio fica responsável apenas pelas funcionalidades que justificam sua existência:

- interpolação textual;
- templates;
- pattern matching;
- captures;
- substituição baseada em padrões;
- modifiers/padrões próprios;
- configuração de delimitadores;
- demais mecanismos genuinamente específicos do Papagaio.

A integração com `String` deve ser mínima:

```js
"Olá $nome".papagaio({
    nome: "Jardel"
});
```

A propriedade `String.prototype.papagaio` será uma **função-objeto**, funcionando simultaneamente como:

1. operação principal de processamento de strings;
2. namespace para toda a API especializada do Papagaio.

---

# 2. Objetivos

## 2.1 Objetivos principais

- Reutilizar JavaScript em vez de recriar funcionalidades da linguagem.
- Ter uma API pequena e coerente.
- Manter as funcionalidades únicas do Papagaio.
- Ser utilizável diretamente em JavaScript moderno.
- Ter **ES2023 como teto de compatibilidade**.
- Ter **QuickJS como runtime mínimo obrigatório**.
- Funcionar sem depender de Node.js.
- Permitir execução em browser, QuickJS e outros runtimes compatíveis.
- Manter o núcleo pequeno e portátil.
- Facilitar futuramente a integração com WASM e outros hosts.

## 2.2 Não objetivos

Papagaio não deve se transformar em:

- uma nova linguagem de programação;
- uma DSL de controle de fluxo;
- um substituto para JavaScript;
- um sistema próprio de arrays;
- um sistema próprio de matemática;
- um sistema próprio de objetos;
- um sistema próprio de módulos;
- um template engine cheio de blocos;
- uma segunda sintaxe para recursos já existentes em JavaScript.

---

# 3. Compatibilidade

| Camada | Requisito |
|---|---|
| Teto da linguagem | ES2023 |
| Runtime mínimo | QuickJS |
| Runtime primário | JavaScript ES2023 |
| Node.js | Suportado |
| Browser | Suportado |
| Bun | Desejável |
| Deno | Desejável |
| MicroQuickJS | Não obrigatório inicialmente |
| APIs específicas de Node | Não permitidas no core |
| Dependências externas | Minimizar |

A implementação pode utilizar recursos de ES2023.

Entretanto, a API pública deve depender apenas de recursos disponíveis no ambiente alvo.

O core não deve assumir a existência de:

```js
process
require
fs
Buffer
node:*
```

ou outras APIs específicas de Node.

---

# 4. Princípio fundamental da API

O Papagaio deve adicionar **uma única propriedade ao `String.prototype`**:

```js
String.prototype.papagaio
```

Essa propriedade será uma função.

Exemplo:

```js
"Olá $nome".papagaio({
    nome: "Jardel"
});
```

A mesma função será um objeto e conterá métodos especializados:

```js
"rgb($r,$g,$b)".papagaio.match(
    "rgb(255,128,64)"
);
```

Conceitualmente:

```text
String
 └── papagaio()
      ├── match()
      ├── replace()
      ├── compile()
      └── ...
```

Não criar uma família de métodos como:

```js
.papagaioMatch()
.papagaioReplace()
.papagaioCompile()
```

---

# 5. Modelo de implementação

A função principal pode ser estruturada conceitualmente como:

```js
function papagaio(template, context) {
    // processamento principal
}

papagaio.match = function (pattern, input, options) {
    // matching
};

papagaio.replace = function (pattern, input, context, options) {
    // replacement
};

papagaio.compile = function (pattern, options) {
    // compilação
};

String.prototype.papagaio = function (context) {
    return papagaio(this, context);
};
```

A implementação real pode ser diferente, mas deve preservar essa semântica pública.

---

# 6. API global

A função também deve poder ser utilizada diretamente:

```js
papagaio("Olá $nome", {
    nome: "Jardel"
});
```

Isso deve ser semanticamente equivalente à forma:

```js
"Olá $nome".papagaio({
    nome: "Jardel"
});
```

A API global é importante para:

- evitar dependência obrigatória de monkey patch;
- uso em código que prefere funções;
- integração com módulos;
- facilitar testes;
- permitir hosts onde modificar `String.prototype` não seja desejável.

---

# 7. API principal

## 7.1 `papagaio()`

Responsável pelo processamento/interpolação.

```js
"Olá $nome".papagaio({
    nome: "Jardel"
});
```

Resultado:

```text
Olá Jardel
```

Forma funcional:

```js
papagaio("Olá $nome", {
    nome: "Jardel"
});
```

---

# 8. Expressões JavaScript

A nova versão deve preferir JavaScript para expressões.

Exemplo:

```js
"Olá ${nome.toUpperCase()}".papagaio({
    nome: "jardel"
});
```

Resultado:

```text
Olá JARDEL
```

Arrays:

```js
"Items: ${items.join(', ')}".papagaio({
    items: ["A", "B", "C"]
});
```

Matemática:

```js
"Total: ${price * quantity}".papagaio({
    price: 10,
    quantity: 5
});
```

Condicional:

```js
`${age >= 18 ? "Adulto" : "Menor"}`.papagaio({
    age: 20
});
```

A semântica exata de `$name` versus `${expression}` deverá ser definida antes da implementação do parser.

---

# 9. Pattern Matching

Pattern matching é uma das funcionalidades centrais que devem permanecer.

Exemplo:

```js
"Hello $name".papagaio.match("Hello Jardel");
```

Resultado conceitual:

```js
{
    name: "Jardel"
}
```

Outro exemplo:

```js
"rgb($r,$g,$b)".papagaio.match(
    "rgb(255,128,64)"
);
```

Resultado:

```js
{
    r: "255",
    g: "128",
    b: "64"
}
```

A semântica de matching deve ser baseada no comportamento real do Papagaio atual, após levantamento do código-fonte.

---

# 10. Captures

Captures devem ser preservados como conceito nativo do Papagaio.

Exemplo conceitual:

```js
"$a $b".papagaio.match("hello world");
```

Resultado:

```js
{
    a: "hello",
    b: "world"
}
```

O novo projeto deve documentar:

- como captures são declarados;
- como são nomeados;
- quando são opcionais;
- comportamento de captures repetidos;
- comportamento de captures vazios;
- captures aninhados;
- tipos dos valores retornados;
- conflitos entre nomes;
- ordem de resolução.

Não alterar a semântica existente sem justificativa.

---

# 11. Modifiers

Todos os modifiers existentes devem ser catalogados antes da implementação.

Para cada modifier, documentar:

| Campo | Descrição |
|---|---|
| Nome | Identificador atual |
| Sintaxe | Sintaxe atual |
| Entrada | O que aceita |
| Saída | O que produz |
| Matching | Como influencia o match |
| Replacement | Se influencia replacement |
| Equivalente JS | Se existir |
| Nova API | Como será exposto |

A regra é:

> Se o JavaScript já fornece a funcionalidade de maneira adequada, não recriar a funcionalidade no Papagaio.

Mas modifiers que fazem parte da semântica específica de pattern matching devem permanecer.

---

# 12. Replacement

Replacement deve continuar existindo porque é uma extensão natural do sistema de padrões.

API conceitual:

```js
papagaio.replace(
    pattern,
    input,
    context
);
```

E, através do namespace da String:

```js
pattern.papagaio.replace(
    input,
    context
);
```

A API deve permitir utilizar captures e expressões JavaScript quando isso for apropriado.

É necessário definir:

- replacement simples;
- replacement com captures;
- replacement com função;
- replacement global;
- replacement condicional;
- comportamento de padrões que não encontram correspondência;
- escaping.

---

# 13. `compile()`

Uma API de compilação pode ser fornecida para evitar recompilar repetidamente o mesmo padrão/template.

Conceito:

```js
const template = "Olá $nome".papagaio.compile();

template({
    nome: "Jardel"
});

template({
    nome: "Maria"
});
```

A API final deve decidir se `compile()` retorna:

- uma função;
- um objeto callable;
- ou outra representação.

A preferência deve ser pela solução mais simples compatível com JavaScript.

---

# 14. Arrays

Não criar `$list`.

JavaScript já possui:

```js
push()
pop()
shift()
unshift()
splice()
slice()
reverse()
join()
indexOf()
includes()
map()
filter()
find()
some()
every()
```

Exemplo:

```js
"${items.reverse().join(', ')}".papagaio({
    items
});
```

O Papagaio não deve fornecer wrappers para essas operações.

---

# 15. Matemática

Não criar `$math`.

Usar:

```js
Math.*
```

e os operadores JavaScript:

```js
+
-
*
/
%
**
<
>
<=
>=
===
!==
```

Exemplo:

```js
"${Math.sqrt(x)}".papagaio({ x: 25 });
```

---

# 16. Comparações

Não criar `$compare`.

Usar JavaScript:

```js
a === b
a !== b
a < b
a > b
a <= b
a >= b
```

---

# 17. Operações de String

Não recriar operações já presentes em JavaScript.

Usar:

```js
length
slice()
substring()
indexOf()
lastIndexOf()
includes()
startsWith()
endsWith()
replace()
replaceAll()
split()
trim()
toUpperCase()
toLowerCase()
```

O Papagaio deve adicionar somente operações que não sejam adequadamente cobertas por essas APIs.

---

# 18. Flow Control

Remover do Papagaio:

```text
$then
$else
$repeat
$while
$until
```

JavaScript já fornece:

```js
if
else
for
while
do
for...of
for...in
map()
filter()
reduce()
```

Exemplo:

```js
const result = condition
    ? "A"
    : "B";
```

O Papagaio não deve implementar uma segunda forma de controle de fluxo.

---

# 19. `$contains`

Remover.

Usar:

```js
string.includes(value)
array.includes(value)
```

---

# 20. `$slice`

Remover.

Usar:

```js
string.slice(...)
array.slice(...)
```

---

# 21. `$replace`

A operação genérica deve ser feita com JavaScript:

```js
string.replace(...)
string.replaceAll(...)
```

Porém, replacement baseado no sistema de patterns do Papagaio continua sendo responsabilidade do Papagaio.

Essa distinção precisa ser mantida:

```text
JavaScript String.replace()
        ↓
replacement convencional

Papagaio pattern replacement
        ↓
pattern matching + captures + semântica Papagaio
```

---

# 22. `$args`

Remover do core.

Argumentos devem ser fornecidos pelo host.

Node:

```js
process.argv
```

QuickJS ou outro host:

```js
const args = ...;
```

O Papagaio apenas recebe os dados.

---

# 23. `$include`

Não deve ser uma funcionalidade obrigatória do core.

O carregamento de arquivos/módulos deve ser responsabilidade do host ou do sistema de módulos JavaScript.

O core não deve depender de:

```js
fs
```

ou APIs específicas de Node.

Se o comportamento original de include for importante, ele poderá ser reintroduzido como uma camada opcional.

---

# 24. `$document`

O conceito deve ser reavaliado.

Primeiro levantar:

- qual problema `$document` resolve atualmente;
- quais partes são específicas do Papagaio;
- se um objeto JavaScript normal resolve o mesmo problema;
- se a funcionalidade é necessária no core.

Só manter como abstração própria se houver uma justificativa concreta.

---

# 25. Scopes

Não criar um sistema próprio de scopes.

JavaScript já possui:

```js
let
const
var
function
block scope
closures
modules
```

O contexto passado ao Papagaio deve ser representado preferencialmente por objetos JavaScript normais.

---

# 26. Configuração

Funcionalidades configuráveis do Papagaio atual que continuarem relevantes devem ser representadas como opções JavaScript.

Exemplo:

```js
template.papagaio({
    context,
    sigil: "$",
    open: "${",
    close: "}"
});
```

A estrutura exata deve ser definida após catalogar todas as opções atuais.

Configurações devem ser:

- explícitas;
- opcionais;
- documentadas;
- independentes do runtime.

---

# 27. Extensibilidade

A extensibilidade deve privilegiar JavaScript.

Exemplo conceitual:

```js
const context = {
    upper: value => value.toUpperCase()
};

"Nome: ${upper(name)}".papagaio(context);
```

Não criar uma linguagem de plugins baseada em comandos Papagaio.

Se futuramente houver:

- plugins nativos;
- plugins WASM;
- bindings C;
- extensões específicas de host;

eles devem ficar abaixo da API JavaScript principal.

---

# 28. Monkey patch opcional

O pacote deve considerar permitir dois modos de uso.

## Modo integrado

```js
import "papagaio";

"Olá $nome".papagaio({
    nome: "Jardel"
});
```

## Modo sem monkey patch

```js
import { papagaio } from "papagaio";

papagaio("Olá $nome", {
    nome: "Jardel"
});
```

Isso é especialmente útil em ambientes onde modificar prototypes globais não seja desejável.

---

# 29. Organização interna

Uma possível estrutura:

```text
papagaio/
├── src/
│   ├── papagaio.js
│   ├── interpolate.js
│   ├── parser.js
│   ├── matcher.js
│   ├── captures.js
│   ├── modifiers.js
│   ├── replacement.js
│   ├── compiler.js
│   ├── options.js
│   └── index.js
│
├── test/
│   ├── interpolation/
│   ├── matching/
│   ├── captures/
│   ├── modifiers/
│   ├── replacement/
│   ├── compatibility/
│   └── api/
│
├── examples/
├── README.md
├── package.json
└── LICENSE
```

A estrutura definitiva deve ser escolhida pelo agente conforme o código existente.

---

# 30. Arquitetura conceitual

```text
                    JavaScript / QuickJS
                            │
                            ▼
                String.prototype.papagaio
                            │
                            ▼
                     papagaio()
                            │
              ┌─────────────┼─────────────┐
              │             │             │
              ▼             ▼             ▼
        Interpolator      Matcher      Replacement
              │             │             │
              └─────────────┼─────────────┘
                            ▼
                    Papagaio Engine
                            │
                            ▼
                    JavaScript values
```

O engine deve ser independente da API específica de Node.

---

# 31. Portabilidade

O core deve funcionar em:

```text
┌───────────┐
│   Node    │
└─────┬─────┘
      │
┌─────▼─────┐
│  Browser  │
└─────┬─────┘
      │
┌─────▼─────┐
│  QuickJS  │
└─────┬─────┘
      │
┌─────▼─────┐
│   Bun     │
└─────┬─────┘
      │
┌─────▼─────┐
│   Deno    │
└───────────┘
```

QuickJS deve ser tratado como o ambiente de menor capacidade suportado oficialmente.

---

# 32. WASM

O design deve evitar bloquear uma futura implementação/execução em WASM.

Possibilidades futuras:

```text
JavaScript
    │
    ├── QuickJS
    ├── Browser
    ├── Node
    │
    └── WASM host
```

Se o engine eventualmente for portado para C/WASM, a API JavaScript poderá permanecer igual.

Entretanto, isso não deve complicar a primeira implementação.

---

# 33. Testes

O projeto deve possuir testes para:

### API

```js
"foo".papagaio(...)
papagaio("foo", ...)
```

### Interpolação

- strings simples;
- variáveis;
- expressões;
- arrays;
- objetos;
- valores nulos;
- booleanos;
- números;
- strings vazias;
- escaping.

### Matching

- literals;
- captures;
- múltiplos captures;
- captures opcionais;
- captures repetidos;
- padrões aninhados;
- modifiers;
- falhas de matching.

### Replacement

- replacement simples;
- captures;
- múltiplas ocorrências;
- ausência de match;
- funções;
- escaping.

### Configuração

- sigil;
- delimitadores;
- opções;
- defaults.

### Runtime

O mesmo conjunto de testes deve ser executável em:

- Node;
- QuickJS;
- browser, quando aplicável.

---

# 34. Compatibilidade com o Papagaio atual

Este é um requisito crítico.

Antes de remover ou alterar qualquer feature, o agente deve:

1. examinar o código atual;
2. catalogar todas as funcionalidades;
3. localizar todos os exemplos;
4. localizar todos os testes;
5. identificar a semântica real;
6. separar funcionalidades essenciais de funcionalidades substituíveis por JavaScript;
7. criar uma tabela de migração.

Formato:

| Papagaio atual | Novo Papagaio | Ação |
|---|---|---|
| Feature A | API JS | Remover |
| Feature B | `papagaio.*` | Preservar |
| Feature C | String API | Remover |
| Feature D | Nova API | Adaptar |

Nenhuma feature deve ser removida apenas por suposição.

---

# 35. Critério para uma feature permanecer no Papagaio

Uma funcionalidade deve permanecer no core somente se satisfizer pelo menos uma destas condições:

1. É parte fundamental do pattern matching do Papagaio.
2. Não possui equivalente adequado em JavaScript.
3. É necessária para preservar compatibilidade.
4. É significativamente mais conveniente como primitiva Papagaio.
5. É necessária para templates/interpolação.

Caso contrário, preferir JavaScript.

---

# 36. Princípio de simplicidade

O agente deve evitar criar abstrações sem necessidade.

Antes de implementar qualquer API nova, perguntar:

> "JavaScript ES2023 já resolve isso?"

Se sim, usar JavaScript.

Exemplo:

```js
// Não
papagaio.list.push(items, value);

// Sim
items.push(value);
```

```js
// Não
papagaio.math.add(a, b);

// Sim
a + b;
```

```js
// Não
papagaio.contains(text, value);

// Sim
text.includes(value);
```

```js
// Não
papagaio.if(condition, ...);

// Sim
condition ? a : b;
```

---

# 37. API alvo preliminar

A API pública deverá convergir para algo próximo de:

```js
// operação principal
papagaio(template, context);

// integração com String
String.prototype.papagaio;

// matching
papagaio.match(pattern, input, options);

// replacement
papagaio.replace(pattern, input, context, options);

// compilação
papagaio.compile(pattern, options);
```

E:

```js
template.papagaio(context);
template.papagaio.match(input, options);
template.papagaio.replace(input, context, options);
template.papagaio.compile(options);
```

Os métodos exatos ainda devem ser validados contra a semântica do Papagaio atual.

---

# 38. Exemplo de uso final

```js
import "papagaio";

const user = {
    name: "Jardel",
    age: 25
};

const items = ["C", "Lua", "JavaScript"];

const result = `
Olá ${user.name}.

Idade: ${user.age}
Linguagens: ${items.join(", ")}
Status: ${user.age >= 18 ? "adulto" : "menor"}
`.papagaio();

console.log(result);
```

Pattern matching:

```js
const rgb = "rgb($r,$g,$b)"
    .papagaio
    .match("rgb(255,128,64)");

console.log(rgb);
// { r: "255", g: "128", b: "64" }
```

---

# 39. Ordem de implementação

## Fase 1 — Auditoria

- Ler o código atual do Papagaio.
- Catalogar todas as features.
- Catalogar sintaxe.
- Catalogar modifiers.
- Catalogar exemplos.
- Catalogar testes.
- Catalogar comportamento de edge cases.

**Nenhuma reimplementação antes dessa etapa.**

## Fase 2 — Especificação

Definir:

- sintaxe de interpolação;
- `$x` versus `${expression}`;
- escaping;
- delimitadores;
- captures;
- modifiers;
- matching;
- replacement;
- opções;
- erros;
- tipos retornados.

## Fase 3 — API mínima

Implementar:

```js
papagaio()
String.prototype.papagaio
```

## Fase 4 — Engine de interpolação

Implementar:

- parsing;
- resolução de contexto;
- expressões;
- escaping;
- configuração.

## Fase 5 — Matcher

Portar:

- patterns;
- captures;
- modifiers;
- nested patterns;
- edge cases.

## Fase 6 — Replacement

Implementar replacement baseado no matcher.

## Fase 7 — Compilation

Adicionar `compile()` somente se houver benefício real.

## Fase 8 — Compatibilidade

Executar testes em:

- QuickJS;
- Node;
- browser.

## Fase 9 — Documentação

Criar:

- README;
- API reference;
- migration guide;
- exemplos;
- documentação de patterns;
- documentação de modifiers.

## Fase 10 — Limpeza

Remover:

- DSL desnecessária;
- funcionalidades substituídas por JS;
- APIs redundantes;
- dependências de Node;
- abstrações não utilizadas.

---

# 40. Critérios de aceitação

O projeto será considerado concluído quando:

- [ ] `String.prototype.papagaio` funcionar.
- [ ] `papagaio()` funcionar sem monkey patch.
- [ ] A função `papagaio` possuir namespace para funcionalidades especializadas.
- [ ] Interpolação funcionar.
- [ ] Pattern matching funcionar.
- [ ] Captures funcionarem.
- [ ] Modifiers preservados forem implementados.
- [ ] Replacement funcionar.
- [ ] Configuração de delimitadores funcionar quando necessária.
- [ ] Não houver DSL para funcionalidades já existentes em JavaScript.
- [ ] `$list` tiver sido substituído por Arrays JavaScript.
- [ ] `$math` tiver sido substituído por `Math`/operadores.
- [ ] `$compare` tiver sido substituído por operadores JS.
- [ ] `$contains` tiver sido substituído por APIs JS.
- [ ] `$slice` tiver sido substituído por `.slice()`.
- [ ] Flow control próprio tiver sido removido.
- [ ] O core não depender de Node.
- [ ] QuickJS for suportado.
- [ ] ES2023 for o teto da API.
- [ ] Testes de regressão cobrirem as features preservadas.
- [ ] Documentação explicar claramente o que pertence ao JavaScript e o que pertence ao Papagaio.

---

# 41. Filosofia final

O novo Papagaio deve seguir uma regra simples:

> **JavaScript é a linguagem. Papagaio é a extensão textual.**

Não devemos transformar Papagaio em uma linguagem menor dentro de JavaScript.

A experiência desejada é:

```js
"Olá ${name.toUpperCase()}".papagaio({
    name: "Jardel"
});
```

e, quando JavaScript não possui uma solução equivalente:

```js
"rgb($r,$g,$b)".papagaio.match(
    "rgb(255,128,64)"
);
```

O Papagaio deve ficar pequeno justamente porque **não tenta substituir JavaScript**.

A API deve ser organizada ao redor de uma única porta de entrada:

```js
String.prototype.papagaio
```

Essa propriedade é uma função e, ao mesmo tempo, o namespace:

```text
papagaio()
├── match()
├── replace()
├── compile()
└── futuras operações genuinamente Papagaio
```

Qualquer nova funcionalidade deve passar pelo teste:

> **"Isso é realmente Papagaio ou JavaScript já deveria fazer isso?"**

Se for JavaScript, usar JavaScript.
Se for Papagaio, colocar no namespace `papagaio`.
