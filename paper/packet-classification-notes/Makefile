packet-classification.pdf: packet-classification.dvi
	dvipdf $< $@

packet-classification.dvi: packet-classification.tex packet-classification.bib
	latex packet-classification
	bibtex packet-classification
	latex packet-classification
	latex packet-classification

clean:
	/bin/rm -f \
		comment.cut \
		packet-classification.aux \
		packet-classification.bbl \
		packet-classification.blg \
		packet-classification.dvi \
		packet-classification.log \
		packet-classification.out \
		packet-classification.pdf \
		packet-classification.xcp
