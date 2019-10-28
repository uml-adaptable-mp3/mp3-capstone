#define _CodMpgDecode 0xd2dd

	.sect data_y,data_y
	.export _mp3PatchArea
_mp3PatchArea:
	.bss 6

	.sect code,CodMpgDecodePatch
	.export _CodMpgDecodePatch
_CodMpgDecodePatch:
	ldc _mp3PatchArea,i7
	sty lr0,(i7)+1
	sty lc,(i7)+1
	sty ls,(i7)+1
	sty le,(i7)

	ldc 0xe1d9,le
	ldc CodMpgDecodeActualPatch,ls
	ldc 1,lc

//	.import _CodMpgDecode
	call _CodMpgDecode
	nop

	ldc _mp3PatchArea,i7
	ldy (i7)-1,lr0
	ldy (i7)-1,lc
	ldy (i7)-1,ls
	ldy (i7)-1,le
	jr
	nop

	.export CodMpgDecodeActualPatch
CodMpgDecodeActualPatch:
//	STX A0,(I4)+2	// A0 = gr = 0 (ROM 0xe1d9)

	ldc 0xe1d9,le
	ldc CodMpgDecodeActualPatch,ls
	ldc 1,lc

#if 0
	ldc _mp3PatchArea+4,i7
	ldy (i7),a1
	sub a1,ones,a1
	sty a1,(i7)
#endif

	ldx (i4)-6,null
	ldx (i4)+6,i7
	ldx (i7)+5,null	//sizof(struct Codec) = 5
	ldx (i7),a1
	add a1,null,a1
	nop
	jnc $5
	nop
	ldc 16,b0
	add a1,b0,a1
	stx a1,(i7)+1
	ldx (i7),a1
	add a1,ones,a1
	stx a1,(i7)

#if 1
#if 0
	nop
	nop
	nop
	nop
#else
	ldc _mp3PatchArea+4,i7
	ldy (i7),a1
	sub a1,ones,a1
	sty a1,(i7)
#endif
#endif
$5:

	//gr already in a0
	LDX (I4)-3,A1	// A1 = granules
	SUB A0,A1,B0
	NOP
	JGE 0xe38f       /* section MpgLayer3 + 0x281 */
	ADD A1,ONES,A1

//	LOOP A1,0xe38e   /* section MpgLayer3 + 0x10280 */
//	NOP

	ldc 0xe20e,le
	ldc 0x7fff,lc
	j 0xe1e1	// Loop start in ROM
	ldc $1,ls

$1:
L0xe20f:	LDX (I1),B0
L0xe210:	STX B0,(I6)+1
L0xe211:	LDX (I4)+3,B1
L0xe212:	ADD B1,A0,B1
	STX B1,(I6)+1; sty b1,(i6)
	mv b1,i7
	ldx (i7),a0
	add a0,null,a0
	nop
	jnc $2
	nop
	ldc 16,b1
	add a0,b1,a0
	stx a0,(i7)+1
	ldx (i7),a0
	add a0,ones,a0
	stx a0,(i7)

#if 1
	ldc _mp3PatchArea+4,i7
	ldy (i7),a0
	sub a0,ones,a0
	sty a0,(i7)
#endif
	
$2:
L0xe213:	CALL 0xdbcd      /* _III_huffman_decode_32y */
			nop
	LDY (I6)-3,i7 ; LDX (I4)+6,NULL

	ldx (i7),a0
	add a0,null,a0
	ldc 16,b0
	jnc $3
	add a0,b0,a0

	stx a0,(i7)+1
	ldx (i7),a0
	add a0,ones,a0
	stx a0,(i7)

#if 1
	ldc _mp3PatchArea+4,i7
	ldy (i7),a0
	add a0,ones,a0
	sty a0,(i7)
#endif
$3:
	ldc 0xe38e,le
	ldc $4,ls
	j 0xe216
	nop

$4:

	j CodMpgDecodeActualPatch
	ldx (i4)+3,null






		.sect data_y,const_cos12_y
		.org 0x4b2

_cos12Tab:
		.word 0x4dec /* 19948     */
		.word 0x89be /*-30274     */
		.word 0xef4b /* -4277     */
		.word 0x7ee8 /* 32488     */
		.word 0xcf04 /*-12540     */
		.word 0x9a73 /*-25997     */
		.word 0x30fc /* 12540     */
		.word 0x89be /*-30274     */
		.word 0x7642 /* 30274     */
		.word 0xcf04 /*-12540     */
		.word 0xcf04 /*-12540     */
		.word 0x7642 /* 30274     */
		.word 0x10b5 /*  4277     */
		.word 0xcf04 /*-12540     */
		.word 0x4dec /* 19948     */
		.word 0x9a73 /*-25997     */
		.word 0x7642 /* 30274     */
		.word 0x8118 /*-32488     */
		.word 0x9a73 /*-25997     */
		.word 0x30fc /* 12540     */
		.word 0x7ee8 /* 32488     */
		.word 0x10b5 /*  4277     */
		.word 0x89be /*-30274     */
		.word 0xb214 /*-19948     */
		.word 0x89be /*-30274     */
		.word 0xcf04 /*-12540     */
		.word 0x30fc /* 12540     */
		.word 0x7642 /* 30274     */
		.word 0x7642 /* 30274     */
		.word 0x30fc /* 12540     */
		.word 0x8118 /*-32488     */
		.word 0x89be /*-30274     */
		.word 0x9a73 /*-25997     */
		.word 0xb214 /*-19948     */
		.word 0xcf04 /*-12540     */
		.word 0xef4b /* -4277     */


		.sect data_x,mp3asm_const_x



	.end
