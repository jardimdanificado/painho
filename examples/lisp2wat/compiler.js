// examples/lisp2wat/compiler.js
// Compilador de Lisp/S-expression de alto nível para WebAssembly Text (WAT)
// usando exclusivamente Papagaio como motor de matching, desestruturação textual e templates.

import { papagaio } from "../../src/index.js";

/**
 * Tabela de instruções unárias e binárias do WASM 1.0 suportadas
 */
const WASM_OPS = {
  // i32
  "+": "i32.add",
  "-": "i32.sub",
  "*": "i32.mul",
  "/": "i32.div_s",
  "/u": "i32.div_u",
  "%": "i32.rem_s",
  "%u": "i32.rem_u",
  "&": "i32.and",
  "|": "i32.or",
  "^": "i32.xor",
  "<<": "i32.shl",
  ">>": "i32.shr_s",
  ">>u": "i32.shr_u",
  "==": "i32.eq",
  "!=": "i32.ne",
  "<": "i32.lt_s",
  "<u": "i32.lt_u",
  "<=": "i32.le_s",
  "<=u": "i32.le_u",
  ">": "i32.gt_s",
  ">u": "i32.gt_u",
  ">=": "i32.ge_s",
  ">=u": "i32.ge_u",

  // i64
  "i64.+": "i64.add",
  "i64.-": "i64.sub",
  "i64.*": "i64.mul",
  "i64./": "i64.div_s",
  "i64.==": "i64.eq",

  // f32
  "f32.+": "f32.add",
  "f32.-": "f32.sub",
  "f32.*": "f32.mul",
  "f32./": "f32.div",
  "f32.sqrt": "f32.sqrt",

  // f64
  "f64.+": "f64.add",
  "f64.-": "f64.sub",
  "f64.*": "f64.mul",
  "f64./": "f64.div",
  "f64.sqrt": "f64.sqrt",

  // Conversões & bitwise unários
  "i32.eqz": "i32.eqz",
  "i64.eqz": "i64.eqz",
  "i32.clz": "i32.clz",
  "i32.ctz": "i32.ctz",
  "i32.popcnt": "i32.popcnt"
};

/**
 * Divide uma lista de S-expressions e tokens no nível superior respeitando parênteses balanceados,
 * strings entre aspas e ignorando comentários Lisp (;;).
 */
export function splitTopLevelSExprs(src) {
  const exprs = [];
  let i = 0;
  // Remove comentários de linha ;; primeiro
  const cleanSrc = src.replace(/;;[^\n]*/g, "");
  const len = cleanSrc.length;

  while (i < len) {
    while (i < len && /\s/.test(cleanSrc[i])) i++;
    if (i >= len) break;

    // 1. S-Expression em parênteses: ( ... )
    if (cleanSrc[i] === "(") {
      let depth = 1;
      let start = i;
      i++;
      while (i < len && depth > 0) {
        if (cleanSrc[i] === "\"") {
          i++;
          while (i < len && cleanSrc[i] !== "\"") {
            if (cleanSrc[i] === "\\") i++;
            i++;
          }
          if (i < len) i++;
          continue;
        }
        if (cleanSrc[i] === "(") depth++;
        else if (cleanSrc[i] === ")") depth--;
        i++;
      }
      exprs.push(cleanSrc.slice(start, i));
      continue;
    }

    // 2. String literal: "..."
    if (cleanSrc[i] === "\"") {
      let start = i;
      i++;
      while (i < len && cleanSrc[i] !== "\"") {
        if (cleanSrc[i] === "\\") i++;
        i++;
      }
      if (i < len) i++;
      exprs.push(cleanSrc.slice(start, i));
      continue;
    }

    // 3. Identificador / Átomo / Operador
    let start = i;
    while (i < len && !/\s/.test(cleanSrc[i]) && cleanSrc[i] !== "(" && cleanSrc[i] !== ")") {
      i++;
    }
    exprs.push(cleanSrc.slice(start, i));
  }

  return exprs;
}

/**
 * Compilador recursivo de expressões Lisp para instruções WAT
 */
export function compileExpr(expr, scope = { locals: new Set() }) {
  const code = expr.trim();
  if (!code) return "";

  // 1. Número Inteiro (i32.const) usando modifier $int
  const intMatch = "$val$int".papagaio.match(code);
  if (intMatch && intMatch.val === code) {
    return "(i32.const $val)".papagaio({ val: intMatch.val });
  }

  // 2. Número Ponto Flutuante (f32.const) usando modifier $float
  const floatMatch = "$val$float".papagaio.match(code);
  if (floatMatch && floatMatch.val === code) {
    return "(f32.const $val)".papagaio({ val: floatMatch.val });
  }

  // 3. Bloco S-Expression: (op ...args)
  const blockMatch = "$inner$block{(}{)}".papagaio.match(code);
  if (blockMatch) {
    const inner = blockMatch.inner.trim();
    const parts = splitTopLevelSExprs(inner);
    if (parts.length === 0) return "";

    const op = parts[0];
    const args = parts.slice(1);

    // Bloco sequencial de comandos: (do ...exprs) ou (then ...exprs)
    if (op === "do" || op === "then") {
      return args.map(a => compileExpr(a, scope)).join("\n");
    }

    // Estrutura de Controle: IF / THEN / ELSE
    if (op === "if") {
      const condCode = compileExpr(args[0], scope);
      const thenCode = compileExpr(args[1], scope);
      const elseCode = args[2] ? compileExpr(args[2], scope) : "";

      // Se não tiver else, é if de instrução/efeito (void)
      if (!elseCode) {
        return `
        $condCode
        (if
          (then
            $thenCode
          )
        )
        `.papagaio({ condCode, thenCode }).trim();
      }

      return `
        $condCode
        (if (result i32)
          (then $thenCode)
          $elseClause
        )
      `.papagaio({
        condCode,
        thenCode,
        elseClause: elseCode ? "(else " + elseCode + ")" : ""
      }).trim();
    }

    // Estrutura de Controle: LOOP & BR_IF
    if (op === "loop") {
      // (loop $label ...body)
      const label = parts[1];
      const body = parts.slice(2).map(a => compileExpr(a, scope)).join("\n");
      return `
        (loop $label
          $body
        )
      `.papagaio({ label, body }).trim();
    }

    // BREAK CONDICIONAL (br_if $label cond)
    if (op === "br_if") {
      const label = parts[1];
      const condCode = compileExpr(parts[2], scope);
      return `
        $condCode
        (br_if $label)
      `.papagaio({ condCode, label }).trim();
    }

    // Retorno explícito (return expr)
    if (op === "return") {
      const valCode = args[0] ? compileExpr(args[0], scope) : "";
      return "$valCode return".papagaio({ valCode }).trim();
    }

    // Definição e atribuição de variáveis locais (set $var expr)
    if (op === "set") {
      const varName = parts[1];
      const valCode = compileExpr(parts[2], scope);
      return `
        $valCode
        (local.set $varName)
      `.papagaio({ valCode, varName }).trim();
    }

    // Operações de Memória: load e store
    if (op === "i32.load") {
      const addr = compileExpr(args[0], scope);
      return "$addr i32.load".papagaio({ addr }).trim();
    }
    if (op === "i32.store") {
      const addr = compileExpr(args[0], scope);
      const val = compileExpr(args[1], scope);
      return "$addr $val i32.store".papagaio({ addr, val }).trim();
    }

    // Chamada de função: (call $fn ...args)
    if (op === "call") {
      const fnName = parts[1];
      const compiledArgs = parts.slice(2).map(a => compileExpr(a, scope)).join(" ");
      return "$compiledArgs (call $fnName)".papagaio({ compiledArgs, fnName }).trim();
    }

    // Operadores Aritméticos / Lógicos WASM 1.0 (binários ou unários)
    if (WASM_OPS[op]) {
      const wasmOp = WASM_OPS[op];
      const compiledArgs = args.map(a => compileExpr(a, scope)).join(" ");
      return "$compiledArgs $wasmOp".papagaio({ compiledArgs, wasmOp }).trim();
    }

    // Chamada implícita de função do usuário
    if (op.startsWith("$")) {
      const compiledArgs = args.map(a => compileExpr(a, scope)).join(" ");
      return "$compiledArgs (call $op)".papagaio({ compiledArgs, op }).trim();
    }
  }

  // 4. Identificador de Variável Local ($var)
  if (code.startsWith("$")) {
    return "(local.get $code)".papagaio({ code });
  }

  return code;
}

/**
 * Compilador de Módulo Lisp Completo para WAT
 */
export function compileLispToWat(src) {
  const topForms = splitTopLevelSExprs(src);
  let functionsWat = [];
  let exportsWat = [];
  let memoryWat = "";

  for (const form of topForms) {
    const innerMatch = "$inner$block{(}{)}".papagaio.match(form);
    if (!innerMatch) continue;

    const inner = innerMatch.inner.trim();
    const parts = splitTopLevelSExprs(inner);
    const formType = parts[0];

    // (memory $name initial_pages max_pages?)
    if (formType === "memory") {
      const memName = parts[1] || "";
      const pages = parts[2] || "1";
      memoryWat = `(memory $name $pages) (export "memory" (memory $name))`.papagaio({
        name: memName,
        pages
      });
      continue;
    }

    // (func $name (params ...) (result ...) ...body)
    if (formType === "func") {
      const fnName = parts[1];
      let params = [];
      let resultType = "";
      let locals = [];
      let bodyExprs = [];

      for (let i = 2; i < parts.length; i++) {
        const item = parts[i];
        if (item.startsWith("(param")) {
          // Extrai "(param $x i32)"
          const pMatch = "$paramBlock$block{(}{)}".papagaio.match(item);
          if (pMatch) params.push(`(${pMatch.paramBlock})`);
        } else if (item.startsWith("(result")) {
          const rMatch = "$resBlock$block{(}{)}".papagaio.match(item);
          if (rMatch) resultType = `(${rMatch.resBlock})`;
        } else if (item.startsWith("(local")) {
          const lMatch = "$locBlock$block{(}{)}".papagaio.match(item);
          if (lMatch) locals.push(`(${lMatch.locBlock})`);
        } else {
          bodyExprs.push(item);
        }
      }

      const compiledBody = bodyExprs.map(e => compileExpr(e)).join("\n    ");

      const fnWat = `
  (func $name $params $result
    $locals
    $compiledBody
  )`.papagaio({
        name: fnName,
        params: params.join(" "),
        result: resultType,
        locals: locals.join("\n    "),
        compiledBody
      });

      functionsWat.push(fnWat);
      continue;
    }

    // (export "externalName" $internalFunc)
    if (formType === "export") {
      const extName = parts[1];
      const target = parts[2];
      exportsWat.push(`(export $extName (func $target))`.papagaio({ extName, target }));
    }
  }

  // Montagem do módulo final via template Papagaio
  const moduleWat = `
(module
  $memoryWat
  $exportsWat
  $functionsWat
)
`.papagaio({
    memoryWat,
    exportsWat: exportsWat.join("\n  "),
    functionsWat: functionsWat.join("\n")
  }).trim();

  return moduleWat;
}
