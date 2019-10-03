all: gfxtoc_sa recsym.c vu.c vlsi.c # gfxtoc

#gfxtoc: gfxtoc.c
#	gcc -g -Wall -funsigned-char -O -o $@ $< -lm -lleo
#	@ls -la $@

gfxtoc_sa: gfxtoc_sa.c
	gcc -g -Wall -funsigned-char -O -o $@ $< -lm
	@ls -la $@

recsym.c: recsym.ppm gfxtoc_sa Makefile
#	Uncompressed Rec/Pause/Playback symbols
	./gfxtoc_sa recsym.ppm recsym 6 8 8 1 1 10 1 19 1 28 1 37 1 46 1

vu.c: vu.ppm gfxtoc_sa Makefile
#	Compressed VU meter graphics
	./gfxtoc_sa vu.ppm vu -1 143 44 0 0

vlsi.c: vlsi.ppm gfxtoc_sa Makefile
#	Compressed VLSI logo
	./gfxtoc_sa vlsi.ppm vlsi -1 160 30 0 0

clean:
	rm -rf Emulation-Debug _* loadable.* *~ \
		*/Emulation-Debug */_* */loadable.* */*~
