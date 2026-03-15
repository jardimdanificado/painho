/*
 * papagaio.md — Obsidian Plugin
 * Painel lateral (ItemView) compatível com desktop e Android.
 *
 * Abre via ribbon (ícone de pena) ou comando "Abrir painel papagaio".
 * O painel contém:
 *   • Toggle + campo de nome → .processed.md
 *   • Toggle + campo de nome → .output.md
 *   • Botão "Processar"
 *   • Preview ao vivo do MD processado
 */

// ─── papagaio core (inlined) ─────────────────────────────────────────────────

const PAPAGAIO_ESCAPED_SIGIL = "\x01";
function make_default_symbols(sigil="$",open="{",close="}"){return{sigil,open,close,pattern:"pattern",eval:"eval",block:"recursive",blockseq:"sequential",regex:"regex",options:"options",optional:"optional"};}
function trim_view(str){return str.replace(/^\s+|\s+$/g,"");}
function prepare_input(input,sym){if(input==null)return null;const sigil=sym.sigil||"$";let out="";for(let i=0;i<input.length;i++){const c=input[i];if(c==="\\"&&sigil&&i+sigil.length<input.length&&input.slice(i+1,i+1+sigil.length)===sigil){out+=PAPAGAIO_ESCAPED_SIGIL;i+=sigil.length;continue;}out+=c;}return out;}
function restore_escaped(input,sym){if(input==null)return null;return input.split(PAPAGAIO_ESCAPED_SIGIL).join(sym.sigil||"$");}
function extract_block(src,pos,open,close){if(!src||pos>=src.length)return{next:pos,content:""};if(open===close){if(!src.startsWith(open,pos))return{next:pos,content:""};const start=pos+open.length;const idx=src.indexOf(close,start);if(idx<0)return{next:src.length,content:src.slice(start)};return{next:idx+close.length,content:src.slice(start,idx)};}if(!src.startsWith(open,pos))return{next:pos,content:""};let cur=pos+open.length,depth=1;const start=cur;while(cur<src.length&&depth>0){if(src.startsWith(open,cur)){depth++;cur+=open.length;}else if(src.startsWith(close,cur)){depth--;if(depth===0)return{next:cur+close.length,content:src.slice(start,cur)};cur+=close.length;}else cur++;}return{next:src.length,content:src.slice(start)};}
const TOK_WS=1,TOK_LITERAL=2,TOK_VAR=3,TOK_BLOCK=4,TOK_BLOCKSEQ=5,TOK_OPTIONS=6,TOK_OPTIONAL_LIT=7;
const MOD_NONE=0,MOD_ALIASES=1,MOD_OPTIONAL=2,MOD_STARTS=3,MOD_ENDS=4,MOD_INT=5,MOD_FLOAT=6,MOD_NUMBER=7,MOD_UPPER=8,MOD_LOWER=9,MOD_CAPITALIZED=10,MOD_WORD=11,MOD_IDENTIFIER=12,MOD_PATH=13,MOD_HEX=14,MOD_BINARY=15,MOD_PERCENT=16;
function skip_ws(src,pos){while(pos<src.length&&/\s/.test(src[pos]))pos++;return pos;}
function parse_pattern_ex(pat,sym){const p={t:[],sym};const n=pat.length;let i=0;const sigil=sym.sigil||"$";while(i<n){const t={type:0,optional:0,all_opt:0,next_sig:-1,modifier:MOD_NONE,var:null,value:"",open:sym.open,close:sym.close,alts:[]};if(/\s/.test(pat[i])){while(i<n&&/\s/.test(pat[i]))i++;t.type=TOK_WS;p.t.push(t);continue;}if(pat.startsWith(sigil,i)){i+=sigil.length;const rem=pat.slice(i);const words=[sym.block,sym.blockseq,sym.options,sym.optional,sym.regex].filter(Boolean);let found=null,foundLen=0;for(const w of words){if(w&&rem.startsWith(w)&&w.length>foundLen){found=w;foundLen=w.length;}}if(found){i+=foundLen;if(found===sym.block)t.type=TOK_BLOCK;else if(found===sym.blockseq)t.type=TOK_BLOCKSEQ;else if(found===sym.options)t.type=TOK_OPTIONS;else if(found===sym.optional)t.type=TOK_OPTIONAL_LIT;else t.type=TOK_LITERAL;const blk=extract_block(pat,i,sym.open,sym.close);i=blk.next;const trimmed=trim_view(blk.content);if(t.type===TOK_BLOCK||t.type===TOK_BLOCKSEQ){t.open=sym.open;t.close=sym.close;t.value=trimmed;}else if(t.type===TOK_OPTIONS){t.alts=trimmed.split(/\s*,\s*/).map(x=>x.trim()).filter(Boolean);}else if(t.type===TOK_OPTIONAL_LIT){t.value=trimmed;t.optional=1;}else t.value=trimmed;p.t.push(t);continue;}let varName="";while(i<n&&/[A-Za-z0-9_]/.test(pat[i]))varName+=pat[i++];t.type=TOK_VAR;t.var=varName;const modMap={int:MOD_INT,float:MOD_FLOAT,number:MOD_NUMBER,upper:MOD_UPPER,lower:MOD_LOWER,capitalized:MOD_CAPITALIZED,word:MOD_WORD,identifier:MOD_IDENTIFIER,path:MOD_PATH,hex:MOD_HEX,binary:MOD_BINARY,percent:MOD_PERCENT};for(const[k,v]of Object.entries(modMap)){if(pat.startsWith(`$${k}`,i)){t.modifier=v;i+=k.length+1;break;}}if(pat.startsWith('$aliases{',i)){i+=9;const end=pat.indexOf('}',i);if(end>=0){t.modifier=MOD_ALIASES;t.alts=pat.slice(i,end).split(/\s*,\s*/).map(s=>s.trim()).filter(Boolean);i=end+1;}}else if(pat.startsWith('$optional{',i)){i+=10;const end=pat.indexOf('}',i);if(end>=0){t.modifier=MOD_OPTIONAL;t.value=pat.slice(i,end);i=end+1;}}else if(pat.startsWith('$starts{',i)){i+=8;const end=pat.indexOf('}',i);if(end>=0){t.modifier=MOD_STARTS;t.value=pat.slice(i,end);i=end+1;}}else if(pat.startsWith('$ends{',i)){i+=7;const end=pat.indexOf('}',i);if(end>=0){t.modifier=MOD_ENDS;t.value=pat.slice(i,end);i=end+1;}}p.t.push(t);continue;}let lit="";while(i<n&&!pat.startsWith(sigil,i)&&!/\s/.test(pat[i]))lit+=pat[i++];t.type=TOK_LITERAL;t.value=lit;p.t.push(t);}let has_non_opt=false;for(const t of p.t){if(t.type!==TOK_WS&&t.modifier!==MOD_OPTIONAL&&t.type!==TOK_OPTIONAL_LIT&&!t.optional)has_non_opt=true;}if(!has_non_opt)for(const t of p.t)t.all_opt=1;for(let j=0;j<p.t.length;j++){if(p.t[j].type===TOK_VAR||p.t[j].type===TOK_BLOCK||p.t[j].type===TOK_BLOCKSEQ){for(let k=j+1;k<p.t.length;k++){if(p.t[k].type!==TOK_WS){if(p.t[k].type===TOK_VAR||p.t[k].type===TOK_LITERAL||p.t[k].type===TOK_OPTIONS)p.t[j].next_sig=k;break;}}}}return p;}
function apply_modifier(val,mod,alts){if(mod===MOD_INT){const n=parseInt(val,10);return isNaN(n)?null:String(n);}if(mod===MOD_FLOAT){const f=parseFloat(val);return isNaN(f)?null:String(f);}if(mod===MOD_NUMBER){const n2=Number(val);return isNaN(n2)?null:String(n2);}if(mod===MOD_UPPER)return val.toUpperCase();if(mod===MOD_LOWER)return val.toLowerCase();if(mod===MOD_CAPITALIZED)return val.charAt(0).toUpperCase()+val.slice(1).toLowerCase();if(mod===MOD_WORD)return/^\w+$/.test(val)?val:null;if(mod===MOD_IDENTIFIER)return/^[A-Za-z_]\w*$/.test(val)?val:null;if(mod===MOD_PATH)return/^[^\0]+$/.test(val)?val:null;if(mod===MOD_HEX)return/^[0-9a-fA-F]+$/.test(val)?val:null;if(mod===MOD_BINARY)return/^[01]+$/.test(val)?val:null;if(mod===MOD_PERCENT){const p=parseFloat(val);return isNaN(p)?null:String(p);}if(mod===MOD_ALIASES&&alts){for(const a of alts)if(val===a)return val;return null;}return val;}
function match_pattern(src,pos,p){const vars={};function match_tok(ti,si){if(ti>=p.t.length)return si;const t=p.t[ti];if(t.type===TOK_WS)return match_tok(ti+1,skip_ws(src,si));if(t.type===TOK_LITERAL){if(src.startsWith(t.value,si))return match_tok(ti+1,si+t.value.length);if(t.optional||t.all_opt)return match_tok(ti+1,si);return-1;}if(t.type===TOK_OPTIONS){for(const alt of t.alts){if(src.startsWith(alt,si)){const r=match_tok(ti+1,si+alt.length);if(r>=0)return r;}}if(t.optional||t.all_opt)return match_tok(ti+1,si);return-1;}if(t.type===TOK_OPTIONAL_LIT){if(src.startsWith(t.value,si)){const r=match_tok(ti+1,si+t.value.length);if(r>=0)return r;}return match_tok(ti+1,si);}if(t.type===TOK_VAR){if(t.modifier===MOD_OPTIONAL){const r1=try_match_var(t,ti,si);if(r1>=0)return r1;return match_tok(ti+1,si);}return try_match_var(t,ti,si);}if(t.type===TOK_BLOCK){const blk=extract_block(src,si,t.open,t.close);if(blk.next===si&&!src.startsWith(t.open,si))return-1;vars[t.var||'_block']=blk.content;return match_tok(ti+1,blk.next);}if(t.type===TOK_BLOCKSEQ){const results=[];let cur=si;while(cur<src.length){const blk=extract_block(src,cur,t.open,t.close);if(blk.next===cur)break;results.push(blk.content);cur=skip_ws(src,blk.next);}vars[t.var||'_seq']=results;return match_tok(ti+1,cur);}return-1;}function try_match_var(t,ti,si){const next=t.next_sig;let end;if(next>=0){const nt=p.t[next];if(nt.type===TOK_LITERAL){const idx=src.indexOf(nt.value,si);if(idx<0)return-1;end=idx;}else end=src.length;}else end=src.length;const candidate=src.slice(si,end);const applied=apply_modifier(candidate,t.modifier,t.alts);if(applied===null)return-1;vars[t.var]=applied;return match_tok(ti+1,end);}const endPos=match_tok(0,pos);if(endPos<0)return null;return{end:endPos,vars};}
function apply_patterns(src,pairs,sym){let out="";let i=0;while(i<src.length){let matched=false;for(const pair of pairs){const res=match_pattern(src,i,pair.pattern);if(res&&res.end>i){let replacement=pair.template;for(const[k,v]of Object.entries(res.vars))replacement=replacement.split(`$${k}`).join(Array.isArray(v)?v.join(''):v);out+=replacement;i=res.end;matched=true;break;}}if(!matched)out+=src[i++];}return out;}
function extract_nested(src,sym){const pairs=[];let clean="";let i=0;const sigil=sym.sigil||"$";while(i<src.length){if(src.startsWith(sigil+sym.pattern,i)){i+=sigil.length+sym.pattern.length;const patBlk=extract_block(src,i,sym.open,sym.close);i=patBlk.next;const tplBlk=extract_block(src,i,sym.open,sym.close);i=tplBlk.next;pairs.push({pattern:parse_pattern_ex(trim_view(patBlk.content),sym),template:trim_view(tplBlk.content)});}else clean+=src[i++];}return{clean,pairs};}
function extract_evals(src,sym){const evals=[];let out="";let i=0;const sigil=sym.sigil||"$";while(i<src.length){if(src.startsWith(sigil+sym.eval,i)){i+=sigil.length+sym.eval.length;const blk=extract_block(src,i,sym.open,sym.close);evals.push(trim_view(blk.content));out+=`__E${evals.length-1}__`;i=blk.next;continue;}out+=src[i++];}return{ph:out,evals};}
function apply_evals(ph,evals){let current=ph;for(let i=evals.length-1;i>=0;i--){let result="";try{const fn=new Function('match',evals[i]);const res=fn('');if(res!==undefined&&res!==null)result=String(res);}catch(e){result=`error: ${e}`;}current=current.split(`__E${i}__`).join(result);}return current;}
function papagaio_process_text(input){const sym=make_default_symbols();let src=prepare_input(input,sym);if(!src)return null;while(true){const nested=extract_nested(src,sym);const evalExtract=extract_evals(nested.clean,sym);let proc=apply_evals(evalExtract.ph,evalExtract.evals);if(nested.pairs.length>0){while(true){const next=apply_patterns(proc,nested.pairs,sym);if(next===proc)break;proc=next;}}const restored=restore_escaped(proc,sym);if(restored===src){src=restored;break;}src=restored;}return restore_escaped(src,sym);}
function parseTableRow(line){return line.trim().replace(/^\|/,"").replace(/\|$/,"").split('|').map(c=>c.trim());}
function papagaio_md_to_object(markdown){const processed=papagaio_process_text(markdown);const lines=processed.split(/\r?\n/);const md={content:[]};let section='content',inCodeFence=false,codeFenceMarker='',codeFenceLines=null,listBuffer=null,tableState=null;const ensureSection=name=>{if(!md[name])md[name]=[];section=name;};const closeList=()=>{if(listBuffer){md[section].push(listBuffer);listBuffer=null;}};const closeTable=()=>{if(tableState){md[section].push(tableState.rows);tableState=null;}};for(let i=0;i<lines.length;i++){const line=lines[i].replace(/\r$/,"");const trimmed=line.trim();if(inCodeFence){codeFenceLines.push(line);if(trimmed.startsWith(codeFenceMarker)){inCodeFence=false;md[section].push(codeFenceLines.join('\n'));codeFenceLines=null;}continue;}if(trimmed.startsWith('```')){closeList();closeTable();inCodeFence=true;codeFenceMarker=trimmed.slice(0,3);codeFenceLines=[line];continue;}const h=line.match(/^\s*#\s*(.*)$/);if(h){closeList();closeTable();ensureSection(h[1].trim()||'content');continue;}const listMatch=line.match(/^\s*-\s*(.*)$/);if(listMatch){closeTable();if(!listBuffer)listBuffer=[];listBuffer.push(listMatch[1]);continue;}const nextLine=i+1<lines.length?lines[i+1].trim():'';if(line.includes('|')&&/^\s*(\|\s*)?[-:]+(?:\s*\|\s*[-:]+)*\s*(\|\s*)?$/.test(nextLine)){closeList();closeTable();tableState={headers:parseTableRow(line),rows:[]};i++;continue;}if(tableState&&line.includes('|')){const cells=parseTableRow(line);const row={};tableState.headers.forEach((h,ci)=>row[h||`col${ci}`]=cells[ci]||'');tableState.rows.push(row);continue;}if(/^\s*$/.test(line)){closeList();closeTable();continue;}closeList();closeTable();md[section].push(trimmed);}closeList();closeTable();if(inCodeFence&&codeFenceLines)md[section].push(codeFenceLines.join('\n'));return md;}
function papagaio_md_to_markdown(md){if(!md||typeof md!=='object')return'';const lines=[];const renderSection=(name,items)=>{if(!items||!items.length)return;if(name!=='content')lines.push(`# ${name}`);for(const item of items){if(typeof item==='string'){lines.push(item);continue;}if(Array.isArray(item)){const first=item[0];if(first&&typeof first==='object'&&!Array.isArray(first)){const headers=Object.keys(first);if(!headers.length)continue;lines.push('| '+headers.join(' | ')+' |');lines.push('| '+headers.map(()=>'---').join(' | ')+' |');for(const row of item)lines.push('| '+headers.map(h=>row[h]!==undefined?String(row[h]):'').join(' | ')+' |');continue;}for(const l of item)lines.push(`- ${l}`);continue;}lines.push(String(item));}lines.push('');};renderSection('content',md.content);for(const[s,items]of Object.entries(md)){if(s!=='content')renderSection(s,items);}while(lines.length>0&&lines[lines.length-1].trim()==='')lines.pop();return lines.join('\n');}

// ─── Obsidian API ─────────────────────────────────────────────────────────────

const { Plugin, ItemView, Notice, MarkdownRenderer } = require("obsidian");

const PAPAGAIO_VIEW_TYPE = "papagaio-panel";

// ─── Sidebar ItemView ─────────────────────────────────────────────────────────

class PapagaioView extends ItemView {
  constructor(leaf, plugin) {
    super(leaf);
    this.plugin = plugin;
    this.exportProcessed = true;
    this.processedName   = "";
    this.exportOutput    = true;
    this.outputName      = "";
    this.lastFile        = null;
    this.isProcessing    = false;
  }

  getViewType()   { return PAPAGAIO_VIEW_TYPE; }
  getDisplayText(){ return "papagaio"; }
  getIcon()       { return "feather"; }

  async onOpen() {
    this._injectStyles();
    this._buildUI();
    this._syncFromActiveFile();

    this.registerEvent(
      this.app.workspace.on("active-leaf-change", () => this._syncFromActiveFile())
    );
  }

  async onClose() {
    this.containerEl.empty();
  }

  // ── Sync filename defaults when note changes ───────────────────────────────
  _syncFromActiveFile() {
    const file = this.app.workspace.getActiveFile();
    if (!file || file === this.lastFile) return;
    this.lastFile = file;

    const defP = `${file.basename}.processed`;
    const defO = `${file.basename}.output`;

    // Only reset if still at a default value (user hasn't customised)
    if (this._processedInput && (!this.processedName || this.processedName.endsWith(".processed"))) {
      this.processedName = defP;
      this._processedInput.value = defP;
    }
    if (this._outputInput && (!this.outputName || this.outputName.endsWith(".output"))) {
      this.outputName = defO;
      this._outputInput.value = defO;
    }

    this._clearPreview();
  }

  // ── DOM construction ───────────────────────────────────────────────────────
  _buildUI() {
    const root = this.containerEl;
    root.empty();
    root.addClass("pg-view");

    // Header
    const hdr = root.createDiv("pg-header");
    hdr.createEl("span", { cls: "pg-parrot", text: "🦜" });
    hdr.createEl("span", { cls: "pg-wordmark", text: "papagaio" });

    // Options card
    const card = root.createDiv("pg-card");
    this._buildRow(card, "processed", true,  (v) => { this.exportProcessed = v; }, (v) => { this.processedName = v; }, (el) => { this._processedInput = el; });
    this._buildRow(card, "output",    true,  (v) => { this.exportOutput    = v; }, (v) => { this.outputName    = v; }, (el) => { this._outputInput    = el; });

    // Process button
    this._btn = root.createEl("button", { cls: "pg-btn", text: "Processar" });
    this._btn.addEventListener("click", () => this._run());

    // Preview
    const prev = root.createDiv("pg-preview");
    const prevHdr = prev.createDiv("pg-preview-hdr");
    prevHdr.createEl("span", { cls: "pg-preview-label", text: "preview" });
    this._statusEl = prevHdr.createEl("span", { cls: "pg-status" });
    this._previewBody = prev.createDiv("pg-preview-body");
    this._clearPreview();
  }

  _buildRow(parent, label, initial, onToggle, onInput, refCallback) {
    const row = parent.createDiv("pg-row");

    // Custom toggle
    let state = initial;
    const track = row.createDiv("pg-track" + (initial ? " pg-track--on" : ""));
    track.createDiv("pg-knob");
    track.addEventListener("click", () => {
      state = !state;
      track.classList.toggle("pg-track--on", state);
      inputEl.disabled = !state;
      inputEl.classList.toggle("pg-dim", !state);
      onToggle(state);
    });

    row.createEl("span", { cls: "pg-label", text: label });

    const wrap = row.createDiv("pg-field");
    const inputEl = wrap.createEl("input", { type: "text", cls: "pg-input", placeholder: "nome do arquivo" });
    wrap.createEl("span", { cls: "pg-dot-md", text: ".md" });

    inputEl.addEventListener("input", (e) => onInput(e.target.value.trim()));
    refCallback(inputEl);
  }

  // ── Processing ─────────────────────────────────────────────────────────────
  async _run() {
    if (this.isProcessing) return;

    const file = this.app.workspace.getActiveFile();
    if (!file) { new Notice("papagaio: nenhum arquivo ativo."); return; }

    const dir  = file.parent ? file.parent.path : "/";
    const nameP = (this._processedInput.value.trim()) || `${file.basename}.processed`;
    const nameO = (this._outputInput.value.trim())    || `${file.basename}.output`;

    this.isProcessing = true;
    this._btn.disabled = true;
    this._btn.textContent = "Processando…";
    this._setStatus("…", "loading");
    this._previewBody.empty();
    this._previewBody.createEl("p", { cls: "pg-hint", text: "Processando…" });

    const content = await this.app.vault.read(file);

    // Capture console
    const logs = [];
    const cap   = (...a) => logs.push(a.map(x => typeof x === "object" ? JSON.stringify(x, null, 2) : String(x)).join(" "));
    const oLog  = console.log;   console.log   = cap;
    const oWarn = console.warn;  console.warn  = (...a) => { cap("[WARN]",  ...a); oWarn(...a);  };
    const oErr  = console.error; console.error = (...a) => { cap("[ERROR]", ...a); oErr(...a);   };
    const oInfo = console.info;  console.info  = (...a) => { cap("[INFO]",  ...a); oInfo(...a);  };

    let processedMd = null;
    try {
      processedMd = papagaio_md_to_markdown(papagaio_md_to_object(content));
    } catch (e) {
      cap("[ERROR]", e.message);
    } finally {
      console.log   = oLog;
      console.warn  = oWarn;
      console.error = oErr;
      console.info  = oInfo;
    }

    this.isProcessing = false;
    this._btn.disabled = false;
    this._btn.textContent = "Processar";

    if (!processedMd) {
      new Notice("papagaio: falha ao processar.");
      this._setStatus("erro", "err");
      this._previewBody.empty();
      this._previewBody.createEl("p", { cls: "pg-hint", text: "Erro ao processar o arquivo." });
      return;
    }

    const written = [];

    if (this.exportProcessed) {
      await this._write(`${dir}/${nameP}.md`, processedMd);
      written.push(`${nameP}.md`);
    }
    if (this.exportOutput) {
      const out = logs.length ? logs.join("\n") : `(sem saída de console — ${file.name})`;
      await this._write(`${dir}/${nameO}.md`, out);
      written.push(`${nameO}.md`);
    }

    if (written.length === 0) new Notice("papagaio: nenhum arquivo exportado.");
    else new Notice(`papagaio: ${written.join(" e ")} ✓`);

    // Render preview using Obsidian's own renderer (respects theme + plugins)
    this._setStatus("ok", "ok");
    this._previewBody.empty();
    MarkdownRenderer.render(
      this.app,
      processedMd,
      this._previewBody,
      file.path,
      this
    );
  }

  _setStatus(text, kind) {
    this._statusEl.textContent = text;
    this._statusEl.className = `pg-status pg-status--${kind}`;
  }

  _clearPreview() {
    if (!this._previewBody) return;
    this._previewBody.empty();
    this._previewBody.createEl("p", { cls: "pg-hint", text: "Processe um arquivo para ver o resultado aqui." });
    if (this._statusEl) this._statusEl.textContent = "";
  }

  async _write(path, content) {
    const ex = this.app.vault.getAbstractFileByPath(path);
    if (ex) await this.app.vault.modify(ex, content);
    else     await this.app.vault.create(path, content);
  }

  // ── Styles (injected into <head> once, removed on unload) ─────────────────
  _injectStyles() {
    if (document.getElementById("pg-styles")) return;
    const s = document.createElement("style");
    s.id = "pg-styles";
    s.textContent = `
      .pg-view {
        display: flex;
        flex-direction: column;
        height: 100%;
        overflow: hidden;
      }

      /* Header */
      .pg-header {
        display: flex;
        align-items: center;
        gap: 6px;
        padding: 12px 14px 10px;
        border-bottom: 1px solid var(--background-modifier-border);
        flex-shrink: 0;
      }
      .pg-parrot  { font-size: 1.15em; }
      .pg-wordmark {
        font-size: 0.9em;
        font-weight: 700;
        letter-spacing: 0.09em;
        color: var(--text-normal);
        opacity: 0.8;
      }

      /* Options card */
      .pg-card {
        margin: 10px 10px 0;
        padding: 10px 12px;
        border-radius: 8px;
        background: var(--background-secondary);
        border: 1px solid var(--background-modifier-border);
        display: flex;
        flex-direction: column;
        gap: 10px;
        flex-shrink: 0;
      }

      /* Row */
      .pg-row {
        display: flex;
        align-items: center;
        gap: 8px;
      }
      .pg-label {
        font-size: 0.72em;
        font-weight: 600;
        letter-spacing: 0.05em;
        color: var(--text-muted);
        width: 58px;
        flex-shrink: 0;
      }

      /* Toggle track + knob */
      .pg-track {
        position: relative;
        width: 30px;
        height: 17px;
        border-radius: 9px;
        background: var(--background-modifier-border);
        cursor: pointer;
        flex-shrink: 0;
        transition: background 0.18s;
        -webkit-tap-highlight-color: transparent;
      }
      .pg-track--on { background: var(--interactive-accent); }
      .pg-knob {
        position: absolute;
        top: 2px; left: 2px;
        width: 13px; height: 13px;
        border-radius: 50%;
        background: #fff;
        box-shadow: 0 1px 3px rgba(0,0,0,.22);
        transition: transform 0.18s;
      }
      .pg-track--on .pg-knob { transform: translateX(13px); }

      /* Text input */
      .pg-field {
        display: flex;
        align-items: center;
        flex: 1;
        min-width: 0;
        gap: 2px;
      }
      .pg-input {
        flex: 1;
        min-width: 0;
        padding: 4px 7px;
        font-size: 0.78em;
        font-family: var(--font-monospace);
        background: var(--background-primary);
        border: 1px solid var(--background-modifier-border);
        border-radius: 5px;
        color: var(--text-normal);
        outline: none;
        transition: border-color 0.12s, opacity 0.15s;
      }
      .pg-input:focus  { border-color: var(--interactive-accent); }
      .pg-input:disabled,
      .pg-input.pg-dim { opacity: 0.3; cursor: not-allowed; }
      .pg-dot-md {
        font-size: 0.72em;
        color: var(--text-faint);
        font-family: var(--font-monospace);
        flex-shrink: 0;
      }

      /* Process button */
      .pg-btn {
        margin: 10px 10px 0;
        width: calc(100% - 20px);
        padding: 9px 0;
        background: var(--interactive-accent);
        color: var(--text-on-accent);
        border: none;
        border-radius: 7px;
        font-size: 0.84em;
        font-weight: 600;
        letter-spacing: 0.04em;
        cursor: pointer;
        flex-shrink: 0;
        transition: opacity 0.14s, transform 0.1s;
        -webkit-tap-highlight-color: transparent;
      }
      .pg-btn:hover    { opacity: 0.86; }
      .pg-btn:active   { transform: scale(0.98); }
      .pg-btn:disabled { opacity: 0.4; cursor: not-allowed; transform: none; }

      /* Preview panel */
      .pg-preview {
        margin: 10px 10px 10px;
        flex: 1;
        min-height: 0;
        display: flex;
        flex-direction: column;
        border: 1px solid var(--background-modifier-border);
        border-radius: 8px;
        overflow: hidden;
      }
      .pg-preview-hdr {
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 5px 10px;
        background: var(--background-secondary);
        border-bottom: 1px solid var(--background-modifier-border);
        flex-shrink: 0;
      }
      .pg-preview-label {
        font-size: 0.67em;
        font-weight: 700;
        letter-spacing: 0.1em;
        text-transform: uppercase;
        color: var(--text-muted);
      }
      .pg-status {
        font-size: 0.65em;
        font-weight: 700;
        padding: 1px 6px;
        border-radius: 4px;
      }
      .pg-status--ok      { background: var(--color-green);  color: #fff; }
      .pg-status--loading { background: var(--color-yellow); color: #000; }
      .pg-status--err     { background: var(--color-red);    color: #fff; }

      .pg-preview-body {
        flex: 1;
        overflow-y: auto;
        padding: 10px 13px;
        font-size: 0.85em;
      }
      .pg-hint {
        color: var(--text-faint);
        font-style: italic;
        text-align: center;
        margin-top: 2em;
        font-size: 0.88em;
      }
    `;
    document.head.appendChild(s);
  }
}

// ─── Plugin ───────────────────────────────────────────────────────────────────

class PapagaioPlugin extends Plugin {
  async onload() {
    this.registerView(PAPAGAIO_VIEW_TYPE, (leaf) => new PapagaioView(leaf, this));

    // Ribbon icon — visible on Android too
    this.addRibbonIcon("feather", "Abrir papagaio", () => this._open());

    this.addCommand({
      id: "papagaio-open-panel",
      name: "Abrir painel papagaio",
      callback: () => this._open(),
    });
  }

  async onunload() {
    this.app.workspace.detachLeavesOfType(PAPAGAIO_VIEW_TYPE);
    document.getElementById("pg-styles")?.remove();
  }

  async _open() {
    const { workspace } = this.app;
    const existing = workspace.getLeavesOfType(PAPAGAIO_VIEW_TYPE);
    if (existing.length) { workspace.revealLeaf(existing[0]); return; }

    const leaf = workspace.getRightLeaf(false);
    await leaf.setViewState({ type: PAPAGAIO_VIEW_TYPE, active: true });
    workspace.revealLeaf(workspace.getLeavesOfType(PAPAGAIO_VIEW_TYPE)[0]);
  }
}

module.exports = PapagaioPlugin;