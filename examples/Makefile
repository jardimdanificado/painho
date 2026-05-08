# Papagaio Examples Makefile
# This Makefile replaces the functionality of the now-deprecated papagaiocc tool.

PAP_BIN = ../papagaio
CLANG   = clang
WASM_FLAGS = --target=wasm32-unknown-unknown -ffreestanding -nostdlib -fvisibility=hidden -Wl,--no-entry,--export-dynamic,--allow-undefined

EXAMPLES = hello_libc.wasm simple_plugin.wasm modern_plugin.wasm

all: $(EXAMPLES)

%.wasm: %.c
	@echo "▶ Preprocessing $< with Papagaio..."
	$(PAP_BIN) $< > $*.ready.c
	@echo "▶ Compiling $*.ready.c to WASM..."
	$(CLANG) $(WASM_FLAGS) $*.ready.c -o $@
	@rm $*.ready.c
	@echo "✓ $@ generated."

clean:
	rm -f $(EXAMPLES) *.ready.c
