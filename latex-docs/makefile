# Always compile main.tex, regardless of which file is changed or where the command is run from
MAIN=main

all:
	latexmk -pdf $(MAIN).tex

clean:
	latexmk -c
	find . -name '*.aux' -delete
	rm -f $(MAIN).bbl

cleanall:
	latexmk -C
	find . -name '*.aux' -delete
	rm -f $(MAIN).bbl

.PHONY: all clean cleanall