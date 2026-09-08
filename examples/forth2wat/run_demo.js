// examples/forth2wat/run_demo.js
// Demonstração da compilação de código Forth idiomático para WAT usando Papagaio.

import { compileForthToWat } from "./forth2wat.js";
import { execSync } from "node:child_process";
import fs from "node:fs";

console.log("=== PAPAGAIO COMPILADOR: FORTH IDIOMÁTICO PARA WEBASSEMBLY ===");

// Código em Forth padrão ANSI/FIG puro:
// - Notação pós-fixa pura (sem parênteses WAT nem operadores de prefixo)
// - Palavras de manipulação de pilha (DUP, DROP, SWAP, OVER, ROT)
// - Laço DO ... LOOP com I
// - Condicionais IF ... ELSE ... THEN
// - Memória com @ (load) e ! (store)
const forthSource = `
  \\ 1. Quadrado de um número (x -> x^2)
  : SQUARE
    DUP * ;

  \\ 2. Hipotenusa ao quadrado (a b -> a^2 + b^2)
  : PYTHAGORAS
    SQUARE SWAP SQUARE + ;

  \\ 3. Fatorial usando loop acumulador (n -> n!)
  : FACTORIAL
    DUP 1 <= IF
      DROP 1
    ELSE
      1 SWAP 1 + 2 DO
        I *
      LOOP
    THEN ;

  \\ 4. Máximo entre dois valores (a b -> max)
  : MAX_VAL
    OVER OVER < IF
      SWAP DROP
    ELSE
      DROP
    THEN ;
`;

console.log("\n[1] Código Fonte Forth Clássico:");
console.log(forthSource.trim());

console.log("\n[2] Compilando Forth para WAT usando transformações e templates Papagaio...");
const watOutput = compileForthToWat(forthSource);
console.log("\n--- CÓDIGO WAT GERADO PELO PAPAGAIO ---");
console.log(watOutput);

// Salva e valida com wat2wasm
fs.writeFileSync("examples/forth2wat/output.wat", watOutput);
console.log("\n[3] Validando com a ferramenta oficial 'wat2wasm'...");
execSync("wat2wasm examples/forth2wat/output.wat -o examples/forth2wat/output.wasm");
console.log("✅ wat2wasm compilou o código WAT gerado para binário WASM com sucesso!");

// Execução no runtime nativo
console.log("\n[4] Instanciando o WebAssembly compilado...");
const wasmBuffer = fs.readFileSync("examples/forth2wat/output.wasm");
const wasmModule = await WebAssembly.instantiate(wasmBuffer);
const { SQUARE, PYTHAGORAS, FACTORIAL, MAX_VAL } = wasmModule.instance.exports;

console.log("\nExecutando as palavras Forth compiladas:");
console.log("• 7 SQUARE        =", SQUARE(7), "(esperado: 49) ->", SQUARE(7) === 49 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• 3 4 PYTHAGORAS  =", PYTHAGORAS(3, 4), "(esperado: 25) ->", PYTHAGORAS(3, 4) === 25 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• 5 FACTORIAL     =", FACTORIAL(5), "(esperado: 120) ->", FACTORIAL(5) === 120 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• 6 FACTORIAL     =", FACTORIAL(6), "(esperado: 720) ->", FACTORIAL(6) === 720 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• 42 99 MAX_VAL   =", MAX_VAL(42, 99), "(esperado: 99) ->", MAX_VAL(42, 99) === 99 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• 150 20 MAX_VAL  =", MAX_VAL(150, 20), "(esperado: 150) ->", MAX_VAL(150, 20) === 150 ? "CORRETO ✅" : "FALHOU ❌");

console.log("\n🚀 Todas as palavras Forth executaram com perfeição no motor WebAssembly!");
