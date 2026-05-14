import { spawnSync } from 'child_process';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const PAP_BIN = path.join(__dirname, '..', 'papagaio');

class Papagaio {
  constructor() {
    this._initialized = false;
    this._args = [];
  }

  async init() {
    this._initialized = true;
    return this;
  }

  registerCommand(name, handler) {
    // Native CLI wrapper doesn't support registering JS commands directly yet.
    // We'll ignore this for now as the tests use it for 'lua' which is handled by mockEval in test.js
    console.warn(`[JS] Warning: registerCommand('${name}') is not supported in native CLI wrapper.`);
  }

  setArgs(argv) {
    this._args = argv;
  }

  process(text) {
    const args = ['-e', text];
    if (this._args && this._args.length > 0) {
        // Papagaio CLI might need to handle args. 
        // For now, we just pass the text.
    }

    const result = spawnSync(PAP_BIN, args, {
        input: '', // No stdin for -e mode usually
        encoding: 'utf-8'
    });

    if (result.error) {
        throw new Error(`Failed to run papagaio: ${result.error.message}`);
    }

    return result.stdout;
  }

  destroy() {
    // Nothing to do for CLI wrapper
  }
}

export default Papagaio;
