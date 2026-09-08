# Legacy Papagaio Compatibility Interpreter Tutorial

This tutorial demonstrates how to build a **compatibility interpreter for classic/historical Papagaio syntax** (`$pattern`, `$from`, `$list`, `$repeat`, `$while`, `$until`, `$compare`, `$then`, `$else`, `$byte`, `$math`) written **100% on top of the new ES2023 Papagaio (`String.prototype.papagaio`)**.

---

## 🎯 What You Will Learn

1. How to use modern Papagaio as a meta-language to parse and execute older DSLs.
2. How to implement dynamic environment scopes and symbol lookup using native JavaScript `Map` collections.
3. How to emulate historical macro constructs without relying on C binaries, DLL loaders, or complex build setups.
4. How to execute test suites from `tests/tests.json` against the recreated interpreter.

---

## 💡 How it Works

The original C Papagaio parsed directive tokens starting with `$` and evaluated blocks delimited by `{}`.

In [examples/legacy/legacy_interpreter.js](./legacy_interpreter.js), each legacy directive is processed using the new Papagaio:

### 1. Parsing `$pattern {match} {replacement}`
Legacy patterns are extracted and registered into an active rules table:
```javascript
// Each $pattern rule is executed via the new .papagaio.replace() method:
text = rule.matchPat.papagaio.replace(text, (captures) => {
  let rep = rule.replaceStr;
  // Dynamic replacement using captures
  for (const [k, v] of Object.entries(captures)) {
    rep = rep.replaceAll(`$${k}`, v);
  }
  return rep;
});
```

### 2. Emulating List Mutations (`$list{sep}$op`)
Operations such as `$list{,}$push{item}`, `$list{,}$pop`, and `$list{,}$join{ - }` are handled by delegating the actual work to standard JavaScript Arrays:
```javascript
// Split on separator
let parts = sep === "" ? listVal.split("") : listVal.split(sep);

// Execute operation
if (op === "push") parts.push(item);
if (op === "pop")  emitStr = parts.pop();
if (op === "join") emitStr = parts.join(newSep);

// Re-serialize back to environment variable
this.setVar(varName, parts.join(sep));
```

### 3. Emulating Math (`$math{expr}`)
The TinyExpr / Louro math evaluator is reproduced using native JavaScript expressions and `Math.*` built-ins:
```javascript
// Clean Louro conditionals and operators
expr = expr.replace(/\bif\s+(.+?)\s+then\s+(.+?)\s+else\s+(.+?)\s+end\b/g, "($1 ? $2 : $3)");
expr = expr.replace(/\^/g, "**");

// Evaluate safely with Math functions exposed
const fn = new Function(...mathKeys, `"use strict"; return (${expr});`);
return String(fn(...mathVals));
```

---

## 🚀 Running the Legacy Demo

Run the demo script:

```sh
node examples/legacy/run_demo.js
```

### Sample Demonstrations:
```text
[1] Pattern with variable inversion:
$pattern {$x $y} {$y, $x}
hello world
-> "world, hello"

[2] List mutation with $repeat and $list$push:
$L$from{a}$repeat{3}{$L$list{,}$push{x}}$L
-> "a,x,x,x"

[3] Byte accumulator ($byte):
$A$from{}$repeat{3}{$A$byte{65}}$A
-> "AAA"

[4] Math evaluation ($math):
$A$from{5}$math{$A * $A + 10}
-> "35"
```
