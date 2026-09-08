// examples/legacy/legacy_interpreter.js
// Interpretador de compatibilidade para código Papagaio clássico / legado
// Construído 100% sobre o novo Papagaio e JS ES2023.

import { papagaio } from "../../src/index.js";

/**
 * Avaliador matemático compatível com $math{} (TinyExpr / Louro)
 */
function evalMathExpr(expr) {
  try {
    let clean = expr.trim();
    // Suporte a if cond then a else b end
    clean = clean.replace(/\bif\s+(.+?)\s+then\s+(.+?)\s+else\s+(.+?)\s+end\b/g, "($1 ? $2 : $3)");
    // Suporte a ^ como potência
    clean = clean.replace(/\^/g, "**");
    // Funções matemáticas padrão
    const mathContext = {
      sqrt: Math.sqrt,
      abs: Math.abs,
      sin: Math.sin,
      cos: Math.cos,
      tan: Math.tan,
      atan2: Math.atan2,
      floor: Math.floor,
      ceil: Math.ceil,
      round: Math.round,
      min: Math.min,
      max: Math.max,
      log: Math.log,
      exp: Math.exp
    };

    const keys = Object.keys(mathContext);
    const vals = Object.values(mathContext);
    const fn = new Function(...keys, `"use strict"; return (${clean});`);
    const res = fn(...vals);

    if (typeof res === "number") {
      if (Number.isInteger(res)) return String(res);
      return String(Number(res.toFixed(6)));
    }
    if (typeof res === "boolean") return res ? "1" : "0";
    return String(res ?? "");
  } catch (e) {
    return "";
  }
}

/**
 * Interpretador completo do pipeline de comandos legados do Papagaio.
 */
export class LegacyPapagaio {
  constructor(options = {}) {
    this.options = options;
    this.scopes = [new Map()];
    this.rules = [];
    this.originalDoc = "";
    this.depth = 0;
  }

  get currentScope() {
    return this.scopes[this.scopes.length - 1];
  }

  getVar(name) {
    for (let i = this.scopes.length - 1; i >= 0; i--) {
      if (this.scopes[i].has(name)) {
        return this.scopes[i].get(name);
      }
    }
    return null;
  }

  setVar(name, val) {
    for (let i = this.scopes.length - 1; i >= 0; i--) {
      if (this.scopes[i].has(name)) {
        this.scopes[i].set(name, String(val));
        return;
      }
    }
    this.currentScope.set(name, String(val));
  }

  pushScope() {
    this.scopes.push(new Map());
  }

  popScope() {
    if (this.scopes.length > 1) {
      this.scopes.pop();
    }
  }

  extractBlock(str, pos, open = "{", close = "}") {
    if (!str.startsWith(open, pos)) return null;
    let depth = 1;
    let p = pos + open.length;
    const start = p;
    while (p < str.length && depth > 0) {
      if (str.startsWith(open, p) && open !== close) {
        depth++;
        p += open.length;
      } else if (str.startsWith(close, p)) {
        depth--;
        if (depth === 0) break;
        p += close.length;
      } else {
        p++;
      }
    }
    return {
      content: str.slice(start, p),
      nextPos: p + close.length
    };
  }

  process(input) {
    if (this.depth >= 64) return input;
    this.depth++;
    if (this.depth === 1) {
      this.originalDoc = input;
    }

    // 1. Extrai regras de $pattern e armazena
    let text = this.#extractPatterns(input);

    // 2. Resolve comandos imperativos e variáveis no texto ($from, $list, $repeat, $while, etc.)
    text = this.#resolveCommands(text);

    // 3. Aplica regras de pattern
    for (const rule of this.rules) {
      text = rule.matchPat.papagaio.replace(text, (captures) => {
        let rep = rule.replaceStr;

        // Trata $once dentro do replacement
        if (rep.includes("$once")) {
          for (const [k, v] of Object.entries(captures)) {
            rep = rep.replaceAll(`$${k}`, v);
            rep = rep.replaceAll(`\${${k}}`, v);
          }
          let onceMatch;
          while ((onceMatch = rep.match(/\$once\{/))) {
            const blk = this.extractBlock(rep, onceMatch.index + 5);
            if (blk) {
              const evaluated = this.process(blk.content);
              rep = rep.slice(0, onceMatch.index) + evaluated + rep.slice(blk.nextPos);
            } else break;
          }
        }

        // Substituição normal
        for (const [k, v] of Object.entries(captures)) {
          rep = rep.replaceAll(`\${${k}}`, v);
          rep = rep.replaceAll(`$${k}`, v);
        }

        rep = rep.replaceAll("$$", "$");
        return rep;
      });
    }

    // 4. Pós-passagem de comandos residuais
    text = this.#resolveCommands(text);

    // Substitui variáveis que restaram no texto final
    for (let s = this.scopes.length - 1; s >= 0; s--) {
      for (const [k, v] of this.scopes[s].entries()) {
        text = text.replaceAll(`$${k}`, v);
      }
    }

    this.depth--;
    return text;
  }

  #extractPatterns(src) {
    let out = "";
    let i = 0;
    const len = src.length;

    while (i < len) {
      if (src.startsWith("$pattern", i)) {
        let p = i + 8;
        while (p < len && /\s/.test(src[p])) p++;
        const blk1 = this.extractBlock(src, p);
        if (blk1) {
          let p2 = blk1.nextPos;
          while (p2 < len && /\s/.test(src[p2])) p2++;
          const blk2 = this.extractBlock(src, p2);
          if (blk2) {
            this.rules.push({
              matchPat: blk1.content.trim(),
              replaceStr: blk2.content
            });
            i = blk2.nextPos;
            if (i < len && src[i] === "\n") i++;
            continue;
          }
        }
      }
      out += src[i];
      i++;
    }
    return out;
  }

  #resolveCommands(src) {
    let out = "";
    let i = 0;
    const len = src.length;

    while (i < len) {
      // Literais de whitespace e constantes
      if (src.startsWith("$space", i)) {
        out += " ";
        i += 6;
        continue;
      }
      if (src.startsWith("$newline", i)) {
        out += "\n";
        i += 8;
        continue;
      }
      if (src.startsWith("$tab", i)) {
        out += "\t";
        i += 4;
        continue;
      }
      if (src.startsWith("$document$original", i)) {
        out += this.originalDoc;
        i += 18;
        continue;
      }
      if (src.startsWith("$sigil", i)) {
        out += "$";
        i += 6;
        continue;
      }

      // $byte{N}
      if (src.startsWith("$byte", i)) {
        let p = i + 5;
        while (p < len && /\s/.test(src[p])) p++;
        const blk = this.extractBlock(src, p);
        if (blk) {
          const valStr = this.process(blk.content).trim();
          const codeVal = parseInt(valStr, 10);
          if (!isNaN(codeVal)) out += String.fromCharCode(codeVal);
          i = blk.nextPos;
          continue;
        }
      }

      // $ascii{N} ou $ascii$N
      if (src.startsWith("$ascii", i)) {
        let p = i + 6;
        if (src.startsWith("$", p)) {
          p++;
          let endP = p;
          while (endP < len && /\d/.test(src[endP])) endP++;
          const codeVal = parseInt(src.slice(p, endP), 10);
          if (!isNaN(codeVal)) out += String.fromCharCode(codeVal);
          i = endP;
          continue;
        }
        while (p < len && /\s/.test(src[p])) p++;
        const blk = this.extractBlock(src, p);
        if (blk) {
          const valStr = this.process(blk.content).trim();
          const codeVal = parseInt(valStr, 10);
          if (!isNaN(codeVal)) out += String.fromCharCode(codeVal);
          i = blk.nextPos;
          continue;
        }
      }

      // $math{expr}
      if (src.startsWith("$math", i)) {
        let p = i + 5;
        while (p < len && /\s/.test(src[p])) p++;
        const blk = this.extractBlock(src, p);
        if (blk) {
          // Substitui variáveis antes de calcular
          let innerExpr = blk.content;
          for (let s = this.scopes.length - 1; s >= 0; s--) {
            for (const [k, v] of this.scopes[s].entries()) {
              innerExpr = innerExpr.replaceAll(`$${k}`, v);
            }
          }
          out += evalMathExpr(innerExpr);
          i = blk.nextPos;
          continue;
        }
      }

      // $once{expr}
      if (src.startsWith("$once", i)) {
        let p = i + 5;
        while (p < len && /\s/.test(src[p])) p++;
        const blk = this.extractBlock(src, p);
        if (blk) {
          out += this.process(blk.content);
          i = blk.nextPos;
          continue;
        }
      }

      // Tratamento de cadeias: $VAR... ou ${EXPR}...
      if (src[i] === "$") {
        let isBraced = false;
        let varName = "";
        let cp = i + 1;

        if (src[cp] === "{") {
          const b = this.extractBlock(src, cp);
          if (b) {
            isBraced = true;
            varName = b.content;
            cp = b.nextPos;
          }
        } else {
          let vEnd = cp;
          while (vEnd < len && /[a-zA-Z0-9_]/.test(src[vEnd])) vEnd++;
          varName = src.slice(cp, vEnd);
          cp = vEnd;
        }

        let isChain = false;
        let curVal = isBraced ? this.process(varName) : (varName ? (this.getVar(varName) ?? "") : "");

        while (cp < len && src[cp] === "$") {
          let opEnd = cp + 1;
          while (opEnd < len && /[a-zA-Z0-9_]/.test(src[opEnd])) opEnd++;
          const opName = src.slice(cp + 1, opEnd);

          // $from{val}
          if (opName === "from") {
            isChain = true;
            let p = opEnd;
            while (p < len && /\s/.test(src[p])) p++;
            const blk = this.extractBlock(src, p);
            if (blk) {
              this.pushScope();
              const assignedVal = this.process(blk.content);
              this.popScope();
              this.setVar(varName, assignedVal);
              curVal = ""; // Atribuição $VAR$from{} emite nada por padrão!
              cp = blk.nextPos;
              continue;
            }
          }

          // $byte{code} encadeado em variável
          if (opName === "byte") {
            isChain = true;
            let p = opEnd;
            while (p < len && /\s/.test(src[p])) p++;
            const blk = this.extractBlock(src, p);
            if (blk) {
              const codeVal = parseInt(this.process(blk.content).trim(), 10);
              if (!isNaN(codeVal)) {
                curVal += String.fromCharCode(codeVal);
                if (varName) this.setVar(varName, curVal);
              }
              cp = blk.nextPos;
              continue;
            }
          }

          // $list{sep}$op...
          if (opName === "list") {
            isChain = true;
            let p = opEnd;
            while (p < len && /\s/.test(src[p])) p++;
            const sepBlk = this.extractBlock(src, p);
            if (sepBlk) {
              const sep = this.process(sepBlk.content);
              let lopPos = sepBlk.nextPos;
              while (lopPos < len && /\s/.test(src[lopPos])) lopPos++;
              if (src[lopPos] === "$") {
                let lopEnd = lopPos + 1;
                while (lopEnd < len && /[a-zA-Z0-9_]/.test(src[lopEnd])) lopEnd++;
                const lopName = src.slice(lopPos + 1, lopEnd);

                let listVal = this.getVar(varName) ?? "";
                let parts = sep === "" ? listVal.split("") : listVal.split(sep);
                if (listVal === "") parts = [];

                let emitStr = "";
                let mutated = false;
                let nextP = lopEnd;

                if (lopName === "count") {
                  emitStr = String(parts.length);
                } else if (lopName === "get") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    let idx = parseInt(this.process(b.content), 10);
                    if (idx < 0) idx = parts.length + idx;
                    emitStr = parts[idx] ?? "";
                    nextP = b.nextPos;
                  }
                } else if (lopName === "push") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    parts.push(this.process(b.content));
                    mutated = true;
                    nextP = b.nextPos;
                  }
                } else if (lopName === "pop") {
                  emitStr = parts.pop() ?? "";
                  mutated = true;
                } else if (lopName === "shift") {
                  emitStr = parts.shift() ?? "";
                  mutated = true;
                } else if (lopName === "unshift") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    parts.unshift(this.process(b.content));
                    mutated = true;
                    nextP = b.nextPos;
                  }
                } else if (lopName === "set") {
                  const b1 = this.extractBlock(src, nextP);
                  if (b1) {
                    const b2 = this.extractBlock(src, b1.nextPos);
                    if (b2) {
                      let idx = parseInt(this.process(b1.content), 10);
                      if (idx < 0) idx = parts.length + idx;
                      parts[idx] = this.process(b2.content);
                      mutated = true;
                      nextP = b2.nextPos;
                    }
                  }
                } else if (lopName === "insert") {
                  const b1 = this.extractBlock(src, nextP);
                  if (b1) {
                    const b2 = this.extractBlock(src, b1.nextPos);
                    if (b2) {
                      let idx = parseInt(this.process(b1.content), 10);
                      if (idx < 0) idx = parts.length + idx;
                      parts.splice(idx, 0, this.process(b2.content));
                      mutated = true;
                      nextP = b2.nextPos;
                    }
                  }
                } else if (lopName === "remove") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    let idx = parseInt(this.process(b.content), 10);
                    if (idx < 0) idx = parts.length + idx;
                    parts.splice(idx, 1);
                    mutated = true;
                    nextP = b.nextPos;
                  }
                } else if (lopName === "swap") {
                  const b1 = this.extractBlock(src, nextP);
                  if (b1) {
                    const b2 = this.extractBlock(src, b1.nextPos);
                    if (b2) {
                      let i1 = parseInt(this.process(b1.content), 10);
                      let i2 = parseInt(this.process(b2.content), 10);
                      if (i1 < 0) i1 = parts.length + i1;
                      if (i2 < 0) i2 = parts.length + i2;
                      const tmp = parts[i1];
                      parts[i1] = parts[i2];
                      parts[i2] = tmp;
                      mutated = true;
                      nextP = b2.nextPos;
                    }
                  }
                } else if (lopName === "reverse") {
                  parts.reverse();
                  mutated = true;
                } else if (lopName === "join") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    emitStr = parts.join(this.process(b.content));
                    nextP = b.nextPos;
                  }
                } else if (lopName === "find") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    const pat = this.process(b.content);
                    const found = parts.find(el => el.includes(pat));
                    emitStr = found ?? "";
                    nextP = b.nextPos;
                  }
                } else if (lopName === "contains") {
                  const b = this.extractBlock(src, nextP);
                  if (b) {
                    const pat = this.process(b.content);
                    for (const el of parts) {
                      const cidx = el.indexOf(pat);
                      if (cidx !== -1) {
                        emitStr = String(cidx);
                        break;
                      }
                    }
                    nextP = b.nextPos;
                  }
                } else if (lopName === "replace") {
                  const b1 = this.extractBlock(src, nextP);
                  if (b1) {
                    const b2 = this.extractBlock(src, b1.nextPos);
                    if (b2) {
                      const pat = this.process(b1.content);
                      const rep = this.process(b2.content);
                      for (let pi = 0; pi < parts.length; pi++) {
                        if (parts[pi].includes(pat)) {
                          emitStr = pat;
                          parts[pi] = parts[pi].replace(pat, rep);
                          mutated = true;
                          break;
                        }
                      }
                      nextP = b2.nextPos;
                    }
                  }
                } else if (lopName === "slice") {
                  const b1 = this.extractBlock(src, nextP);
                  if (b1) {
                    let s = parseInt(this.process(b1.content), 10);
                    let e = parts.length;
                    let pEnd = b1.nextPos;
                    const b2 = this.extractBlock(src, b1.nextPos);
                    if (b2) {
                      e = parseInt(this.process(b2.content), 10);
                      pEnd = b2.nextPos;
                    }
                    emitStr = parts.slice(s, e).join(sep);
                    nextP = pEnd;
                  }
                }

                if (mutated) {
                  this.setVar(varName, parts.join(sep));
                }
                out += emitStr;
                cp = nextP;
                continue;
              }
            }
          }

          // $compare{target}
          if (opName === "compare") {
            isChain = true;
            const blk = this.extractBlock(src, opEnd);
            if (blk) {
              const target = this.process(blk.content);
              curVal = (curVal === target) ? curVal : "";
              cp = blk.nextPos;
              continue;
            }
          }

          // $then{content}
          if (opName === "then") {
            isChain = true;
            const blk = this.extractBlock(src, opEnd);
            if (blk) {
              if (curVal !== "") {
                this.pushScope();
                curVal = this.process(blk.content);
                this.popScope();
              }
              cp = blk.nextPos;
              continue;
            }
          }

          // $else{content}
          if (opName === "else") {
            isChain = true;
            const blk = this.extractBlock(src, opEnd);
            if (blk) {
              if (curVal === "") {
                this.pushScope();
                curVal = this.process(blk.content);
                this.popScope();
              }
              cp = blk.nextPos;
              continue;
            }
          }

          // $repeat{N}{code}
          if (opName === "repeat") {
            isChain = true;
            const b1 = this.extractBlock(src, opEnd);
            if (b1) {
              const b2 = this.extractBlock(src, b1.nextPos);
              if (b2) {
                const count = parseInt(this.process(b1.content), 10);
                if (!isNaN(count) && count > 0) {
                  for (let rep = 0; rep < count; rep++) {
                    this.process(b2.content);
                  }
                }
                curVal = ""; // $repeat emite nada
                cp = b2.nextPos;
                continue;
              }
            }
          }

          // $while{pat}{code}
          if (opName === "while") {
            isChain = true;
            const b1 = this.extractBlock(src, opEnd);
            if (b1) {
              const b2 = this.extractBlock(src, b1.nextPos);
              if (b2) {
                const pat = b1.content.trim();
                let lastRes = "";
                let iters = 0;
                while (iters < 500) {
                  const stepRes = this.process(b2.content);
                  const m = pat.papagaio.match(stepRes);
                  if (!m && stepRes !== pat) break;
                  lastRes = stepRes;
                  iters++;
                }
                curVal = lastRes;
                cp = b2.nextPos;
                continue;
              }
            }
          }

          // $until{pat}{code}
          if (opName === "until") {
            isChain = true;
            const b1 = this.extractBlock(src, opEnd);
            if (b1) {
              const b2 = this.extractBlock(src, b1.nextPos);
              if (b2) {
                const pat = b1.content.trim();
                let lastRes = "";
                let iters = 0;
                while (iters < 500) {
                  const stepRes = this.process(b2.content);
                  const m = pat.papagaio.match(stepRes);
                  lastRes = stepRes;
                  if (m || stepRes === pat) break;
                  iters++;
                }
                curVal = lastRes;
                cp = b2.nextPos;
                continue;
              }
            }
          }

          // $find{pattern}, $contains{pattern}, $replace{pat}{rep}, $slice{s}{e}
          if (opName === "find") {
            isChain = true;
            const b = this.extractBlock(src, opEnd);
            if (b) {
              const p = this.process(b.content);
              curVal = curVal.includes(p) ? p : "";
              cp = b.nextPos;
              continue;
            }
          }
          if (opName === "contains") {
            isChain = true;
            const b = this.extractBlock(src, opEnd);
            if (b) {
              const p = this.process(b.content);
              const idx = curVal.indexOf(p);
              curVal = idx !== -1 ? String(idx) : "";
              cp = b.nextPos;
              continue;
            }
          }
          if (opName === "slice") {
            isChain = true;
            const b1 = this.extractBlock(src, opEnd);
            if (b1) {
              let s = parseInt(this.process(b1.content), 10);
              let e = curVal.length;
              let pEnd = b1.nextPos;
              const b2 = this.extractBlock(src, b1.nextPos);
              if (b2) {
                e = parseInt(this.process(b2.content), 10);
                pEnd = b2.nextPos;
              }
              curVal = curVal.slice(s, e);
              cp = pEnd;
              continue;
            }
          }
          if (opName === "replace") {
            isChain = true;
            const b1 = this.extractBlock(src, opEnd);
            if (b1) {
              const b2 = this.extractBlock(src, b1.nextPos);
              if (b2) {
                const p = this.process(b1.content);
                const r = this.process(b2.content);
                curVal = curVal.replace(p, r);
                if (varName) this.setVar(varName, curVal);
                cp = b2.nextPos;
                continue;
              }
            }
          }

          break;
        }

        if (isChain) {
          out += curVal;
          i = cp;
          continue;
        }
      }

      out += src[i];
      i++;
    }

    return out;
  }
}

export function runLegacy(code) {
  const runner = new LegacyPapagaio();
  return runner.process(code);
}

export default LegacyPapagaio;
