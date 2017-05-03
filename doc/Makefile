# Makefile for the amask document in LaTex format.

version=1.0
Mug =  amask_ug
#default: $(Mug).pdf

INTERMEDIATE_FILES=$(Mug).pdf \
                   $(Mug).toc \
                   $(Mug).out \
                   $(Mug).log \
                   $(Mug).aux \


#.PHONY : $(Mug).pdf
$(Mug).pdf: 
	rm -f $(INTERMEDIATE_FILES)
	pdflatex -interaction=batchmode -file-line-error $(Mug).tex
	pdflatex -interaction=batchmode -file-line-error $(Mug).tex
	pdflatex -interaction=batchmode -file-line-error $(Mug).tex
	cp $(Mug).pdf $(Mug)-$(version).pdf

clean:
	rm -f $(INTERMEDIATE_FILES)
