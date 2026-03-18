import createWasmModule from './papagaio_wasm.js';

class Papagaio {
  constructor() {
    this._module = null;
    this._initialized = false;
    this._initPromise = this.init();
  }

  async init() {
    if (this._initialized) return;
    this._module = await createWasmModule();
    this._initialized = true;
    return this;
  }

  process(text) {
    if (!this._initialized) {
      throw new Error("Papagaio wasm not initialized. Use 'await papagaio.init()'");
    }
    // Use 'number' return type to get the pointer so we can free it.
    const ptr = this._module.ccall(
      "papagaio_process_text", 
      "number", 
      ["number", "string", "number"], 
      [0, text, text.length]
    );

    if (ptr === 0) return "";
    
    // Convert C string to JS string
    const result = this._module.UTF8ToString(ptr);
    
    // Free the C string
    this._module._free(ptr);
    
    return result;
  }
}

export default Papagaio;
