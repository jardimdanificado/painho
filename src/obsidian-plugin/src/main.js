import {
  Plugin,
  Notice,
  MarkdownView,
  ItemView
} from "obsidian";

// @ts-ignore
import createPapagaioModule from "../../../dist/wasm/papagaio_wasm.js";

const PAPAGAIO_OUTPUT_VIEW = "papagaio-output-view";

class PapagaioOutputView extends ItemView {
  constructor(leaf) {
    super(leaf);
    this.textarea = null;
  }

  getViewType() {
    return PAPAGAIO_OUTPUT_VIEW;
  }

  getDisplayText() {
    return "Papagaio Output";
  }

  async onOpen() {
    this.contentEl.empty();
    this.textarea = this.contentEl.createEl("textarea", {
      attr: {
        placeholder: "Papagaio output appears here (editable)",
      },
    });
    this.textarea.style.width = "100%";
    this.textarea.style.height = "100%";
    this.textarea.style.boxSizing = "border-box";
    this.textarea.style.resize = "none";
    this.textarea.style.padding = "0.8rem";
    this.textarea.style.fontFamily = "var(--font-family)";
    this.textarea.style.fontSize = "var(--font-size)";
    this.textarea.style.border = "1px solid var(--interactive-border)";
  }

  onClose() {
    this.textarea = null;
  }

  setText(value) {
    if (!this.textarea) return;
    this.textarea.value = value;
    this.textarea.focus();
  }
}

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

    this.registerView(PAPAGAIO_OUTPUT_VIEW, (leaf) => new PapagaioOutputView(leaf));

    // Toolbar button (ribbon icon)
    this.addRibbonIcon("play", "Papagaio: Process File", async () => {
      await this.processCurrentFile();
    });

    // Command
    this.addCommand({
      id: "papagaio-process-file",
      name: "Process current file with Papagaio (open output in right pane)",
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

      const leaf = this.app.workspace.getRightLeaf(false);
      await leaf.setViewState({
        type: PAPAGAIO_OUTPUT_VIEW,
        active: true,
      });
      this.app.workspace.revealLeaf(leaf);

      const view = leaf.view;
      if (view && typeof view.setText === "function") {
        view.setText(output);
      }

      new Notice("🦜 Done! Output opened in right pane");
    } catch (err) {
      new Notice(`❌ Error: ${err.message}`);
      console.error(err);
    }
  }
}
