// papagaio.md.js
// Single-file JavaScript implementation of the papagaio text processor + markdown extraction.
//
// - Runs entirely in JS (no WASM)
// - Applies the full papagaio template processor (patterns, $eval, modifiers, escaping)
// - Parses Markdown into JS objects (sections, lists, tables, paragraphs)
// - Does NOT execute fenced code blocks; they are treated as normal text
// - JS execution happens only via $eval{...}
//
// Usage (browser):
//   const md = papagaio_md_to_object(markdownText);
//   // md is also available on window.md
//
// Usage (Node):
//   const { papagaio_md_to_object } = require('./papagaio.md.js');

(function (root) {
  const PAPAGAIO_ESCAPED_SIGIL = "\x01";

  const PAPAGAIO_DEFAULT_PATTERN = "pattern";
  const PAPAGAIO_DEFAULT_EVAL = "eval";
  const PAPAGAIO_DEFAULT_BLOCK = "recursive";
  const PAPAGAIO_DEFAULT_BLOCKSEQ = "sequential";
  const PAPAGAIO_DEFAULT_REGEX = "regex";
  const PAPAGAIO_DEFAULT_OPTIONS = "options";
  const PAPAGAIO_DEFAULT_OPTIONAL = "optional";

  function make_default_symbols(sigil = "$", open = "{", close = "}") {
    return {
      sigil,
      open,
      close,
      pattern: PAPAGAIO_DEFAULT_PATTERN,
      eval: PAPAGAIO_DEFAULT_EVAL,
      block: PAPAGAIO_DEFAULT_BLOCK,
      blockseq: PAPAGAIO_DEFAULT_BLOCKSEQ,
      regex: PAPAGAIO_DEFAULT_REGEX,
      options: PAPAGAIO_DEFAULT_OPTIONS,
      optional: PAPAGAIO_DEFAULT_OPTIONAL,
    };
  }

  function trim_view(str) {
    return str.replace(/^\s+|\s+$/g, "");
  }

  function prepare_input(input, sym) {
    if (input == null) return null;
    const sigil = sym.sigil || "$";
    let out = "";
    for (let i = 0; i < input.length; i++) {
      const c = input[i];
      if (
        c === "\\" &&
        sigil &&
        i + sigil.length < input.length &&
        input.slice(i + 1, i + 1 + sigil.length) === sigil
      ) {
        out += PAPAGAIO_ESCAPED_SIGIL;
        i += sigil.length;
        continue;
      }
      out += c;
    }
    return out;
  }

  function restore_escaped(input, sym) {
    if (input == null) return null;
    const sigil = sym.sigil || "$";
    return input.split(PAPAGAIO_ESCAPED_SIGIL).join(sigil);
  }

  function extract_block(src, pos, open, close) {
    if (!src || pos >= src.length) return { next: pos, content: "" };
    if (open === close) {
      if (!src.startsWith(open, pos)) return { next: pos, content: "" };
      const start = pos + open.length;
      const idx = src.indexOf(close, start);
      if (idx < 0) {
        return { next: src.length, content: src.slice(start) };
      }
      return { next: idx + close.length, content: src.slice(start, idx) };
    }
    if (!src.startsWith(open, pos)) return { next: pos, content: "" };
    let cur = pos + open.length;
    let depth = 1;
    const start = cur;
    while (cur < src.length && depth > 0) {
      if (src.startsWith(open, cur)) {
        depth++;
        cur += open.length;
      } else if (src.startsWith(close, cur)) {
        depth--;
        if (depth === 0) {
          return { next: cur + close.length, content: src.slice(start, cur) };
        }
        cur += close.length;
      } else {
        cur++;
      }
    }
    return { next: src.length, content: src.slice(start) };
  }

  function read_quoted(str, idx) {
    const quote = str[idx];
    if (quote !== '"' && quote !== "'") return null;
    let i = idx + 1;
    while (i < str.length) {
      if (str[i] === "\\") {
        i += 2;
        continue;
      }
      if (str[i] === quote) {
        return { raw: str.slice(idx, i + 1), end: i + 1 };
      }
      i++;
    }
    return null;
  }

  function parse_literal(lit) {
    try {
      return JSON.parse(lit);
    } catch (e) {
      return lit.replace(/^['"]|['"]$/g, "");
    }
  }

  const TOK_WS = 1;
  const TOK_LITERAL = 2;
  const TOK_VAR = 3;
  const TOK_BLOCK = 4;
  const TOK_BLOCKSEQ = 5;
  const TOK_OPTIONS = 6;
  const TOK_OPTIONAL_LIT = 7;

  const MOD_NONE = 0;
  const MOD_ALIASES = 1;
  const MOD_OPTIONAL = 2;
  const MOD_STARTS = 3;
  const MOD_ENDS = 4;
  const MOD_INT = 5;
  const MOD_FLOAT = 6;
  const MOD_NUMBER = 7;
  const MOD_UPPER = 8;
  const MOD_LOWER = 9;
  const MOD_CAPITALIZED = 10;
  const MOD_WORD = 11;
  const MOD_IDENTIFIER = 12;
  const MOD_PATH = 13;
  const MOD_HEX = 14;
  const MOD_BINARY = 15;
  const MOD_PERCENT = 16;

  function skip_ws(src, pos) {
    while (pos < src.length && /\s/.test(src[pos])) pos++;
    return pos;
  }

  function parse_pattern_ex(pat, sym) {
    const p = { t: [], sym };
    const n = pat.length;
    let i = 0;
    const sigil = sym.sigil || "$";

    while (i < n) {
      const t = {
        type: 0,
        optional: 0,
        all_opt: 0,
        next_sig: -1,
        modifier: MOD_NONE,
        var: null,
        value: "",
        open: sym.open,
        close: sym.close,
        alts: [],
      };

      if (/\s/.test(pat[i])) {
        while (i < n && /\s/.test(pat[i])) i++;
        t.type = TOK_WS;
        p.t.push(t);
        continue;
      }

      if (pat.startsWith(sigil, i)) {
        i += sigil.length;

        const rem = pat.slice(i);
        const words = [sym.block, sym.blockseq, sym.options, sym.optional, sym.regex].filter(Boolean);
        let found = null;
        let foundLen = 0;
        for (const w of words) {
          if (w && rem.startsWith(w) && w.length > foundLen) {
            found = w;
            foundLen = w.length;
          }
        }
        if (found) {
          i += foundLen;
          if (found === sym.block) t.type = TOK_BLOCK;
          else if (found === sym.blockseq) t.type = TOK_BLOCKSEQ;
          else if (found === sym.options) t.type = TOK_OPTIONS;
          else if (found === sym.optional) t.type = TOK_OPTIONAL_LIT;
          else t.type = TOK_LITERAL;

          const blk = extract_block(pat, i, sym.open, sym.close);
          i = blk.next;
          const trimmed = trim_view(blk.content);
          if (t.type === TOK_BLOCK || t.type === TOK_BLOCKSEQ) {
            t.open = sym.open;
            t.close = sym.close;
            t.value = trimmed;
          } else if (t.type === TOK_OPTIONS) {
            t.alts = trimmed.split(/\s*,\s*/).map((x) => x.trim()).filter(Boolean);
          } else if (t.type === TOK_OPTIONAL_LIT) {
            t.value = trimmed;
            t.optional = 1;
          } else {
            t.value = trimmed;
          }

          p.t.push(t);
          continue;
        }

        let varName = "";
        while (i < n && /[A-Za-z0-9_]/.test(pat[i])) {
          varName += pat[i++];
        }
        t.type = TOK_VAR;
        t.var = varName;

        const modMap = {
          int: MOD_INT,
          float: MOD_FLOAT,
          number: MOD_NUMBER,
          upper: MOD_UPPER,
          lower: MOD_LOWER,
          capitalized: MOD_CAPITALIZED,
          word: MOD_WORD,
          identifier: MOD_IDENTIFIER,
          path: MOD_PATH,
          hex: MOD_HEX,
          binary: MOD_BINARY,
          percent: MOD_PERCENT,
        };
        for (const [k, v] of Object.entries(modMap)) {
          if (pat.startsWith(`$${k}`, i)) {
            t.modifier = v;
            i += k.length + 1;
            break;
          }
        }

        if (pat.startsWith('$aliases{', i)) {
          i += '$aliases{'.length;
          const end = pat.indexOf('}', i);
          if (end >= 0) {
            t.modifier = MOD_ALIASES;
            t.alts = pat.slice(i, end).split(/\s*,\s*/).map((s) => s.trim()).filter(Boolean);
            i = end + 1;
          }
        } else if (pat.startsWith('$optional{', i)) {
          i += '$optional{'.length;
          const end = pat.indexOf('}', i);
          if (end >= 0) {
            t.modifier = MOD_OPTIONAL;
            t.value = pat.slice(i, end);
            i = end + 1;
          }
        } else if (pat.startsWith('$starts{', i)) {
          i += '$starts{'.length;
          const end = pat.indexOf('}', i);
          if (end >= 0) {
            t.modifier = MOD_STARTS;
            t.value = pat.slice(i, end);
            i = end + 1;
          }
        } else if (pat.startsWith('$ends{', i)) {
          i += '$ends{'.length;
          const end = pat.indexOf('}', i);
          if (end >= 0) {
            t.modifier = MOD_ENDS;
            t.value = pat.slice(i, end);
            i = end + 1;
          }
        }

        if (pat[i] === '?') {
          t.optional = 1;
          i++;
        }

        p.t.push(t);
        continue;
      }

      let l = i;
      while (i < n && !/\s/.test(pat[i]) && !pat.startsWith(sigil, i)) i++;
      t.type = TOK_LITERAL;
      t.value = pat.slice(l, i);
      p.t.push(t);
    }

    for (let a = 0; a < p.t.length; a++) {
      const tok = p.t[a];
      tok.next_sig = -1;
      for (let b = a + 1; b < p.t.length; b++) {
        if (p.t[b].type !== TOK_WS) {
          tok.next_sig = b;
          break;
        }
      }

      let all = true;
      for (let b = a + 1; b < p.t.length; b++) {
        if (p.t[b].type === TOK_WS) continue;
        if (!p.t[b].optional) {
          all = false;
          break;
        }
      }
      tok.all_opt = all ? 1 : 0;
    }

    for (let a = 0; a < p.t.length; a++) {
      const tok = p.t[a];
      if (tok.type !== TOK_WS) continue;
      const ns = tok.next_sig;
      if (ns >= 0 && p.t[ns].optional) {
        tok.optional = 1;
        continue;
      }
      for (let b = a - 1; b >= 0; b--) {
        if (p.t[b].type === TOK_WS) continue;
        if (p.t[b].optional) tok.optional = 1;
        break;
      }
    }

    return p;
  }

  function match_pattern(src, p, start) {
    const m = { cap: [], start, end: start, src };
    let pos = start;

    for (let i = 0; i < p.t.length; i++) {
      const t = p.t[i];

      if (t.type === TOK_WS) {
        if (!/\s/.test(src[pos])) {
          if (!t.all_opt && !t.optional) return null;
          continue;
        }
        pos = skip_ws(src, pos);
        continue;
      }

      if (t.type === TOK_LITERAL) {
        if (!src.startsWith(t.value, pos)) return null;
        pos += t.value.length;
        continue;
      }

      const nx = t.next_sig >= 0 ? p.t[t.next_sig] : null;

      if (t.type === TOK_VAR) {
        if (i === 0 || p.t[i - 1].type !== TOK_WS) {
          pos = skip_ws(src, pos);
        }
        const s = pos;

        const pushCapture = (val) => {
          m.cap.push({ name: t.var, value: val });
        };

        if (t.modifier === MOD_ALIASES) {
          let matched = false;
          for (const alt of t.alts) {
            if (src.startsWith(alt, pos)) {
              pos += alt.length;
              matched = true;
              break;
            }
          }
          if (!matched) {
            if (!t.optional) return null;
            pushCapture("");
            continue;
          }
          pushCapture(src.slice(s, pos));
          continue;
        }

        if (t.modifier === MOD_OPTIONAL) {
          if (src.startsWith(t.value, pos)) {
            pos += t.value.length;
          }
          pushCapture(src.slice(s, pos));
          continue;
        }

        if (t.modifier === MOD_STARTS) {
          if (!src.startsWith(t.value, pos)) {
            if (!t.optional) return null;
            pushCapture("");
            continue;
          }
        }

        if (nx && (nx.type === TOK_LITERAL || nx.type === TOK_BLOCK || nx.type === TOK_BLOCKSEQ || nx.type === TOK_OPTIONS)) {
          while (pos < src.length) {
            if (src[pos] === '\n') break;
            if (nx.type === TOK_LITERAL && src.startsWith(nx.value, pos)) break;
            if ((nx.type === TOK_BLOCK || nx.type === TOK_BLOCKSEQ) && src.startsWith(nx.open, pos)) break;

            let valid = true;
            const c = src[pos];

            const validateChar = (cond) => {
              if (!cond) valid = false;
            };

            if (t.modifier === MOD_INT) {
              validateChar(/[0-9]/.test(c) || (pos === s && c === '-'));
            } else if (t.modifier === MOD_FLOAT || t.modifier === MOD_NUMBER) {
              validateChar(/[0-9.]/.test(c) || (pos === s && c === '-'));
            } else if (t.modifier === MOD_UPPER) {
              validateChar(/[A-Z]/.test(c));
            } else if (t.modifier === MOD_LOWER) {
              validateChar(/[a-z]/.test(c));
            } else if (t.modifier === MOD_CAPITALIZED) {
              if (pos === s) validateChar(/[A-Z]/.test(c));
              else validateChar(/[a-z]/.test(c));
            } else if (t.modifier === MOD_WORD) {
              validateChar(/[A-Za-z]/.test(c));
            } else if (t.modifier === MOD_IDENTIFIER) {
              validateChar(/[A-Za-z0-9_]/.test(c));
              if (pos === s) validateChar(!/[0-9]/.test(c));
            } else if (t.modifier === MOD_HEX) {
              validateChar(/[0-9A-Fa-fxX]/.test(c));
            } else if (t.modifier === MOD_PATH) {
              validateChar(!/\s/.test(c));
            } else if (t.modifier === MOD_BINARY) {
              validateChar(/[01bB]/.test(c));
            } else if (t.modifier === MOD_PERCENT) {
              validateChar(/[0-9.%]/.test(c) || (pos === s && c === '-'));
            }

            if (!valid) break;

            pos++;

            if (t.modifier === MOD_ENDS && t.value.length > 0) {
              if (pos - s >= t.value.length && src.slice(pos - t.value.length, pos) === t.value) break;
            }
          }

          if (t.modifier === MOD_ENDS && t.value.length > 0) {
            if (pos - s < t.value.length || src.slice(pos - t.value.length, pos) !== t.value) {
              if (!t.optional) return null;
              pushCapture("");
              continue;
            }
          }

          let end = pos;
          while (end > s && /\s/.test(src[end - 1])) end--;
          if (end === s) {
            if (!t.optional) return null;
            pushCapture("");
            continue;
          }
          pushCapture(src.slice(s, end));
          pos = end;
          continue;
        }

        while (pos < src.length) {
          if (nx && /\s/.test(src[pos])) break;
          if (nx) {
            if (nx.type === TOK_LITERAL && src.startsWith(nx.value, pos)) break;
            if ((nx.type === TOK_BLOCK || nx.type === TOK_BLOCKSEQ) && src.startsWith(nx.open, pos)) break;
          } else if (/\s/.test(src[pos])) break;

          let valid = true;
          const c = src[pos];
          const validateChar = (cond) => {
            if (!cond) valid = false;
          };

          if (t.modifier === MOD_INT) {
            validateChar(/[0-9]/.test(c) || (pos === s && c === '-'));
          } else if (t.modifier === MOD_FLOAT || t.modifier === MOD_NUMBER) {
            validateChar(/[0-9.]/.test(c) || (pos === s && c === '-'));
          } else if (t.modifier === MOD_UPPER) {
            validateChar(/[A-Z]/.test(c));
          } else if (t.modifier === MOD_LOWER) {
            validateChar(/[a-z]/.test(c));
          } else if (t.modifier === MOD_CAPITALIZED) {
            if (pos === s) validateChar(/[A-Z]/.test(c));
            else validateChar(/[a-z]/.test(c));
          } else if (t.modifier === MOD_WORD) {
            validateChar(/[A-Za-z]/.test(c));
          } else if (t.modifier === MOD_IDENTIFIER) {
            validateChar(/[A-Za-z0-9_]/.test(c));
            if (pos === s) validateChar(!/[0-9]/.test(c));
          } else if (t.modifier === MOD_HEX) {
            validateChar(/[0-9A-Fa-fxX]/.test(c));
          } else if (t.modifier === MOD_PATH) {
            validateChar(!/\s/.test(c));
          } else if (t.modifier === MOD_BINARY) {
            validateChar(/[01bB]/.test(c));
          } else if (t.modifier === MOD_PERCENT) {
            validateChar(/[0-9.%]/.test(c) || (pos === s && c === '-'));
          }

          if (!valid) break;

          pos++;

          if (t.modifier === MOD_ENDS && t.value.length > 0) {
            if (pos - s >= t.value.length && src.slice(pos - t.value.length, pos) === t.value) break;
          }
        }

        if (t.modifier === MOD_ENDS && t.value.length > 0) {
          if (pos - s < t.value.length || src.slice(pos - t.value.length, pos) !== t.value) {
            if (!t.optional) return null;
            pushCapture("");
            continue;
          }
        }

        if (pos === s) {
          if (!t.optional) return null;
          pushCapture("");
          continue;
        }

        pushCapture(src.slice(s, pos));
        continue;
      }

      if (t.type === TOK_BLOCK) {
        if (!src.startsWith(t.open, pos)) {
          if (!t.optional) return null;
          pushCapture("");
          continue;
        }
        const blk = extract_block(src, pos, t.open, t.close);
        pos = blk.next;
        pushCapture(blk.content);
        continue;
      }

      if (t.type === TOK_OPTIONS) {
        let matched = false;
        for (const alt of t.alts) {
          if (src.startsWith(alt, pos)) {
            pos += alt.length;
            matched = true;
            break;
          }
        }
        if (!matched && !t.optional) return null;
        continue;
      }

      if (t.type === TOK_OPTIONAL_LIT) {
        if (src.startsWith(t.value, pos)) {
          pos += t.value.length;
        }
        continue;
      }

      if (t.type === TOK_BLOCKSEQ) {
        if (!src.startsWith(t.open, pos)) {
          if (!t.optional) return null;
          pushCapture("");
          continue;
        }
        let buf = "";
        let blocks = 0;
        while (src.startsWith(t.open, pos)) {
          const blk = extract_block(src, pos, t.open, t.close);
          if (blocks > 0) buf += " ";
          buf += blk.content;
          blocks++;
          pos = skip_ws(src, blk.next);
        }
        if (blocks === 0) {
          if (!t.optional) return null;
          pushCapture("");
          continue;
        }
        pushCapture(buf);
        continue;
      }
    }

    m.end = pos;
    return m;
  }

  function apply_eval_placeholder(str, idx, sym, match) {
    const sigil = sym.sigil;
    const evalKey = sym.eval;
    if (!str.startsWith(sigil, idx)) return null;
    let pos = idx + sigil.length;
    if (!str.startsWith(evalKey, pos)) return null;
    pos += evalKey.length;
    while (pos < str.length && /\s/.test(str[pos])) pos++;
    const blk = extract_block(str, pos, sym.open, sym.close);
    if (blk.next === pos) return null;
    const trimmed = trim_view(blk.content);
    let result = "";
    try {
      const fn = new Function('match', trimmed);
      const res = fn(match);
      if (res !== undefined && res !== null) result = String(res);
    } catch (e) {
      result = `error: ${e}`;
    }
    return { result, next: blk.next };
  }

  function apply_replacement_ex(rep, m, sym, match) {
    let out = "";
    const sigil = sym.sigil;
    let i = 0;
    while (i < rep.length) {
      if (rep.startsWith(sigil, i)) {
        const evalRes = apply_eval_placeholder(rep, i, sym, match);
        if (evalRes) {
          out += evalRes.result;
          i = evalRes.next;
          continue;
        }
        let nameStart = i + sigil.length;
        let nameEnd = nameStart;
        while (nameEnd < rep.length && /[A-Za-z0-9_]/.test(rep[nameEnd])) nameEnd++;
        const name = rep.slice(nameStart, nameEnd);
        let found = false;
        if (name.length > 0) {
          for (const cap of m.cap || []) {
            if (cap.name === name) {
              out += cap.value;
              found = true;
              break;
            }
          }
        }
        if (!found) {
          out += sigil + name;
        }
        i = nameEnd;
        continue;
      }
      out += rep[i++];
    }
    return out;
  }

  function apply_patterns(src, pairs, sym) {
    if (!src) return null;
    let current = src;
    for (const pair of pairs) {
      const pat = parse_pattern_ex(pair.m, sym);
      let out = "";
      let pos = 0;
      let matched = false;
      while (pos < current.length) {
        const m = match_pattern(current, pat, pos);
        if (m) {
          const rep = apply_replacement_ex(pair.r, m, sym, current.slice(m.start, m.end));
          out += rep;
          pos = m.end;
          matched = true;
          continue;
        }
        out += current[pos++];
      }
      if (matched) {
        current = out;
      }
    }
    return current;
  }

  function extract_nested(src, sym) {
    const pairs = [];
    let out = "";
    let i = 0;
    while (i < src.length) {
      if (src.startsWith(sym.sigil, i)) {
        const openIdx = i + sym.sigil.length;
        if (src.startsWith(sym.pattern, openIdx)) {
          let pos = openIdx + sym.pattern.length;
          while (pos < src.length && /\s/.test(src[pos])) pos++;
          const blk1 = extract_block(src, pos, sym.open, sym.close);
          if (blk1.next === pos) {
            out += src[i++];
            continue;
          }
          pos = blk1.next;
          while (pos < src.length && /\s/.test(src[pos])) pos++;
          const blk2 = extract_block(src, pos, sym.open, sym.close);
          if (blk2.next === pos) {
            out += src[i++];
            continue;
          }
          pairs.push({ m: blk1.content, r: blk2.content });
          i = blk2.next;
          continue;
        }
      }
      out += src[i++];
    }
    return { clean: out, pairs };
  }

  function extract_evals(src, sym) {
    const evals = [];
    let out = "";
    let i = 0;
    while (i < src.length) {
      if (src.startsWith(sym.sigil, i) && src.startsWith(sym.eval, i + sym.sigil.length)) {
        let pos = i + sym.sigil.length + sym.eval.length;
        while (pos < src.length && /\s/.test(src[pos])) pos++;
        const blk = extract_block(src, pos, sym.open, sym.close);
        if (blk.next === pos) {
          out += src[i++];
          continue;
        }
        const trimmed = trim_view(blk.content);
        const placeholder = `__E${evals.length}__`;
        evals.push(trimmed);
        out += placeholder;
        i = blk.next;
        continue;
      }
      out += src[i++];
    }
    return { ph: out, evals };
  }

  function apply_evals(ph, evals) {
    let current = ph;
    for (let i = evals.length - 1; i >= 0; i--) {
      const placeholder = `__E${i}__`;
      const code = evals[i];
      let result = "";
      try {
        const fn = new Function('match', code);
        const res = fn('');
        if (res !== undefined && res !== null) result = String(res);
      } catch (e) {
        result = `error: ${e}`;
      }
      current = current.split(placeholder).join(result);
    }
    return current;
  }

  function papagaio_process_text(input) {
    const sym = make_default_symbols();
    let src = prepare_input(input, sym);
    if (!src) return null;

    while (true) {
      const nested = extract_nested(src, sym);
      const clean = nested.clean;
      const pairs = nested.pairs;

      const evalExtract = extract_evals(clean, sym);
      let proc = apply_evals(evalExtract.ph, evalExtract.evals);

      if (pairs.length > 0) {
        while (true) {
          const next = apply_patterns(proc, pairs, sym);
          if (next === proc) break;
          proc = next;
        }
      }

      const restored = restore_escaped(proc, sym);
      if (restored === src) {
        src = restored;
        break;
      }
      src = restored;
    }

    const restored = restore_escaped(src, sym);
    if (typeof window !== 'undefined') {
      window.papagaio = window.papagaio || {};
      window.papagaio.content = restored;
    }
    return restored;
  }

  function parseTableRow(line) {
    const trimmed = line.trim();
    const noPipes = trimmed.replace(/^\|/, "").replace(/\|$/, "");
    return noPipes.split('|').map((c) => c.trim());
  }

  function papagaio_md_to_object(markdown) {
    const processed = papagaio_process_text(markdown);
    const lines = processed.split(/\r?\n/);

    const md = {
      content: [],
    };
    let section = 'content';
    let inCodeFence = false;
    let codeFenceMarker = '';
    let codeFenceLines = null;
    let listBuffer = null;
    let tableState = null;

    const ensureSection = (name) => {
      if (!md[name]) md[name] = [];
      section = name;
    };

    const closeList = () => {
      if (listBuffer) {
        md[section].push(listBuffer);
        listBuffer = null;
      }
    };
    const closeTable = () => {
      if (tableState) {
        md[section].push(tableState.rows);
        tableState = null;
      }
    };

    for (let i = 0; i < lines.length; i++) {
      const raw = lines[i];
      const line = raw.replace(/\r$/, "");
      const trimmed = line.trim();

      if (inCodeFence) {
        codeFenceLines.push(line);
        if (trimmed.startsWith(codeFenceMarker)) {
          inCodeFence = false;
          codeFenceMarker = '';
          md[section].push(codeFenceLines.join('\n'));
          codeFenceLines = null;
        }
        continue;
      }

      if (trimmed.startsWith('```')) {
        closeList();
        closeTable();
        inCodeFence = true;
        codeFenceMarker = trimmed.slice(0, 3);
        codeFenceLines = [line];
        continue;
      }

      const h = line.match(/^\s*#\s*(.*)$/);
      if (h) {
        closeList();
        closeTable();
        ensureSection(h[1].trim() || 'content');
        continue;
      }

      const listMatch = line.match(/^\s*-\s*(.*)$/);
      if (listMatch) {
        closeTable();
        if (!listBuffer) listBuffer = [];
        listBuffer.push(listMatch[1]);
        continue;
      }

      // Table detection: header + separator
      const isTableHeader = line.includes('|');
      const nextLine = i + 1 < lines.length ? lines[i + 1].trim() : '';
      const isTableSeparator = /^\s*(\|\s*)?[-:]+(?:\s*\|\s*[-:]+)*\s*(\|\s*)?$/.test(nextLine);

      if (isTableHeader && isTableSeparator) {
        closeList();
        closeTable();
        const headers = parseTableRow(line);
        tableState = { headers, rows: [] };
        i++; // skip separator
        continue;
      }

      if (tableState && line.includes('|')) {
        const cells = parseTableRow(line);
        const row = {};
        for (let ci = 0; ci < tableState.headers.length; ci++) {
          const key = tableState.headers[ci] || `col${ci}`;
          row[key] = cells[ci] || '';
        }
        tableState.rows.push(row);
        continue;
      }

      if (/^\s*$/.test(line)) {
        closeList();
        closeTable();
        continue;
      }

      closeList();
      closeTable();
      md[section].push(trimmed);
    }

    closeList();
    closeTable();

    if (inCodeFence && codeFenceLines) {
      md[section].push(codeFenceLines.join('\n'));
      inCodeFence = false;
      codeFenceLines = null;
    }

    if (typeof window !== 'undefined') {
      window.md = md;
    }

    return md;
  }

  function papagaio_md_to_markdown(md) {
    if (!md || typeof md !== 'object') return '';

    const lines = [];

    const renderSection = (name, items) => {
      if (!items || !items.length) return;
      if (name !== 'content') {
        lines.push(`# ${name}`);
      }

      const pushLine = (line) => {
        lines.push(line);
      };

      for (const item of items) {
        if (typeof item === 'string') {
          pushLine(item);
          continue;
        }

        if (Array.isArray(item)) {
          const first = item[0];
          if (first && typeof first === 'object' && !Array.isArray(first)) {
            // Table
            const headers = Object.keys(first);
            if (headers.length === 0) continue;
            pushLine('| ' + headers.join(' | ') + ' |');
            pushLine('| ' + headers.map(() => '---').join(' | ') + ' |');
            for (const row of item) {
              const cells = headers.map((h) => (row[h] !== undefined ? String(row[h]) : ''));
              pushLine('| ' + cells.join(' | ') + ' |');
            }
            continue;
          }

          // List
          for (const line of item) {
            pushLine(`- ${line}`);
          }
          continue;
        }

        // Fallback: stringify
        pushLine(String(item));
      }

      lines.push('');
    };

    renderSection('content', md.content);

    for (const [section, items] of Object.entries(md)) {
      if (section === 'content') continue;
      renderSection(section, items);
    }

    // Trim trailing blank line
    while (lines.length > 0 && lines[lines.length - 1].trim() === '') {
      lines.pop();
    }

    return lines.join('\n');
  }

  const papagaioMd = {
    papagaio_process_text,
    papagaio_md_to_object,
    papagaio_md_to_markdown,
  };

  if (typeof module !== 'undefined' && module.exports) {
    module.exports = papagaioMd;
  }

  if (typeof root !== 'undefined') {
    root.papagaio_process_text = papagaio_process_text;
    root.papagaio_md_to_object = papagaio_md_to_object;
    root.papagaio_md_to_markdown = papagaio_md_to_markdown;
    root.papagaio = root.papagaio || {};
    root.papagaio.md = papagaioMd;
  }
})(typeof window !== 'undefined' ? window : typeof global !== 'undefined' ? global : this);

// ESM default export (when imported via `import papagaio from './papagaio.js'`).
// In non-ESM environments, the object is still available as `globalThis.papagaio.md`.
const _papagaioExport = (typeof globalThis !== 'undefined' && globalThis.papagaio && globalThis.papagaio.md)
  ? globalThis.papagaio.md
  : {};

export default _papagaioExport;
