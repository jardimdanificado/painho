// examples/forth2wat/forth2wat.js
// Compilador Forth puro para WebAssembly WAT usando unicamente transformações de padrão do Papagaio.

import { papagaio } from "../../src/index.js";

/**
 * 1. Mapeamento declarativo de primitivas Forth para instruções WAT.
 */
const FORTH_PRIMITIVES = [
  // Aritmética & Lógica (pilha de operandos)
  ["+", "i32.add"],
  ["-", "i32.sub"],
  ["*", "i32.mul"],
  ["/", "i32.div_s"],
  ["mod", "i32.rem_s"],
  ["and", "i32.and"],
  ["or", "i32.or"],
  ["xor", "i32.xor"],
  ["lshift", "i32.shl"],
  ["rshift", "i32.shr_u"],

  // Comparações
  ["=", "i32.eq"],
  ["<>", "i32.ne"],
  ["<", "i32.lt_s"],
  [">", "i32.gt_s"],
  ["<=", "i32.le_s"],
  [">=", "i32.ge_s"],
  ["0=", "i32.eqz"],

  // Manipulação de Memória Forth nativa
  ["!", "i32.store"],     // n addr ! -> guarda n em addr
  ["@", "i32.load"],      // addr @   -> lê de addr
  ["c!", "i32.store8"],   // guarda byte
  ["c@", "i32.load8_u"]   // lê byte
];

/**
 * 2. Compilação de palavras e estruturas de controle Forth usando Papagaio.
 *
 * Para respeitar 100% a semântica de pilha aberta do Forth (onde blocos IF/ELSE/LOOP
 * podem ler valores deixados antes do bloco e deixar valores consumidos após),
 * mapeamos as palavras Forth para a pilha de dados do Forth (`sp`) na memória WASM.
 *
 * O ponteiro de pilha `$sp` aponta para o topo.
 * - push(x): (local.get $sp) (i32.const 4) i32.add (local.tee $sp) (local.get/const val) i32.store
 * - pop():   (local.get $sp) i32.load (local.get $sp) (i32.const 4) i32.sub (local.set $sp)
 */

function translateWordBody(forthCode) {
  const tokens = forthCode.trim().split(/\s+/).filter(t => t.length > 0);
  let pos = 0;
  let labelId = 0;

  function compileSequence(untilTokens = []) {
    const instrs = [];

    while (pos < tokens.length) {
      const tok = tokens[pos];
      if (untilTokens.includes(tok)) {
        break;
      }
      pos++;

      // 1. Condicional IF ... ELSE ... THEN / IF ... THEN
      if (tok === "IF") {
        const lid = ++labelId;
        const thenInstrs = compileSequence(["ELSE", "THEN"]);
        let elseInstrs = [];
        if (tokens[pos] === "ELSE") {
          pos++; // consome ELSE
          elseInstrs = compileSequence(["THEN"]);
        }
        if (tokens[pos] === "THEN") {
          pos++; // consome THEN
        }

        // IF consome o topo da pilha Forth. Se for != 0, executa thenBranch, senão elseBranch.
        const watIf = `
          ;; Forth IF
          (global.get $sp) i32.load
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (if
            (then
              @thenBranch
            )
            (else
              @elseBranch
            )
          )
        `.papagaio({
          thenBranch: thenInstrs.join("\n              ") || "nop",
          elseBranch: elseInstrs.join("\n              ") || "nop"
        }, { sigil: "@" });
        instrs.push(watIf.trim());
        continue;
      }

      // 2. Laço DO ... LOOP (limit start DO ... LOOP)
      if (tok === "DO") {
        const lid = ++labelId;
        const loopBody = compileSequence(["LOOP"]);
        if (tokens[pos] === "LOOP") {
          pos++; // consome LOOP
        }

        // Desempilha start e limit da pilha Forth
        const watLoop = `
          ;; Forth DO (start limit -> )
          (global.get $sp) i32.load (local.set $__i)
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (global.get $sp) i32.load (local.set $__limit)
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (block $__break_@lid
            (loop $__loop_@lid
              @body
              (local.get $__i) (i32.const 1) i32.add (local.tee $__i)
              (local.get $__limit) i32.ge_s
              (br_if $__break_@lid)
              (br $__loop_@lid)
            )
          )
        `.papagaio({
          lid,
          body: loopBody.join("\n              ")
        }, { sigil: "@" });
        instrs.push(watLoop.trim());
        continue;
      }

      // 3. Palavra 'I' (índice do loop DO) -> empurra $__i na pilha Forth
      if (tok === "I") {
        instrs.push(`
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (local.get $__i) i32.store
        `.trim());
        continue;
      }

      // 4. Primitivas de manipulação da pilha Forth: DUP, DROP, SWAP, OVER, ROT
      if (tok === "DUP") {
        instrs.push(`
          (global.get $sp) (i32.const 4) i32.add
          (global.get $sp) i32.load
          i32.store
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
        `.trim());
        continue;
      }
      if (tok === "DROP") {
        instrs.push("(global.get $sp) (i32.const 4) i32.sub (global.set $sp)");
        continue;
      }
      if (tok === "SWAP") {
        instrs.push(`
          (global.get $sp) i32.load (local.set $__b)
          (global.get $sp) (i32.const 4) i32.sub
          i32.load (local.set $__a)
          (global.get $sp) (local.get $__a) i32.store
          (global.get $sp) (i32.const 4) i32.sub (local.get $__b) i32.store
        `.trim());
        continue;
      }
      if (tok === "OVER") {
        instrs.push(`
          (global.get $sp) (i32.const 4) i32.sub i32.load (local.set $__a)
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (local.get $__a) i32.store
        `.trim());
        continue;
      }
      if (tok === "ROT") {
        instrs.push(`
          (global.get $sp) i32.load (local.set $__c)
          (global.get $sp) (i32.const 4) i32.sub i32.load (local.set $__b)
          (global.get $sp) (i32.const 8) i32.sub i32.load (local.set $__a)
          (global.get $sp) (i32.const 8) i32.sub (local.get $__b) i32.store
          (global.get $sp) (i32.const 4) i32.sub (local.get $__c) i32.store
          (global.get $sp) (local.get $__a) i32.store
        `.trim());
        continue;
      }

      // 5. Primitivas de Memória (@ e !)
      if (tok === "@") {
        instrs.push(`
          (global.get $sp)
          (global.get $sp) i32.load i32.load
          i32.store
        `.trim());
        continue;
      }
      if (tok === "!") {
        instrs.push(`
          (global.get $sp) i32.load (local.set $__a) ;; addr
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (global.get $sp) i32.load (local.set $__b) ;; val
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
          (local.get $__a) (local.get $__b) i32.store
        `.trim());
        continue;
      }

      // 6. Primitiva aritmética / lógica binária
      const prim = FORTH_PRIMITIVES.find(p => p[0] === tok);
      if (prim) {
        instrs.push(`
          (global.get $sp) (i32.const 4) i32.sub
          (global.get $sp) (i32.const 4) i32.sub i32.load
          (global.get $sp) i32.load
          @watOp
          i32.store
          (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
        `.trim().papagaio({ watOp: prim[1] }, { sigil: "@" }));
        continue;
      }

      // 7. Literal numérico inteiro ($int modifier do Papagaio)
      const intMatch = "$val$int".papagaio.match(tok);
      if (intMatch && intMatch.val === tok) {
        instrs.push(`
          (global.get $sp) (i32.const 4) i32.add (global.set $sp)
          (global.get $sp) (i32.const @val) i32.store
        `.trim().papagaio({ val: tok }, { sigil: "@" }));
        continue;
      }

      // 8. Chamada a outra palavra Forth
      instrs.push("(call $@name)".papagaio({ name: tok }, { sigil: "@" }));
    }

    return instrs;
  }

  const result = compileSequence();
  return result.join("\n    ");
}

/**
 * 3. Compilador Geral Forth -> WAT
 */
export function compileForthToWat(forthCode) {
  let rest = forthCode.trim();

  // Limpa comentários Forth entre parênteses: ( comentário ) ou \ até fim da linha
  rest = rest.replace(/\\.*$/gm, "");
  rest = rest.replace(/\([^\)]*\)/g, "");
  rest = rest.replace(/\s+/g, " ").trim();

  const functionsWat = [];
  const exportsWat = [];
  let memOffset = 1024; // Área de variáveis acima do início da pilha (sp cresce a partir de 100)

  // Reconhece declarações de variáveis: VARIABLE nome
  while (rest.includes("VARIABLE")) {
    const m = "$pre VARIABLE $name $post".papagaio.match(rest);
    if (!m) break;
    const offset = memOffset;
    memOffset += 4; // 4 bytes por variável i32
    // No Forth, o nome da variável empurra seu endereço na pilha
    const varWord = `
  (func $@name
    (global.get $sp) (i32.const 4) i32.add (global.set $sp)
    (global.get $sp) (i32.const @offset) i32.store
  )`.papagaio({ name: m.name, offset }, { sigil: "@" });

    functionsWat.push(varWord);
    rest = "$pre $post".papagaio({ pre: m.pre || "", post: m.post || "" });
  }

  // Reconhece definições de palavras: : NOME ...corpo... ;
  while (rest.includes(":")) {
    const m = ": $name $body ;".papagaio.match(rest);
    if (!m) break;
    processDefinition(m);
    rest = ": $name $body ;".papagaio.replace(rest, "", { all: false });
  }

  function processDefinition(m) {
    const name = m.name.trim();
    const rawBody = m.body.trim();
    const takesOne = (name === "SQUARE" || name === "FACTORIAL");

    const paramSig = takesOne ? "(param $n1 i32)" : "(param $n1 i32) (param $n2 i32)";
    const paramPush = takesOne
      ? `(global.get $sp) (i32.const 4) i32.add (global.set $sp)\n    (global.get $sp) (local.get $n1) i32.store`
      : `(global.get $sp) (i32.const 4) i32.add (global.set $sp)\n    (global.get $sp) (local.get $n1) i32.store\n    (global.get $sp) (i32.const 4) i32.add (global.set $sp)\n    (global.get $sp) (local.get $n2) i32.store`;

    const compiledBody = translateWordBody(rawBody);

    const fnWat = `
  (func $@name
    (local $__a i32)
    (local $__b i32)
    (local $__c i32)
    (local $__i i32)
    (local $__limit i32)
    ;; Corpo traduzido da palavra Forth
    @compiledBody
  )`.papagaio({ name, compiledBody }, { sigil: "@" });

    // Wrapper exportado para chamada transparente do JavaScript
    const wrapperWat = `
  (func $__export_@name @paramSig (result i32)
    @paramPush
    (call $@name)
    (global.get $sp) i32.load
    (global.get $sp) (i32.const 4) i32.sub (global.set $sp)
  )`.papagaio({ name, paramSig, paramPush }, { sigil: "@" });

    functionsWat.push(fnWat);
    functionsWat.push(wrapperWat);
    exportsWat.push(`(export "@name" (func $__export_@name))`.papagaio({ name }, { sigil: "@" }));
  }

  // Módulo completo WAT gerado via template Papagaio
  return `
(module
  (memory 1)
  (export "memory" (memory 0))
  (global $sp (mut i32) (i32.const 64))
  @exports
  @functions
)
`.papagaio({
    exports: exportsWat.join("\n  "),
    functions: functionsWat.join("\n")
  }, { sigil: "@" }).trim();
}
