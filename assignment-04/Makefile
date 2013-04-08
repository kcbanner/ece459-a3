CFLAGS?=-Wall -O2 -lm -lOpenCL

default: all

# modify to generate parallel and optimized versions!
all: bin bin/nbody-seq bin/nbody bin/nbody-opt

report: report.pdf

bin:
	mkdir bin

bin/nbody-seq: src/nbody-seq.c
	$(CC) $(CFLAGS) -o bin/nbody-seq $<

bin/nbody: src/nbody-ocl.c
	$(CXX) $(CFLAGS) -o bin/nbody $<

bin/nbody-opt: src/nbody-ocl-ff.c
	$(CXX) $(CFLAGS) -o bin/nbody-opt $<

report.pdf: report/report.tex
	cd report && pdflatex report.tex && pdflatex report.tex
	mv report/report.pdf report.pdf

clean:
	rm -r bin
