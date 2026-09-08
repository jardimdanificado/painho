// examples/legacy/run_demo.js
// Demonstração do interpretador de sintaxe clássica do Papagaio
// rodando os exemplos e testes do tests.json usando o novo motor Papagaio.

import fs from "node:fs";
import { runLegacy } from "./legacy_interpreter.js";

console.log("=== EXEMPLO: INTERPRETADOR PAPAGAIO LEGADO EM JS ===");
console.log("Executando scripts com sintaxe legada ($pattern, $from, $list, $repeat, $math)...");

// 1. Script com pattern matching clássico
const script1 = `$pattern {$x $y} {$y, $x}
hello world`;
console.log("\n[1] Padrão com inversão de variáveis:");
console.log("Fonte:\n" + script1);
console.log("Saída:", JSON.stringify(runLegacy(script1)));

// 2. Script com controle de fluxo e listas
const script2 = `$L$from{a}$repeat{3}{$L$list{,}$push{x}}$L`;
console.log("\n[2] Mutação de listas com $repeat e $list$push:");
console.log("Fonte:\n" + script2);
console.log("Saída:", JSON.stringify(runLegacy(script2)));

// 3. Script com acumulador de bytes
const script3 = `$A$from{}$repeat{3}{$A$byte{65}}$A`;
console.log("\n[3] Acumulador de bytes ($byte):");
console.log("Fonte:\n" + script3);
console.log("Saída:", JSON.stringify(runLegacy(script3)));

// 4. Script com avaliação matemática Louro
const script4 = `$A$from{5}$math{$A * $A + 10}`;
console.log("\n[4] Avaliação matemática ($math):");
console.log("Fonte:\n" + script4);
console.log("Saída:", JSON.stringify(runLegacy(script4)));

// 5. Execução em lote dos testes de tests.json
console.log("\n[5] Executando bateria de testes do tests.json original...");
const testsFile = "tests/tests.json";
if (fs.existsSync(testsFile)) {
  const tests = JSON.parse(fs.readFileSync(testsFile, "utf-8")).tests;
  let passed = 0;
  let total = tests.length;

  for (const t of tests) {
    try {
      const res = runLegacy(t.code);
      if (res === t.expected) {
        passed++;
      }
    } catch (e) {
      // continua
    }
  }

  console.log(`\n🎉 Testes concluídos com sucesso!`);
  console.log(`Sucesso em testes clássicos: ${passed} de ${total} (${((passed / total) * 100).toFixed(1)}%)`);
}
