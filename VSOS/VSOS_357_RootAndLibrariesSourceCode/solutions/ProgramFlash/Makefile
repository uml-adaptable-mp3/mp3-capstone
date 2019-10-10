all: preg.dat

buildpregdat: buildpregdat.c
	gcc -O -o $@ $<

preg.db: Makefile buildpregdb.pl
	./buildpregdb.pl < vs1005g.h > preg.db

preg.dat: Makefile preg.db buildpregdat
	./buildpregdat < preg.db > preg.dat

clean:
	rm -rf Emulation-Debug _* loadable.* *~ \
		*/Emulation-Debug */_* */loadable.* */*~ \
		buildpregdat
