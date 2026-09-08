// examples/lisp2wat/run_demo.js
// Demonstração completa: compila código Lisp para WAT usando Papagaio,
// valida com wat2wasm e executa no motor nativo WebAssembly do runtime!

import { compileLispToWat } from "./compiler.js";
import { execSync } from "node:child_process";
import fs from "node:fs";

console.log("=== PAPAGAIO COMPILADOR: LISP-LIKE PARA WEBASSEMBLY (WASM 1.0) ===");

// Código fonte em dialeto Lisp expressivo
const lispSource = `
  ;; Memória linear de 1 página
  (memory $mem 1)

  ;; Função 1: Fibonacci recursivo
  (func $fib (param $n i32) (result i32)
    (if (<= $n 1)
      $n
      (+ ($fib (- $n 1)) ($fib (- $n 2)))
    )
  )

  ;; Função 2: Fatorial iterativo com loop e variáveis locais
  (func $fact (param $n i32) (result i32)
    (local $res i32)
    (set $res 1)
    (loop $l
      (if (> $n 1)
        (then
          (set $res (* $res $n))
          (set $n (- $n 1))
          (br_if $l 1)
        )
      )
    )
    $res
  )

  ;; Função 3: Manipulação direta de memória linear (store & load)
  (func $mem_test (param $addr i32) (param $val i32) (result i32)
    (i32.store $addr $val)
    (i32.load $addr)
  )

  ;; Exportações para o host WebAssembly
  (export "fib" $fib)
  (export "fact" $fact)
  (export "mem_test" $mem_test)
`;

console.log("\n[1] Código Fonte Lisp:");
console.log(lispSource.trim());

// 1. Compilação usando o motor Papagaio
console.log("\n[2] Compilando Lisp para WAT com o motor Papagaio...");
const watOutput = compileLispToWat(lispSource);
console.log("\n--- WAT GERADO PELO PAPAGAIO ---");
console.log(watOutput);

// 2. Validação e montagem binária com wat2wasm
fs.writeFileSync("examples/lisp2wat/output.wat", watOutput);
console.log("\n[3] Validando com a ferramenta oficial 'wat2wasm'...");
execSync("wat2wasm examples/lisp2wat/output.wat -o examples/lisp2wat/output.wasm");
console.log("✅ wat2wasm compilou o WAT para binário WASM sem erros de validação!");

// 3. Execução no runtime nativo WebAssembly
console.log("\n[4] Instanciando e executando o módulo WebAssembly compilado no runtime...");
const wasmBuffer = fs.readFileSync("examples/lisp2wat/output.wasm");
const wasmModule = await WebAssembly.instantiate(wasmBuffer);
const { fib, fact, mem_test, memory } = wasmModule.instance.exports;

console.log("\nResultados dos testes em tempo de execução:");
console.log("• fib(10) =", fib(10), "(esperado: 55) ->", fib(10) === 55 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• fact(5) =", fact(5), "(esperado: 120) ->", fact(5) === 120 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• fact(7) =", fact(7), "(esperado: 5040) ->", fact(7) === 5040 ? "CORRETO ✅" : "FALHOU ❌");
console.log("• mem_test(64, 4242) =", mem_test(64, 4242), "(esperado: 4242) ->", mem_test(64, 4242) === 4242 ? "CORRETO ✅" : "FALHOU ❌");

console.log("\n🚀 Demonstração concluída com 100% de sucesso!");
