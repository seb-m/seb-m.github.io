DIST_NAME = resume
SRC = kheir-martini-tiberghien-resume.tex

LATEX = latex
BIB = bibtex
CONV = dvipdf

AUX = $(SRC:.tex=.aux)
BBL = $(SRC:.tex=.bbl)
TMP = $(SRC:.tex=.aux.bbl)
DVI = $(SRC:.tex=.dvi)
PDF = $(SRC:.tex=.pdf)
PS = $(SRC:.tex=.ps)

all: bib

base:
	$(LATEX) $(SRC)

bib: base
	$(BIB) $(AUX)
	$(LATEX) $(SRC)
	$(LATEX) $(SRC)

pdf: all
	$(CONV) $(DVI)

ps: all
	dvips $(DVI)

pdflatex : all
	pdflatex $(SRC)

clean:
	rm -f *~
	rm -f *#
	rm -f *.tmp
	rm -f *.aux
	rm -f *.lo*
	rm -f *.dvi
	rm -f *.mtc*
	rm -f *.blg
	rm -f *.toc
	rm -f *.bbl
	rm -f *.out
	rm -rf ./tmp/
	rm -rf ./figure/*.bak

dist-clean: clean
	rm -f *.pdf
	rm -f *.ps

re: clean all

dist : dist-clean
	mkdir $(DIST_NAME)
	cp -R Makefile figure/ *.tex *.bib *.sty $(DIST_NAME)
	tar -cvf $(DIST_NAME).tar $(DIST_NAME)
	bzip2 -f $(DIST_NAME).tar
	rm -rf $(DIST_NAME)

move : dist
	mv $(DIST_NAME).tar.bz2 ~/Desktop/desk/

