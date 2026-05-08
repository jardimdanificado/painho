#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import Papagaio from '../dist/wasm/papagaio.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

async function main() {
    const args = process.argv.slice(2);
    
    // Help / Version
    if (args.includes('--version') || args.includes('-v')) {
        const pkg = JSON.parse(fs.readFileSync(path.join(__dirname, '../package.json'), 'utf-8'));
        console.log(`papagaio v${pkg.version}`);
        process.exit(0);
    }

    if (args.includes('--help') || args.includes('-h')) {
        console.log("Papagaio - easy yet powerful preprocessor");
        console.log("Usage: papagaio <file.txt> [key=value ...]");
        process.exit(0);
    }

    if (args.length < 1) {
        console.error("Usage: papagaio <file.txt> [key=value ...]");
        process.exit(1);
    }

    const filePath = args[0];
    if (!fs.existsSync(filePath)) {
        console.error(`Error reading file: ${filePath}`);
        process.exit(1);
    }

    const input = fs.readFileSync(filePath, 'utf-8');
    const p = new Papagaio();
    await p.init();
    
    // Set arguments in context (mimic C main.c behavior)
    // argv[0] = papagaio, argv[1] = file, argv[2...] = extras
    p.setArgs(process.argv.slice(1)); 

    const output = p.process(input);
    process.stdout.write(output);
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
