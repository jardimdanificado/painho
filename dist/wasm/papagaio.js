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
    this._ctx = this._module._papagaio_open();
    this._initialized = true;
    return this;
  }

  registerCommand(name, handler) {
    if (!this._initialized) throw new Error("Not initialized");
    
    // name=i, argc=ii, argv=iii, argl=iiii, userdata=iiiii
    const wrapper = (ctx, namePtr, argc, argvPtr, arglPtr, userdata) => {
        const cmdName = this._module.UTF8ToString(namePtr);
        const args = [];
        for (let i = 0; i < argc; i++) {
            const ptr = this._module.getValue(argvPtr + (i * 4), "i32");
            const len = this._module.getValue(arglPtr + (i * 4), "i32");
            args.push(this._module.UTF8ToString(ptr, len));
        }
        const result = handler(cmdName, ...args);
        
        if (result === null || result === undefined) return 0;
        const resStr = String(result);
        const resLen = this._module.lengthBytesUTF8(resStr) + 1;
        const resPtr = this._module._malloc(resLen);
        this._module.stringToUTF8(resStr, resPtr, resLen);
        return resPtr;
    };

    const funcPtr = this._module.addFunction(wrapper, 'iiiiiii');
    this._module.ccall(
      "papagaio_register_command",
      null,
      ["number", "string", "number", "number"],
      [this._ctx, name, funcPtr, 0]
    );
  }

  setArgs(argv) {
    if (!this._initialized) throw new Error("Not initialized");
    if (!Array.isArray(argv)) throw new Error("argv must be an array");

    /* Create a null-terminated array of string pointers in Wasm memory */
    const ptrs = argv.map(str => {
      const len = this._module.lengthBytesUTF8(str) + 1;
      const ptr = this._module._malloc(len);
      this._module.stringToUTF8(str, ptr, len);
      return ptr;
    });

    const argc = argv.length;
    const argvPtr = this._module._malloc(argc * 4);
    for (let i = 0; i < argc; i++) {
        this._module.setValue(argvPtr + (i * 4), ptrs[i], "i32");
    }

    this._module._papagaio_set_args(this._ctx, argc, argvPtr);
    /* We don't free ptrs/argvPtr immediately as Papagaio might need them during processing.
       In a production-ready class, we should track these for cleanup. */
  }

  process(text) {
    if (!this._initialized) {
      throw new Error("Papagaio wasm not initialized. Use 'await papagaio.init()'");
    }
    const ptr = this._module.ccall(
      "papagaio_process_text", 
      "number", 
      ["number", "string", "number"], 
      [this._ctx, text, text.length]
    );

    if (ptr === 0) return "";
    const result = this._module.UTF8ToString(ptr);
    this._module._free(ptr);
    return result;
  }

  destroy() {
    if (this._ctx) {
      this._module._papagaio_close(this._ctx);
      this._ctx = null;
    }
  }
}

export default Papagaio;
