import {
  Plugin,
  Notice,
  MarkdownView
} from "obsidian";

// @ts-ignore
import createPapagaioModule from "../../dist/wasm/papagaio_wasm.js";

class LuaEngine {
  constructor() {
    this.mod = null;
    this.ready = false;
    this.initPromise = null;
  }

  async init() {
    if (this.ready) return;
    if (this.initPromise) return this.initPromise;

    this.initPromise = (async () => {
      try {
        this.mod = await createPapagaioModule();
        this.ready = true;
      } catch (e) {
        console.error("papagaio: failed to init WASM", e);
        throw e;
      }
    })();

    return this.initPromise;
  }

  async processText(text) {
    if (!this.mod || !this.ready) await this.init();
    try {
      // Use ccall with 'number' to get the pointer so we can free it
      const ptr = this.mod.ccall(
        "papagaio_process_text", 
        "number", 
        ["number", "string", "number"], 
        [0, text, text.length]
      );
      
      if (ptr === 0) return "";
      
      const result = this.mod.UTF8ToString(ptr);
      this.mod._free(ptr);
      return result;
    } catch (e) {
      return `[ERROR] ${e.message || e}`;
    }
  }
}

export default class PapagaioPlugin extends Plugin {
  async onload() {
    this.engine = new LuaEngine();

    // Toolbar button (ribbon icon)
    this.addRibbonIcon("play", "Papagaio: Process File", async () => {
      await this.processCurrentFile();
    });

    // Command
    this.addCommand({
      id: "papagaio-process-file",
      name: "Process current file with Papagaio (.processed.md)",
      callback: () => this.processCurrentFile(),
    });

    console.log("Papagaio plugin loaded");
  }

  async processCurrentFile() {
    const view = this.app.workspace.getActiveViewOfType(MarkdownView);
    if (!view) {
      new Notice("No active markdown note");
      return;
    }

    const file = view.file;
    if (!file || !file.path.endsWith(".md")) {
      new Notice("Not a markdown file");
      return;
    }

    const content = view.editor.getValue();
    new Notice("🦜 Processing...");

    try {
      const output = await this.engine.processText(content);
      
      const newPath = file.path.replace(/\.md$/, ".processed.md");
      const existing = this.app.vault.getAbstractFileByPath(newPath);
      
      if (existing) {
        await this.app.vault.modify(existing, output);
      } else {
        await this.app.vault.create(newPath, output);
      }
      
      new Notice(`🦜 Done! Created ${newPath}`);
    } catch (err) {
      new Notice(`❌ Error: ${err.message}`);
      console.error(err);
    }
  }
}
