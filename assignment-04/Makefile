CFLAGS?=-Wall -O2 -lm -lOpenCL

default: all

# modify to generate parallel and optimized versions!
all: bin bin/nbody-seq bin/nbody-ocl

report: report.pdf

bin:
	mkdir bin

bin/nbody-seq: src/nbody-seq.c
	$(CC) $(CFLAGS) -o bin/nbody-seq $<

bin/nbody-ocl: src/nbody-ocl.c
	$(CXX) $(CFLAGS) -o bin/nbody-ocl $<

report.pdf: report/report.tex
	cd report && pdflatex report.tex && pdflatex report.tex
	mv report/report.pdf report.pdf

clean:
	rm -r bin
