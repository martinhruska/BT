PROJ=obhajoba
TEX_COMP=pdflatex # prekladac latexu
BIB=bibtex
DVI=dvips # prevod do post scriptu
PDF=pdflatex #prevod do pdf
$A4=-sPAPERSIZE=a4
VLNA=vlna
VLNA_OPTS=-l -m -n

all:
	$(VLNA) $(VLNA_OPTS) $(PROJ).tex 
	$(PDF) $(PROJ).tex
	$(PDF) $(PROJ).tex
