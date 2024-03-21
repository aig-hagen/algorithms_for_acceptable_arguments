all: solver

solver:
	$(MAKE) -C src all
	mv src/cgss2 cgss2

.PHONY: lib
lib:
	$(MAKE) -C src lib

clean:
	$(MAKE) -C src clean
