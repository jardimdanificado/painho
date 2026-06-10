PAPAGAIO = ../papagaio

.PHONY: all test test_scripts test_plugins test_louco_aot clean

all: test

test: test_scripts test_plugins test_louco_aot
	@echo "All examples tested successfully!"

test_scripts:
	@echo "--- Testing calculator.pap ---"
	$(PAPAGAIO) calculator.pap > /dev/null
	@echo "--- Testing rpg.pap ---"
	$(PAPAGAIO) rpg.pap > /dev/null

test_plugins:
	@echo "--- Building modifiers plugins ---"
	$(MAKE) -C modifiers
	@echo "--- Building tcc plugin ---"
	$(MAKE) -C tcc

test_louco_aot:
	@echo "--- Building and testing louco_aot ---"
	$(MAKE) -C louco_aot
	cd louco_aot && ../$(PAPAGAIO) aot_template.pap "$$(cat script.txt)" > /dev/null

clean:
	$(MAKE) -C modifiers clean
	$(MAKE) -C tcc clean
	$(MAKE) -C louco_aot clean
