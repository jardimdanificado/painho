// ============================================
// painho
// ============================================

const painho_version = "0.0.7"
const MAX_ITERATIONS = 512;
let globalClearFlag = false;

// ============================================
// CONFIG GLOBAL DE DELIMITADORES
// ============================================
let OPEN = "{";
let CLOSE = "}";

function changeQuote(open, close) {
    OPEN = open;
    CLOSE = close;
}

function extractBlock(src, openpos, open = OPEN, close = CLOSE) {
    let i = openpos;
    let depth = 0;
    let startInner = null;
    let inString = false;
    let strChar = '';

    while (i < src.length) {
        let ch = src[i];

        if (inString) {
            if (ch === '\\') { i += 2; continue; }
            if (ch === strChar) { inString = false; strChar = ''; }
            i++;
            continue;
        } else {
            if (ch === '"' || ch === "'" || ch === "`") {
                inString = true;
                strChar = ch;
                i++;
                continue;
            }
        }

        if (ch === open) {
            depth++;
            if (startInner === null) startInner = i + 1;
        } else if (ch === close) {
            depth--;
            if (depth === 0) {
                const inner = startInner !== null ? src.substring(startInner, i) : '';
                return [inner, i + 1];
            }
        }

        i++;
    }

    const inner = startInner !== null ? src.substring(startInner) : '';
    return [inner, src.length];
}


// ============================================
// SISTEMA GLOBAL DE CONTADOR + UNIQUE
// ============================================
const counterState = {
    value: 0,
    unique: 0,
    reset() {
        this.value = 0;
        this.unique = 0;
    },
    genUnique() {
        return "u" + (this.unique++).toString(36);
    }
};


function patternToRegex(pattern) {
    let regex = '';
    let i = 0;
    let varCounter = 0;

    while (i < pattern.length) {

        if (i + 1 < pattern.length && pattern[i] === '$' && pattern[i + 1] === '$') {
            // aqui é o $$ "concat"
            regex += '\\s*';
            i += 2;
            continue;
        }

        // --- resto inalterado: delimitadores com variáveis, $var..., etc. ---
        // Verifica por delimitadores com variáveis: {$var}, ($var), ...
        if ((pattern[i] === OPEN || pattern[i] === CLOSE ||
            pattern[i] === '{' || pattern[i] === '(' || pattern[i] === '[' || pattern[i] === '<' ||
            pattern[i] === '"' || pattern[i] === "'" || pattern[i] === '`') &&
            i + 1 < pattern.length && pattern[i + 1] === '$') {

            const openDelim = pattern[i];
            const closeDelim =
                openDelim === OPEN ? CLOSE :
                openDelim === '{' ? '}' :
                openDelim === '(' ? ')' :
                openDelim === '[' ? ']' :
                openDelim === '<' ? '>' :
                openDelim === '"' ? '"' :
                openDelim === "'" ? "'" :
                openDelim === '`' ? '`' : openDelim;

            // Captura nome da variável
            let j = i + 2; // pula o delimitador e o $
            while (j < pattern.length && /[A-Za-z0-9_]/.test(pattern[j])) {
                j++;
            }

            // Verifica se fecha com o delimitador correto
            if (j < pattern.length && pattern[j] === closeDelim) {
                const escapedOpen = openDelim === '(' ? '\\(' : 
                                   openDelim === '[' ? '\\[' : 
                                   openDelim === '{' ? '\\{' :
                                   openDelim === '<' ? '\\<' :
                                   openDelim === '"' ? '"' :
                                   openDelim === "'" ? "'" :
                                   openDelim === '`' ? '`' :
                                   openDelim;
                const escapedClose = closeDelim === ')' ? '\\)' : 
                                    closeDelim === ']' ? '\\]' : 
                                    closeDelim === '}' ? '\\}' :
                                    closeDelim === '>' ? '\\>' :
                                    closeDelim === '"' ? '"' :
                                    closeDelim === "'" ? "'" :
                                    closeDelim === '`' ? '`' :
                                    closeDelim;

                // Para aspas, usa regex simples; para delimitadores, usa blocos balanceados
                let innerRegex;
                if (openDelim === '"' || openDelim === "'" || openDelim === '`') {
                    innerRegex = '[^' + escapedOpen + ']*';
                } else {
                    innerRegex = buildBalancedBlockRegex(openDelim, closeDelim);
                }

                regex += escapedOpen + '(' + innerRegex + ')' + escapedClose;
                varCounter++;
                i = j + 1;
                continue;
            }
        }

        // Verifica por captura até token: $var...token
        if (pattern[i] === '$') {
            let j = i + 1;
            let varName = '';
            while (j < pattern.length && /[A-Za-z0-9_]/.test(pattern[j])) {
                varName += pattern[j];
                j++;
            }

            if (varName && j < pattern.length && pattern[j] === '.' && j + 2 < pattern.length && pattern[j + 1] === '.' && pattern[j + 2] === '.') {
                // Achamos $var...
                j += 3; // pula ...
                let token = '';
                while (j < pattern.length && /\S/.test(pattern[j])) { // Lê até encontrar um espaço ou fim
                    token += pattern[j];
                    j++;
                }

                if (token) {
                    // Escapa o token para regex
                    const escapedToken = token.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
                    // Captura qualquer coisa até o token (não-greedy)
                    regex += '((?:.|\\r|\\n)*?)' + escapedToken;
                    varCounter++;
                    i = j;
                    continue;
                }
            }

            // Se não for ...token, apenas captura variável normal
            if (varName) {
                regex += '(\\S+)';
                varCounter++;
                i = j;
            } else {
                // isolado: só um $ literal
                regex += '\\$';
                i++;
            }
            continue;
        }

        if (/\s/.test(pattern[i])) {
            // Qualquer whitespace vira \s+
            regex += '\\s+';
            // Pula todos os espaços consecutivos no pattern
            while (i < pattern.length && /\s/.test(pattern[i])) {
                i++;
            }
        } else {
            // Caractere literal (escapa se necessário)
            const char = pattern[i];
            if (/[.*+?^${}()|[\]\\]/.test(char)) {
                regex += '\\' + char;
            } else {
                regex += char;
            }
            i++;
        }
    }

    return new RegExp(regex, 'g');
}


// Função auxiliar para gerar regex que captura blocos balanceados
function buildBalancedBlockRegex(open, close) {
    const escapedOpen = open === '(' ? '\\(' : (open === '[' ? '\\[' : open === '{' ? '\\{' : open === '<' ? '\\<' : open);
    const escapedClose = close === ')' ? '\\)' : (close === ']' ? '\\]' : close === '}' ? '\\}' : close === '>' ? '\\>' : close);

    // Regex para capturar blocos balanceados
    // Ex: {a{b}c} -> captura a{b}c
    return `(?:[^${escapedOpen}${escapedClose}\\\\]|\\\\.|${escapedOpen}(?:[^${escapedOpen}${escapedClose}\\\\]|\\\\.)*${escapedClose})*`;
}

// Versão corrigida de extractVarNames
function extractVarNames(pattern) {
    const vars = [];
    const seen = new Set();
    let i = 0;

    while (i < pattern.length) {

        // Verifica por delimitadores com variáveis (incluindo aspas e angle brackets)
        if ((pattern[i] === '{' || pattern[i] === '(' || pattern[i] === '[' || pattern[i] === '<' ||
             pattern[i] === '"' || pattern[i] === "'" || pattern[i] === '`') &&
            i + 1 < pattern.length && pattern[i + 1] === '$') {

            const closeDelim = pattern[i] === '{' ? '}' : 
                               pattern[i] === '(' ? ')' : 
                               pattern[i] === '[' ? ']' :
                               pattern[i] === '<' ? '>' :
                               pattern[i] === '"' ? '"' :
                               pattern[i] === "'" ? "'" :
                               pattern[i] === '`' ? '`' :
                               pattern[i];
            let j = i + 2;

            while (j < pattern.length && /[A-Za-z0-9_]/.test(pattern[j])) {
                j++;
            }

            if (j < pattern.length && pattern[j] === closeDelim) {
                const varName = pattern.substring(i + 2, j);
                if (!seen.has(varName)) {
                    vars.push('$' + varName);
                    seen.add(varName);
                }
                i = j + 1;
                continue;
            }
        }

        // Verifica por captura até token: $var...token
        if (pattern[i] === '$') {
            let j = i + 1;
            let varName = '';
            while (j < pattern.length && /[A-Za-z0-9_]/.test(pattern[j])) {
                varName += pattern[j];
                j++;
            }

            if (varName && j < pattern.length && pattern[j] === '.' && j + 2 < pattern.length && pattern[j + 1] === '.' && pattern[j + 2] === '.') {
                // Achamos $var...
                j += 3; // pula ...
                let token = '';
                while (j < pattern.length && /\S/.test(pattern[j])) { // Lê até encontrar um espaço ou fim
                    token += pattern[j];
                    j++;
                }

                if (token && !seen.has(varName)) {
                    vars.push('$' + varName);
                    seen.add(varName);
                }
                i = j;
                continue;
            }

            if (varName && !seen.has(varName)) {
                vars.push('$' + varName);
                seen.add(varName);
            }
            i = j;
        } else {
            i++;
        }
    }

    return vars;
}

// ============================================
// INTEGRAÇÕES PRINCIPAIS
// ============================================

// === COLETA DE MACROS (MODIFICADA) ===
function collectMacros(src) {
    const macros = {};

    // exemplo: macro nome {
    const macroRegex = new RegExp(`\\bmacro\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\${OPEN}`, "g");

    let match;
    const matches = [];

    while ((match = macroRegex.exec(src)) !== null) {
        matches.push({
            name: match[1],
            matchStart: match.index,
            openPos: match.index + match[0].length - 1
        });
    }

    for (let j = matches.length - 1; j >= 0; j--) {
        const m = matches[j];
        const [body, posAfter] = extractBlock(src, m.openPos, OPEN, CLOSE);
        macros[m.name] = body;

        let left = src.substring(0, m.matchStart);
        let right = src.substring(posAfter);
        src = collapseLocalNewlines(left, right);
    }

    return [macros, src];
}

// === COLETA DE PATTERNS (MODIFICADA) ===
function collectPatterns(src) {
    const patterns = [];

    const patternRegex = new RegExp(`\\bpattern\\s*\\${OPEN}`, "g");

    let match;
    const matches = [];

    while ((match = patternRegex.exec(src)) !== null) {
        matches.push({
            matchStart: match.index,
            openPos: match.index + match[0].length - 1
        });
    }

    for (let j = matches.length - 1; j >= 0; j--) {
        const m = matches[j];
        const [matchPattern, posAfterMatch] = extractBlock(src, m.openPos, OPEN, CLOSE);

        let k = posAfterMatch;
        while (k < src.length && /\s/.test(src[k])) k++;

        if (k < src.length && src[k] === OPEN) {
            const [replacePattern, posAfterReplace] = extractBlock(src, k, OPEN, CLOSE);

            patterns.push({
                match: matchPattern.trim(),
                replace: replacePattern.trim()
            });

            let left = src.substring(0, m.matchStart);
            let right = src.substring(posAfterReplace);
            src = collapseLocalNewlines(left, right);
        }
    }

    return [patterns, src];
}


function applyPatterns(src, patterns) {
    let globalClearFlag = false;
    let lastResult = "";

    for (const pattern of patterns) {
        let changed = true;
        let iterations = 0;

        while (changed && iterations < MAX_ITERATIONS) {
            changed = false;
            iterations++;

            const regex = patternToRegex(pattern.match);
            const varNames = extractVarNames(pattern.match);

            src = src.replace(regex, (...args) => {
                changed = true;

                const fullMatch = args[0];
                const captures = args.slice(1, -2);
                const matchStart = args[args.length - 2];
                const matchEnd = matchStart + fullMatch.length;

                // monta varMap
                const varMap = {};
                for (let i = 0; i < varNames.length; i++) {
                    varMap[varNames[i]] = captures[i] || '';
                }

                const _pre   = src.slice(0, matchStart);
                const _post  = src.slice(matchEnd);

                let result = pattern.replace;

                // --------------------------
                // substitui variáveis normais
                // --------------------------
                for (const [varName, value] of Object.entries(varMap)) {
                    const escaped = varName.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
                    result = result.replace(new RegExp(escaped + '(?![A-Za-z0-9_])', 'g'), value);
                }

                // unique
                result = result.replace(/\$unique\b/g, () =>
                    counterState.genUnique()
                );

                // concat ($$ → nada)
                result = result.replace(/\$\$/g, '');

                // ======================================================
                // PATCH ESSENCIAL:
                // detectar $clear ANTES de expandir $pre/$post/$match
                // evita que "$clear##$pre" vire "$clearasddasda",
                // o que impediria a regex de detectar.
                // ======================================================
                if (/\$clear\b/.test(result)) {
                    result = result.replace(/\$clear\b/g, '');
                    globalClearFlag = true;
                }

                // agora expande $pre/$post/$match
                result = result.replace(/\$pre\b/g, _pre);
                result = result.replace(/\$post\b/g, _post);
                result = result.replace(/\$match\b/g, fullMatch);

                lastResult = result;
                return result;
            });

            // aplica clear no final da rodada
            if (globalClearFlag) {
                src = lastResult;
                globalClearFlag = false;
                changed = true; // força nova rodada com src limpo
            }
        }
    }

    return src;
}



// === EXPANSÃO DE MACROS (MODIFICADA) ===
function expandMacros(src, macros) {
    for (const name of Object.keys(macros)) {
        const body = macros[name];
        let changed = true;
        let iterations = 0;
        
        while (changed && iterations < MAX_ITERATIONS) {
            changed = false;
            iterations++;
            const originalSrc = src;
            
            let i = 0;
            let result = '';
            
            while (i < src.length) {
                const remaining = src.substring(i);
                const nameMatch = remaining.match(new RegExp(`^(.*?)\\b${escapeRegex(name)}\\b`, 's'));
                
                if (!nameMatch) {
                    result += remaining;
                    break;
                }
                
                result += nameMatch[1];
                i += nameMatch[0].length;
                
                let k = i;
                while (k < src.length && src[k] === ' ') k++;
                
                let vals = [];
                
                if (k < src.length && src[k] === '(') {
                    const [argsStr, posAfter] = extractBlock(src, k, '(', ')');
                    vals = argsStr.split(',').map(v => v.trim());
                    i = posAfter;
                    changed = true;
                } else {
                    const spaceMatch = src.substring(i).match(/^(\s+([^\s{};()]+(?:\s+[^\s{};()]+)*?))?(?=\s*[{};()$]|\n|$)/);
                    if (spaceMatch && spaceMatch[2]) {
                        vals = spaceMatch[2].split(/\s+/);
                        i += spaceMatch[0].length;
                        changed = true;
                    } else {
                        result += name;
                        continue;
                    }
                }
                
                let exp = body;
                exp = exp.replace(/\$0\b/g, name);
                for (let j = 0; j < vals.length; j++) {
                    const paramNum = j + 1;
                    const paramVal = vals[j];
                    exp = exp.replace(new RegExp(`\\$${paramNum}(?!\\d)`, 'g'), paramVal);
                }
                exp = exp.replace(/\$\d+\b/g, '');
                result += exp;
            }
            
            src = result;
            
            if (src === originalSrc) {
                changed = false;
            }
        }
    }
    return src;
}

function escapeRegex(str) {
    return str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

function collapseLocalNewlines(left, right) {
    // remove excesso no fim do trecho anterior
    left = left.replace(/\n+$/, '\n');
    // remove excesso no começo do trecho posterior
    right = right.replace(/^\n+/, '\n');
    
    // se ambos começam/terminam com \n, acaba duplicando.
    // força ficar apenas UM \n entre eles.
    if (left.endsWith('\n') && right.startsWith('\n')) {
        right = right.replace(/^\n+/, '\n');
    }
    
    // remove newline inicial se não houver nada antes
    if (left === '' && right.startsWith('\n')) {
        right = right.replace(/^\n+/, '');
    }

    return left + right;
}

function processEvalBlocks(src) {
    const evalRegex = new RegExp(`\\beval\\s*\\${OPEN}`, "g");

    let match;
    const matches = [];

    while ((match = evalRegex.exec(src)) !== null) {
        matches.push({
            matchStart: match.index,
            openPos: match.index + match[0].length - 1
        });
    }

    for (let j = matches.length - 1; j >= 0; j--) {
        const m = matches[j];

        const [content, posAfter] = extractBlock(src, m.openPos, OPEN, CLOSE);

        let out = "";
        try {
            out = String(
                Function("painho", "ctx", `"use strict"; return (${content});`)
                (painho, { })
            );
        } catch (e) {
            out = "";
        }

        let left = src.substring(0, m.matchStart);
        let right = src.substring(posAfter);
        src = left + out + right;
    }

    return src;
}

// === PROCESSAMENTO DE BLOCOS ISOLADOS ===
function processNamespaceBlocks(src) {
    const namespaceRegex = new RegExp(`\\bnamespace\\s*\\${OPEN}`, "g");

    let match;
    const matches = [];

    while ((match = namespaceRegex.exec(src)) !== null) {
        matches.push({
            matchStart: match.index,
            openPos: match.index + match[0].length - 1
        });
    }

    for (let j = matches.length - 1; j >= 0; j--) {
        const m = matches[j];
        const [content, posAfter] = extractBlock(src, m.openPos, OPEN, CLOSE);

        const processedContent = painho(content);

        let left = src.substring(0, m.matchStart);
        let right = src.substring(posAfter);

        let prefix = "";
        if (left.endsWith("\n")) {
            prefix = "\n";
            left = left.slice(0, -1);
        }

        src = left + prefix + processedContent + right;
    }

    return src;
}


function painho(input) {
    let src = input;

    src = processNamespaceBlocks(src);
    src = processEvalBlocks(src);        // << novo bloco

    const [macros, srcAfterMacros] = collectMacros(src);
    src = srcAfterMacros;

    const [patterns, srcAfterPatterns] = collectPatterns(src);
    src = srcAfterPatterns;

    src = applyPatterns(src, patterns);
    src = expandMacros(src, macros);

    return src;
}


// Export para diferentes ambientes
if (typeof module !== 'undefined' && module.exports) {
    // Node.js / CommonJS
    module.exports = { painho };
} else if (typeof exports !== 'undefined') {
    // Browser / QuickJS
    exports.painho = painho;
}