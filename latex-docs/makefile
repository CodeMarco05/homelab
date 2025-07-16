# Always compile main.tex, regardless of which file is changed or where the command is run from
MAIN=main

all:
	latexmk -pdf $(MAIN).tex

clean:
	latexmk -c

cleanall:
	latexmk -C

.PHONY: all clean cleanall