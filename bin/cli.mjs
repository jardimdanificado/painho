#!/usr/bin/env node

import fs from "fs";
import papagaio from "../papagaio.js";

async function main() {
  const args = process.argv.slice(2);

  const pkgPath = new URL("../package.json", import.meta.url);
  const pkg = JSON.parse(fs.readFileSync(pkgPath, "utf8"));
  const VERSION = pkg.version;

  if (args.includes("-v") || args.includes("--version")) {
    console.log(VERSION);
    process.exit(0);
  }

  if (args.includes("-h") || args.includes("--help")) {
    console.log(`Usage: papagaio [options] <file1> [file2] [...]

Options:
  -h, --help            Show this help message
  -v, --version         Show version number
  --json                Output JSON (default)
  --md                  Output Markdown
  -o, --out <file>      Write output to file instead of stdout

Examples:
  papagaio input.txt
  papagaio --md input.md
  papagaio --out out.json input.md`);
    process.exit(0);
  }

  const format = args.includes("--md") ? "md" : "json";

  let outPath = null;
  const files = [];
  for (let i = 0; i < args.length; i++) {
    const arg = args[i];
    if (arg === "-o" || arg === "--out") {
      outPath = args[i + 1];
      i++;
      continue;
    }
    if (arg === "--md" || arg === "--json" || arg === "-h" || arg === "--help" || arg === "-v" || arg === "--version") {
      continue;
    }
    files.push(arg);
  }

  if (files.length === 0) {
    console.error("Error: no input file specified.\nUse --help for usage.");
    process.exit(1);
  }

  let concatenatedSrc = "";
  let hasErrors = false;

  for (const file of files) {
    try {
      if (!fs.existsSync(file)) {
        throw new Error(`file not found: ${file}`);
      }
      concatenatedSrc += fs.readFileSync(file, "utf8");
    } catch (err) {
      console.error(`Error reading ${file}: ${err.message || err}`);
      hasErrors = true;
    }
  }

  if (hasErrors) {
    process.exit(1);
  }

  const { papagaio_md_to_object, papagaio_md_to_markdown } = papagaio;

  const mdObj = papagaio_md_to_object(concatenatedSrc);
  let out;
  if (format === "md") {
    out = papagaio_md_to_markdown(mdObj);
  } else {
    out = JSON.stringify(mdObj, null, 2);
  }

  if (outPath) {
    fs.writeFileSync(outPath, out, "utf8");
    console.log(`Wrote output to ${outPath}`);
  } else {
    process.stdout.write(out + "\n");
  }
}

main().catch((err) => {
  console.error("Fatal error:", err.message || err);
  process.exit(1);
});
