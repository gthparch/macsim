RM = /bin/rm -rf


all: 
	pdflatex -output-format=pdf -jobname=macsim main; 
	bibtex macsim; 
	pdflatex -output-format=pdf -jobname=macsim main; 
	pdflatex -output-format=pdf -jobname=macsim main; 
	cp macsim.pdf ../;

clean: 
	$(RM) *.log *.aux *.blg *.bbl *.dvi *.brf macsim.ps macsim.pdf
