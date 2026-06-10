import ModuleFactory from './papagaio_wasm.js';

class Papagaio {
  constructor() {
    this._module = null;
    this._ctx = null;
    this._initialized = false;
    this._args = [];
  }

  async init() {
    if (this._initialized) return this;
    
    // Load WASM module
    this._module = await ModuleFactory();
    this._ctx = this._module._papagaio_open();
    this._initialized = true;
    return this;
  }

  registerCommand(name, handler) {
    // Note: Implementing JS->C callbacks requires addFunction and extra glue.
  }

  setArgs(argv) {
    if (!this._initialized) throw new Error("Papagaio not initialized. Call init() first.");
    this._args = argv;

    const argc = argv.length;
    const argvPtr = this._module._malloc(argc * 4); 
    for (let i = 0; i < argc; i++) {
        const str = argv[i];
        const strLen = this._module.lengthBytesUTF8(str) + 1;
        const strPtr = this._module._malloc(strLen);
        this._module.stringToUTF8(str, strPtr, strLen);
        this._module.setValue(argvPtr + (i * 4), strPtr, 'i32');
    }

    this._module._papagaio_set_args(this._ctx, argc, argvPtr);
    // We don't free argvPtr/strPtr here as Papagaio context might use them.
    // They will be "leaked" until destroy(), which is fine for a processing session.
  }

  process(text) {
    if (!this._initialized) throw new Error("Papagaio not initialized. Call init() first.");

    const textLen = this._module.lengthBytesUTF8(text);
    const textPtr = this._module._malloc(textLen + 1);
    this._module.stringToUTF8(text, textPtr, textLen + 1);

    const outPtr = this._module._papagaio_process_text(this._ctx, textPtr, textLen);
    const output = this._module.UTF8ToString(outPtr);

    this._module._free(textPtr);
    this._module._free(outPtr);

    return output;
  }

  destroy() {
    if (this._ctx) {
      this._module._papagaio_close(this._ctx);
      this._ctx = null;
    }
  }
}

export default Papagaio;
