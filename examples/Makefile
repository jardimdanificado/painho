PAPAGAIO = ../papagaio

.PHONY: all test test_scripts test_plugins test_louco clean

all: test

test: test_scripts test_plugins test_louco
	@echo "All examples tested successfully!"

test_scripts:
	@echo "--- Testing calculator.pap ---"
	$(PAPAGAIO) calculator.pap > /dev/null
	@echo "--- Testing rpg.pap ---"
	$(PAPAGAIO) rpg.pap > /dev/null

test_plugins:
	@echo "--- Building modifiers plugins ---"
	$(MAKE) -C modifiers
	@echo "--- Building math plugin ---"
	$(MAKE) -C math
	@echo "--- Building tcc plugin ---"
	$(MAKE) -C tcc
	@echo "--- Building wasm plugin ---"
	$(MAKE) -C wasm

test_louco:
	@echo "--- Building and testing louco ---"
	$(MAKE) -C louco
	cd louco && ../$(PAPAGAIO) aot_template.pap "$$(cat script.txt)" > /dev/null

clean:
	$(MAKE) -C modifiers clean
	$(MAKE) -C math clean
	$(MAKE) -C tcc clean
	$(MAKE) -C louco clean
	$(MAKE) -C wasm clean
