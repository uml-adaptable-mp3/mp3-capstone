all: gfxtoc recsym.c vu.c vlsi.c qsym.c

gfxtoc: gfxtoc.c
	gcc -g -Wall -funsigned-char -O -o $@ $< -lm -lleo
	@ls -la $@

recsym.c: recsym.ppm gfxtoc Makefile
#	Uncompressed Rec/Pause/Playback symbols
	./gfxtoc recsym.ppm recsym 4 8 8 1 1 10 1 19 1 28 1

vu.c: vu.ppm gfxtoc Makefile
#	Compressed VU meter graphics
	./gfxtoc vu.ppm vu -1 143 44 0 0

vlsi.c: vlsi.ppm gfxtoc Makefile
#	Compressed VLSI logo
	./gfxtoc vlsi.ppm vlsi -1 160 30 0 0

clean:
	rm -rf Emulation-Debug _* loadable.* *~ \
		*/Emulation-Debug */_* */loadable.* */*~
