import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

let Papagaio = (await import('../dist/wasm/papagaio.js')).default;
const p = new Papagaio();
await p.init();

// Step 1: Code Generation
const template = `
$pattern{VARS}{$let $name = $val;}
VARS
`;

// Wait, the user wants a test where code is generated and saved.
const codeGenTemplate = `
$pattern{COMPONENT($name)}{
  $pattern{HTML_ELEMENT}{<div>Hello $name</div>}
  HTML_ELEMENT
}
COMPONENT(World)
`;

const generatedCode = p.process(codeGenTemplate).trim();
console.log("--- Generated Code ---");
console.log(generatedCode);

// Step 2: Save to file
const filePath = path.join(process.cwd(), 'tests', 'generated.pap');
fs.writeFileSync(filePath, generatedCode, 'utf8');
console.log(`\nSaved to: ${filePath}`);

// Step 3: Run the generated code in Papagaio
const p2 = new Papagaio();
await p2.init();
const finalOutput = p2.process(fs.readFileSync(filePath, 'utf8')).trim();
console.log("\n--- Final Execution Output ---");
console.log(finalOutput);

if (finalOutput === "<div>Hello World</div>") {
    console.log("\n[PASS] Code Generation Test Successful!");
    process.exit(0);
} else {
    console.error("\n[FAIL] Output mismatch!");
    process.exit(1);
}
