all: analyzerds intcheck

analyzerds: analyzerds.c
	gcc -O -o $@ $<

intcheck: intcheck.c
	gcc -O -o $@ $<

clean:
	rm -rf Emulation-Debug _* loadable.* *~ \
		*/Emulation-Debug */_* */loadable.* */*~
