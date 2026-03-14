/*
 * papagaio.md — Obsidian Plugin
 * Compatible with desktop & Android (mobile).
 *
 * Command: "Process with papagaio"
 *
 * Generates two files next to the active note:
 *   nome.processed.md  — papagaio_process_text → md_to_object → md_to_markdown (MD recompilado)
 *   nome.output.md     — console.log / stdout capturado durante o processamento
 */

// ─── papagaio.js (inlined) ───────────────────────────────────────────────────

const PAPAGAIO_ESCAPED_SIGIL = "\x01";
function make_default_symbols(sigil = "$", open = "{", close = "}") {
  return { sigil, open, close, pattern: "pattern", eval: "eval", block: "recursive", blockseq: "sequential", regex: "regex", options: "options", optional: "optional" };
}
function trim_view(str) { return str.replace(/^\s+|\s+$/g, ""); }
function prepare_input(input, sym) {
  if (input == null) return null;
  const sigil = sym.sigil || "$"; let out = "";
  for (let i = 0; i < input.length; i++) {
    const c = input[i];
    if (c === "\\" && sigil && i + sigil.length < input.length && input.slice(i + 1, i + 1 + sigil.length) === sigil) { out += PAPAGAIO_ESCAPED_SIGIL; i += sigil.length; continue; }
    out += c;
  }
  return out;
}
function restore_escaped(input, sym) {
  if (input == null) return null;
  return input.split(PAPAGAIO_ESCAPED_SIGIL).join(sym.sigil || "$");
}
function extract_block(src, pos, open, close) {
  if (!src || pos >= src.length) return { next: pos, content: "" };
  if (open === close) {
    if (!src.startsWith(open, pos)) return { next: pos, content: "" };
    const start = pos + open.length; const idx = src.indexOf(close, start);
    if (idx < 0) return { next: src.length, content: src.slice(start) };
    return { next: idx + close.length, content: src.slice(start, idx) };
  }
  if (!src.startsWith(open, pos)) return { next: pos, content: "" };
  let cur = pos + open.length, depth = 1; const start = cur;
  while (cur < src.length && depth > 0) {
    if (src.startsWith(open, cur)) { depth++; cur += open.length; }
    else if (src.startsWith(close, cur)) { depth--; if (depth === 0) return { next: cur + close.length, content: src.slice(start, cur) }; cur += close.length; }
    else cur++;
  }
  return { next: src.length, content: src.slice(start) };
}
const TOK_WS=1,TOK_LITERAL=2,TOK_VAR=3,TOK_BLOCK=4,TOK_BLOCKSEQ=5,TOK_OPTIONS=6,TOK_OPTIONAL_LIT=7;
const MOD_NONE=0,MOD_ALIASES=1,MOD_OPTIONAL=2,MOD_STARTS=3,MOD_ENDS=4,MOD_INT=5,MOD_FLOAT=6,MOD_NUMBER=7,MOD_UPPER=8,MOD_LOWER=9,MOD_CAPITALIZED=10,MOD_WORD=11,MOD_IDENTIFIER=12,MOD_PATH=13,MOD_HEX=14,MOD_BINARY=15,MOD_PERCENT=16;
function skip_ws(src, pos) { while (pos < src.length && /\s/.test(src[pos])) pos++; return pos; }
function parse_pattern_ex(pat, sym) {
  const p = { t: [], sym }; const n = pat.length; let i = 0; const sigil = sym.sigil || "$";
  while (i < n) {
    const t = { type:0, optional:0, all_opt:0, next_sig:-1, modifier:MOD_NONE, var:null, value:"", open:sym.open, close:sym.close, alts:[] };
    if (/\s/.test(pat[i])) { while (i < n && /\s/.test(pat[i])) i++; t.type = TOK_WS; p.t.push(t); continue; }
    if (pat.startsWith(sigil, i)) {
      i += sigil.length;
      const rem = pat.slice(i);
      const words = [sym.block, sym.blockseq, sym.options, sym.optional, sym.regex].filter(Boolean);
      let found = null, foundLen = 0;
      for (const w of words) { if (w && rem.startsWith(w) && w.length > foundLen) { found = w; foundLen = w.length; } }
      if (found) {
        i += foundLen;
        if (found === sym.block) t.type = TOK_BLOCK; else if (found === sym.blockseq) t.type = TOK_BLOCKSEQ; else if (found === sym.options) t.type = TOK_OPTIONS; else if (found === sym.optional) t.type = TOK_OPTIONAL_LIT; else t.type = TOK_LITERAL;
        const blk = extract_block(pat, i, sym.open, sym.close); i = blk.next; const trimmed = trim_view(blk.content);
        if (t.type === TOK_BLOCK || t.type === TOK_BLOCKSEQ) { t.open = sym.open; t.close = sym.close; t.value = trimmed; }
        else if (t.type === TOK_OPTIONS) { t.alts = trimmed.split(/\s*,\s*/).map(x => x.trim()).filter(Boolean); }
        else if (t.type === TOK_OPTIONAL_LIT) { t.value = trimmed; t.optional = 1; }
        else t.value = trimmed;
        p.t.push(t); continue;
      }
      let varName = ""; while (i < n && /[A-Za-z0-9_]/.test(pat[i])) varName += pat[i++];
      t.type = TOK_VAR; t.var = varName;
      const modMap = { int:MOD_INT, float:MOD_FLOAT, number:MOD_NUMBER, upper:MOD_UPPER, lower:MOD_LOWER, capitalized:MOD_CAPITALIZED, word:MOD_WORD, identifier:MOD_IDENTIFIER, path:MOD_PATH, hex:MOD_HEX, binary:MOD_BINARY, percent:MOD_PERCENT };
      for (const [k, v] of Object.entries(modMap)) { if (pat.startsWith(`$${k}`, i)) { t.modifier = v; i += k.length + 1; break; } }
      if (pat.startsWith('$aliases{', i)) { i += 9; const end = pat.indexOf('}', i); if (end >= 0) { t.modifier = MOD_ALIASES; t.alts = pat.slice(i, end).split(/\s*,\s*/).map(s=>s.trim()).filter(Boolean); i = end + 1; } }
      else if (pat.startsWith('$optional{', i)) { i += 10; const end = pat.indexOf('}', i); if (end >= 0) { t.modifier = MOD_OPTIONAL; t.value = pat.slice(i, end); i = end + 1; } }
      else if (pat.startsWith('$starts{', i)) { i += 8; const end = pat.indexOf('}', i); if (end >= 0) { t.modifier = MOD_STARTS; t.value = pat.slice(i, end); i = end + 1; } }
      else if (pat.startsWith('$ends{', i)) { i += 7; const end = pat.indexOf('}', i); if (end >= 0) { t.modifier = MOD_ENDS; t.value = pat.slice(i, end); i = end + 1; } }
      p.t.push(t); continue;
    }
    let lit = ""; while (i < n && !pat.startsWith(sigil, i) && !/\s/.test(pat[i])) lit += pat[i++];
    t.type = TOK_LITERAL; t.value = lit; p.t.push(t);
  }
  let has_non_opt = false;
  for (const t of p.t) { if (t.type !== TOK_WS && t.modifier !== MOD_OPTIONAL && t.type !== TOK_OPTIONAL_LIT && !t.optional) has_non_opt = true; }
  if (!has_non_opt) for (const t of p.t) t.all_opt = 1;
  for (let j = 0; j < p.t.length; j++) { if (p.t[j].type === TOK_VAR || p.t[j].type === TOK_BLOCK || p.t[j].type === TOK_BLOCKSEQ) { for (let k = j + 1; k < p.t.length; k++) { if (p.t[k].type !== TOK_WS) { if (p.t[k].type === TOK_VAR || p.t[k].type === TOK_LITERAL || p.t[k].type === TOK_OPTIONS) p.t[j].next_sig = k; break; } } } }
  return p;
}
function apply_modifier(val, mod, alts) {
  if (mod === MOD_INT) { const n = parseInt(val, 10); return isNaN(n) ? null : String(n); }
  if (mod === MOD_FLOAT) { const f = parseFloat(val); return isNaN(f) ? null : String(f); }
  if (mod === MOD_NUMBER) { const n2 = Number(val); return isNaN(n2) ? null : String(n2); }
  if (mod === MOD_UPPER) return val.toUpperCase();
  if (mod === MOD_LOWER) return val.toLowerCase();
  if (mod === MOD_CAPITALIZED) return val.charAt(0).toUpperCase() + val.slice(1).toLowerCase();
  if (mod === MOD_WORD) return /^\w+$/.test(val) ? val : null;
  if (mod === MOD_IDENTIFIER) return /^[A-Za-z_]\w*$/.test(val) ? val : null;
  if (mod === MOD_PATH) return /^[^\0]+$/.test(val) ? val : null;
  if (mod === MOD_HEX) return /^[0-9a-fA-F]+$/.test(val) ? val : null;
  if (mod === MOD_BINARY) return /^[01]+$/.test(val) ? val : null;
  if (mod === MOD_PERCENT) { const p = parseFloat(val); return isNaN(p) ? null : String(p); }
  if (mod === MOD_ALIASES && alts) { for (const a of alts) if (val === a) return val; return null; }
  return val;
}
function match_pattern(src, pos, p) {
  const vars = {};
  function match_tok(ti, si) {
    if (ti >= p.t.length) return si;
    const t = p.t[ti];
    if (t.type === TOK_WS) return match_tok(ti + 1, skip_ws(src, si));
    if (t.type === TOK_LITERAL) {
      if (src.startsWith(t.value, si)) return match_tok(ti + 1, si + t.value.length);
      if (t.optional || t.all_opt) return match_tok(ti + 1, si);
      return -1;
    }
    if (t.type === TOK_OPTIONS) {
      for (const alt of t.alts) { if (src.startsWith(alt, si)) { const r = match_tok(ti + 1, si + alt.length); if (r >= 0) return r; } }
      if (t.optional || t.all_opt) return match_tok(ti + 1, si);
      return -1;
    }
    if (t.type === TOK_OPTIONAL_LIT) {
      if (src.startsWith(t.value, si)) { const r = match_tok(ti + 1, si + t.value.length); if (r >= 0) return r; }
      return match_tok(ti + 1, si);
    }
    if (t.type === TOK_VAR) {
      if (t.modifier === MOD_OPTIONAL) { const r1 = try_match_var(t, ti, si); if (r1 >= 0) return r1; return match_tok(ti + 1, si); }
      return try_match_var(t, ti, si);
    }
    if (t.type === TOK_BLOCK) {
      const blk = extract_block(src, si, t.open, t.close);
      if (blk.next === si && !src.startsWith(t.open, si)) return -1;
      vars[t.var || '_block'] = blk.content;
      return match_tok(ti + 1, blk.next);
    }
    if (t.type === TOK_BLOCKSEQ) {
      const results = []; let cur = si;
      while (cur < src.length) { const blk = extract_block(src, cur, t.open, t.close); if (blk.next === cur) break; results.push(blk.content); cur = skip_ws(src, blk.next); }
      vars[t.var || '_seq'] = results;
      return match_tok(ti + 1, cur);
    }
    return -1;
  }
  function try_match_var(t, ti, si) {
    const next = t.next_sig; let end;
    if (next >= 0) {
      const nt = p.t[next];
      if (nt.type === TOK_LITERAL) { const idx = src.indexOf(nt.value, si); if (idx < 0) return -1; end = idx; }
      else end = src.length;
    } else end = src.length;
    const candidate = src.slice(si, end);
    const applied = apply_modifier(candidate, t.modifier, t.alts);
    if (applied === null) return -1;
    vars[t.var] = applied;
    return match_tok(ti + 1, end);
  }
  const endPos = match_tok(0, pos);
  if (endPos < 0) return null;
  return { end: endPos, vars };
}
function apply_patterns(src, pairs, sym) {
  let out = ""; let i = 0;
  while (i < src.length) {
    let matched = false;
    for (const pair of pairs) {
      const res = match_pattern(src, i, pair.pattern);
      if (res && res.end > i) {
        let replacement = pair.template;
        for (const [k, v] of Object.entries(res.vars)) replacement = replacement.split(`$${k}`).join(Array.isArray(v) ? v.join('') : v);
        out += replacement; i = res.end; matched = true; break;
      }
    }
    if (!matched) out += src[i++];
  }
  return out;
}
function extract_nested(src, sym) {
  const pairs = []; let clean = ""; let i = 0; const sigil = sym.sigil || "$";
  while (i < src.length) {
    if (src.startsWith(sigil + sym.pattern, i)) {
      i += sigil.length + sym.pattern.length;
      const patBlk = extract_block(src, i, sym.open, sym.close); i = patBlk.next;
      const tplBlk = extract_block(src, i, sym.open, sym.close); i = tplBlk.next;
      pairs.push({ pattern: parse_pattern_ex(trim_view(patBlk.content), sym), template: trim_view(tplBlk.content) });
    } else clean += src[i++];
  }
  return { clean, pairs };
}
function extract_evals(src, sym) {
  const evals = []; let out = ""; let i = 0; const sigil = sym.sigil || "$";
  while (i < src.length) {
    if (src.startsWith(sigil + sym.eval, i)) {
      i += sigil.length + sym.eval.length;
      const blk = extract_block(src, i, sym.open, sym.close);
      evals.push(trim_view(blk.content)); out += `__E${evals.length - 1}__`; i = blk.next; continue;
    }
    out += src[i++];
  }
  return { ph: out, evals };
}
function apply_evals(ph, evals) {
  let current = ph;
  for (let i = evals.length - 1; i >= 0; i--) {
    let result = "";
    try { const fn = new Function('match', evals[i]); const res = fn(''); if (res !== undefined && res !== null) result = String(res); } catch (e) { result = `error: ${e}`; }
    current = current.split(`__E${i}__`).join(result);
  }
  return current;
}
function papagaio_process_text(input) {
  const sym = make_default_symbols();
  let src = prepare_input(input, sym);
  if (!src) return null;
  while (true) {
    const nested = extract_nested(src, sym);
    const evalExtract = extract_evals(nested.clean, sym);
    let proc = apply_evals(evalExtract.ph, evalExtract.evals);
    if (nested.pairs.length > 0) { while (true) { const next = apply_patterns(proc, nested.pairs, sym); if (next === proc) break; proc = next; } }
    const restored = restore_escaped(proc, sym);
    if (restored === src) { src = restored; break; }
    src = restored;
  }
  return restore_escaped(src, sym);
}
function parseTableRow(line) {
  return line.trim().replace(/^\|/, "").replace(/\|$/, "").split('|').map(c => c.trim());
}
function papagaio_md_to_object(markdown) {
  const processed = papagaio_process_text(markdown);
  const lines = processed.split(/\r?\n/);
  const md = { content: [] };
  let section = 'content', inCodeFence = false, codeFenceMarker = '', codeFenceLines = null, listBuffer = null, tableState = null;
  const ensureSection = name => { if (!md[name]) md[name] = []; section = name; };
  const closeList = () => { if (listBuffer) { md[section].push(listBuffer); listBuffer = null; } };
  const closeTable = () => { if (tableState) { md[section].push(tableState.rows); tableState = null; } };
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].replace(/\r$/, ""); const trimmed = line.trim();
    if (inCodeFence) { codeFenceLines.push(line); if (trimmed.startsWith(codeFenceMarker)) { inCodeFence = false; md[section].push(codeFenceLines.join('\n')); codeFenceLines = null; } continue; }
    if (trimmed.startsWith('```')) { closeList(); closeTable(); inCodeFence = true; codeFenceMarker = trimmed.slice(0, 3); codeFenceLines = [line]; continue; }
    const h = line.match(/^\s*#\s*(.*)$/);
    if (h) { closeList(); closeTable(); ensureSection(h[1].trim() || 'content'); continue; }
    const listMatch = line.match(/^\s*-\s*(.*)$/);
    if (listMatch) { closeTable(); if (!listBuffer) listBuffer = []; listBuffer.push(listMatch[1]); continue; }
    const nextLine = i + 1 < lines.length ? lines[i + 1].trim() : '';
    if (line.includes('|') && /^\s*(\|\s*)?[-:]+(?:\s*\|\s*[-:]+)*\s*(\|\s*)?$/.test(nextLine)) { closeList(); closeTable(); tableState = { headers: parseTableRow(line), rows: [] }; i++; continue; }
    if (tableState && line.includes('|')) { const cells = parseTableRow(line); const row = {}; tableState.headers.forEach((h, ci) => row[h || `col${ci}`] = cells[ci] || ''); tableState.rows.push(row); continue; }
    if (/^\s*$/.test(line)) { closeList(); closeTable(); continue; }
    closeList(); closeTable(); md[section].push(trimmed);
  }
  closeList(); closeTable();
  if (inCodeFence && codeFenceLines) md[section].push(codeFenceLines.join('\n'));
  return md;
}
function papagaio_md_to_markdown(md) {
  if (!md || typeof md !== 'object') return '';
  const lines = [];
  const renderSection = (name, items) => {
    if (!items || !items.length) return;
    if (name !== 'content') lines.push(`# ${name}`);
    for (const item of items) {
      if (typeof item === 'string') { lines.push(item); continue; }
      if (Array.isArray(item)) {
        const first = item[0];
        if (first && typeof first === 'object' && !Array.isArray(first)) {
          const headers = Object.keys(first);
          if (!headers.length) continue;
          lines.push('| ' + headers.join(' | ') + ' |');
          lines.push('| ' + headers.map(() => '---').join(' | ') + ' |');
          for (const row of item) lines.push('| ' + headers.map(h => row[h] !== undefined ? String(row[h]) : '').join(' | ') + ' |');
          continue;
        }
        for (const l of item) lines.push(`- ${l}`);
        continue;
      }
      lines.push(String(item));
    }
    lines.push('');
  };
  renderSection('content', md.content);
  for (const [s, items] of Object.entries(md)) { if (s !== 'content') renderSection(s, items); }
  while (lines.length > 0 && lines[lines.length - 1].trim() === '') lines.pop();
  return lines.join('\n');
}

// ─── Obsidian Plugin ──────────────────────────────────────────────────────────

const { Plugin, Notice } = require("obsidian");

class PapagaioPlugin extends Plugin {
  async onload() {
    this.addCommand({
      id: "papagaio-process",
      name: "Process with papagaio",
      callback: () => this.processActiveFile(),
    });
  }

  async processActiveFile() {
    const file = this.app.workspace.getActiveFile();
    if (!file) {
      new Notice("papagaio: nenhum arquivo ativo.");
      return;
    }

    const content = await this.app.vault.read(file);
    const baseName = file.basename;
    const dir = file.parent ? file.parent.path : "/";

    // ── Captura console.log / warn / error / info ─────────────────────────────
    const logs = [];
    const origLog   = console.log;
    const origWarn  = console.warn;
    const origError = console.error;
    const origInfo  = console.info;

    const capture = (...args) =>
      logs.push(args.map(a => typeof a === 'object' ? JSON.stringify(a, null, 2) : String(a)).join(' '));

    console.log   = capture;
    console.warn  = (...a) => { capture('[WARN]',  ...a); origWarn(...a);  };
    console.error = (...a) => { capture('[ERROR]', ...a); origError(...a); };
    console.info  = (...a) => { capture('[INFO]',  ...a); origInfo(...a);  };

    let processedMd;
    try {
      // 1. MD → objeto JS (roda papagaio internamente)
      const mdObject = papagaio_md_to_object(content);
      // 2. objeto JS → MD recompilado
      processedMd = papagaio_md_to_markdown(mdObject);
    } catch (e) {
      capture('[ERROR]', e.message);
    } finally {
      console.log   = origLog;
      console.warn  = origWarn;
      console.error = origError;
      console.info  = origInfo;
    }

    if (!processedMd) {
      new Notice("papagaio: falha ao processar o arquivo.");
      return;
    }

    // ── Escreve .processed.md ─────────────────────────────────────────────────
    await this.writeOrCreate(`${dir}/${baseName}.processed.md`, processedMd);

    // ── Escreve .output.md (stdout capturado) ─────────────────────────────────
    const outputContent = logs.length > 0
      ? logs.join('\n')
      : `(nenhuma saída de console durante o processamento de ${file.name})`;
    await this.writeOrCreate(`${dir}/${baseName}.output.md`, outputContent);

    new Notice(`papagaio: ${baseName}.processed.md e ${baseName}.output.md gerados ✓`);
  }

  async writeOrCreate(path, content) {
    const existing = this.app.vault.getAbstractFileByPath(path);
    if (existing) {
      await this.app.vault.modify(existing, content);
    } else {
      await this.app.vault.create(path, content);
    }
  }
}

module.exports = PapagaioPlugin;
