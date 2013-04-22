# Autor: Martin Hruska
# E-mail: xhrusk16@stud.fit.vutbr.cz

PROJ=obhajoba
TEX_COMP=latex # prekladac latexu
DVI=dvips # prevod do post scriptu
PDF=ps2pdf #prevod do pdf
A4=-sPAPERSIZE=a4
VLNA=vlna
VLNA_OPTS=-l -m -n

all:
	$(VLNA) $(VLNA_OPTS) $(PROJ).tex 
	$(TEX_COMP) $(PROJ).tex
	$(TEX_COMP) $(PROJ).tex
	$(TEX_COMP) $(PROJ).tex
	$(DVI) $(PROJ).dvi
	$(PDF) $(A4) $(PROJ).ps
