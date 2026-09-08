// test/suite.test.js
// Bateria de testes da reimaginação do Papagaio para Node.js, Bun e QuickJS
import assert from "node:assert/strict";
import { papagaio } from "../src/index.js";

console.log("=== INICIANDO SUÍTE DE TESTES PAPAGAIO ES2023 ===");

// 1. Interpolação Básica e Variáveis Simples
{
  const res = "Olá $nome".papagaio({ nome: "Jardel" });
  assert.equal(res, "Olá Jardel", "Interpolação simples via String.prototype falhou");

  const resFunc = papagaio("Olá $nome", { nome: "Maria" });
  assert.equal(resFunc, "Olá Maria", "Interpolação via função papagaio falhou");
}

// 2. Expressões JavaScript ES2023
{
  const resUpper = "Olá ${nome.toUpperCase()}".papagaio({ nome: "jardel" });
  assert.equal(resUpper, "Olá JARDEL", "Expressão toUpperCase falhou");

  const resMath = "Total: ${price * quantity}".papagaio({ price: 10, quantity: 5 });
  assert.equal(resMath, "Total: 50", "Expressão matemática falhou");

  const resArray = "Itens: ${items.join(', ')}".papagaio({ items: ["A", "B", "C"] });
  assert.equal(resArray, "Itens: A, B, C", "Expressão de array falhou");

  const resTernary = "${age >= 18 ? 'Adulto' : 'Menor'}".papagaio({ age: 20 });
  assert.equal(resTernary, "Adulto", "Operador ternário falhou");
}

// 3. Pattern Matching e Captures
{
  const matchRgb = "rgb($r,$g,$b)".papagaio.match("rgb(255,128,64)");
  assert.deepEqual(matchRgb, { r: "255", g: "128", b: "64" }, "Pattern match simples falhou");

  const matchGreeting = "Hello $name".papagaio.match("Hello Jardel");
  assert.deepEqual(matchGreeting, { name: "Jardel" }, "Hello $name match falhou");

  const noMatch = "foo $x".papagaio.match("bar 123");
  assert.equal(noMatch, null, "Padrão não correspondente deveria retornar null");
}

// 4. Modifiers
{
  // int
  assert.ok("valor: $x$int".papagaio.match("valor: 42"));
  assert.equal("valor: $x$int".papagaio.match("valor: abc"), null);

  // upper / lower
  assert.ok("$code$upper".papagaio.match("ABC"));
  assert.equal("$code$upper".papagaio.match("abc"), null);
  assert.ok("$code$lower".papagaio.match("abc"));

  // hex
  assert.ok("cor: $c$hex".papagaio.match("cor: 0xff00aa"));

  // identifier
  assert.ok("fn: $f$identifier".papagaio.match("fn: minhaFuncao_123"));
  assert.equal("fn: $f$identifier".papagaio.match("fn: 123invalido"), null);
}

// 5. Delimitadores de Bloco Balanceados ($block)
{
  const resBlock = "$content$block{(}{)}".papagaio.match("(a (b (c) d) e)");
  assert.deepEqual(resBlock, { content: "a (b (c) d) e" }, "Bloco balanceado falhou");

  const resAngle = "$txt$block{<}{>}".papagaio.match("<outer <inner> final>");
  assert.deepEqual(resAngle, { txt: "outer <inner> final" }, "Bloco angle brackets falhou");
}

// 6. Alternativas ($aliases)
{
  const resAlias = "$tipo$aliases{gato}{cachorro}{papagaio}".papagaio.match("papagaio");
  assert.deepEqual(resAlias, { tipo: "papagaio" }, "Aliases match falhou");
}

// 7. Trailing Sigil e Flex-matching
{
  const resWs = "$a$ $b".papagaio.match("hello     world");
  assert.deepEqual(resWs, { a: "hello", b: "world" }, "Flex-matching / trailing sigil falhou");
}

// 8. Replacement com Captures e Funções
{
  const repStr = "rgb($r,$g,$b)".papagaio.replace("cor: rgb(10,20,30)", "R:$r G:$g B:$b");
  assert.equal(repStr, "cor: R:10 G:20 B:30", "Replacement com string de template falhou");

  const repFn = "$x $y".papagaio.replace("hello world", (caps) => `${caps.y}, ${caps.x}`);
  assert.equal(repFn, "world, hello", "Replacement com função falhou");
}

// 9. Compilação (compile)
{
  const compiled = "Olá $nome".papagaio.compile();
  assert.equal(compiled({ nome: "Jardel" }), "Olá Jardel");
  assert.equal(compiled({ nome: "Ana" }), "Olá Ana");
}

// 10. Regressão direta com casos de teste originais (tests.json)
{
  // Test 38: int
  assert.equal(
    "$n$int".papagaio.replace("123 and ABC and -45", "INT[$n]"),
    "INT[123] and ABC andINT[-45]",
    "Regressão Test 38 (int) falhou"
  );

  // Test 39: float
  assert.equal(
    "$n$float".papagaio.replace("12.3 and ABC and -4.5", "FLT[$n]"),
    "FLT[12.3] and ABC andFLT[-4.5]",
    "Regressão Test 39 (float) falhou"
  );

  // Test 40: upper / lower
  assert.equal(
    "$u$upper $l$lower".papagaio.replace("HELLO world and HELLO WORLD", "MATCH"),
    "MATCH and HELLO WORLD",
    "Regressão Test 40 (upper/lower) falhou"
  );

  // Test 42: hex
  assert.equal(
    "$h$hex".papagaio.replace("1a2B and xy", "HEX[$h]"),
    "HEX[1a2B]HEX[a]nHEX[d] xy",
    "Regressão Test 42 (hex) falhou"
  );

  // Test 43: binary
  assert.equal(
    "$b$binary".papagaio.replace("0101 and 123", "BIN[$b]"),
    "BIN[0101] andBIN[1]23",
    "Regressão Test 43 (binary) falhou"
  );

  // Test 46: starts
  assert.equal(
    "$w$starts{pre}".papagaio.replace("prepare\npremium\npost\npre", "MATCH[$w]"),
    "MATCH[prepare]\nMATCH[premium]\npost\nMATCH[pre]",
    "Regressão Test 46 (starts) falhou"
  );

  // Test 47: ends
  assert.equal(
    "$w$ends{ing}".papagaio.replace("running\nswimming\nsing\ning", "MATCH[$w]"),
    "MATCH[running]\nMATCH[swimming]\nMATCH[sing]\nMATCH[ing]",
    "Regressão Test 47 (ends) falhou"
  );

  // Test 48: prefix
  assert.equal(
    "$w$prefix{pre}".papagaio.replace("prepare\npremium\npost\npre", "MATCH[$w]"),
    "MATCH[prepare]\nMATCH[premium]\npost\npre",
    "Regressão Test 48 (prefix) falhou"
  );

  // Test 49: suffix
  assert.equal(
    "$w$suffix{ing}".papagaio.replace("running\nswimming\nsing\ning", "MATCH[$w]"),
    "MATCH[running]\nMATCH[swimming]\nMATCH[sing]\ning",
    "Regressão Test 49 (suffix) falhou"
  );

  // Test 50: infix
  assert.equal(
    "$w$infix{in}".papagaio.replace("pinball\nwindows\ninside\npin\nin", "MATCH[$w]"),
    "MATCH[pinball]\nMATCH[windows]\ninside\npin\nin",
    "Regressão Test 50 (infix) falhou"
  );

  // Test 51: includes
  assert.equal(
    "$w$includes{in}".papagaio.replace("pinball\nwindows\ninside\npin\nin", "MATCH[$w]"),
    "MATCH[pinball]\nMATCH[windows]\nMATCH[inside]\nMATCH[pin]\nMATCH[in]",
    "Regressão Test 51 (includes) falhou"
  );

  // Test 64: braced variable ambiguity
  assert.equal(
    "$id$word".papagaio.replace("foo", "${id}x"),
    "foox",
    "Regressão Test 64 (braced var ambiguity) falhou"
  );

  // Test 70: braced variable unknown kept as-is
  assert.equal(
    "$x$word".papagaio.replace("foo", "${x}suffix${unknown}"),
    "foosuffix${unknown}",
    "Regressão Test 70 (braced var unknown) falhou"
  );
}

console.log("✅ TODOS OS TESTES PASSARAM COM SUCESSO!");

