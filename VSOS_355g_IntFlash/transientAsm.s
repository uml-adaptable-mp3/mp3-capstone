/**
	\file transientAsm.s

	Adds symbols that cannot be added via a C interface
	
*/
#include <vs1005g.h>
#include <vstypes.h>


#ifdef ASM
	.import df_add,df_div,df_ge,df_lt,df_mul,df_norm,df_sub
	.import df_to_int,df_to_short,df_to_uint,df_to_ushort
	.import divide16signed,divide16unsigned,divide32signed,divide32unsigned
	.import int_to_df,MonitorBreak,shiftMultipliers,short_to_df
	.import uint_to_df,ushort_to_df

	.sect data_x,transientasmconst_x
df_add_sym:
	.uword "df_add\0"
df_div_sym:
	.uword "df_div\0"
df_ge_sym:
	.uword "df_ge\0"
df_lt_sym:
	.uword "df_lt\0"
df_mul_sym:
	.uword "df_mul\0"
df_norm_sym:
	.uword "df_norm\0"
df_sub_sym:
	.uword "df_sub\0"
df_to_int_sym:
	.uword "df_to_int\0"
df_to_short_sym:
	.uword "df_to_short\0"
df_to_uint_sym:
	.uword "df_to_uint\0"
df_to_ushort_sym:
	.uword "df_to_ushort\0"
divide16signed_sym:
	.uword "divide16signed\0"
divide16unsigned_sym:
	.uword "divide16unsigned\0"
divide32signed_sym:
	.uword "divide32signed\0"
divide32unsigned_sym:
	.uword "divide32unsigned\0"
int_to_df_sym:
	.uword "int_to_df\0"
MonitorBreak_sym:
	.uword "MonitorBreak\0"
shiftMultipliers_sym:
	.uword "shiftMultipliers\0"
short_to_df_sym:
	.uword "short_to_df\0"
uint_to_df_sym:
	.uword "uint_to_df\0"
ushort_to_df_sym:
	.uword "ushort_to_df\0"

	.sect code,TransientAddKernelSymbolsAsm
	.export _TransientAddKernelSymbolsAsm
_TransientAddKernelSymbolsAsm:
	.import _SymbolAdd
	ldx (i6)+1,null
	stx i0,(I6)+1 ; sty i1,(I6)
	stx i2,(I6)+1 ; sty i3,(I6)
	stx i4,(I6)+1 ; sty lr0,(I6)
	stx a0,(I6)+1 ; sty a1,(I6)
	stx b0,(I6)+1 ; sty b1,(I6)
	stx c0,(I6)+1 ; sty c1,(I6)
	stx d0,(I6) ; sty d1,(I6)

	ldc 1,i1
	ldc df_add,c0
	call _SymbolAdd
	ldc df_add_sym,i2

	ldc 1,i1
	ldc df_div,c0
	call _SymbolAdd
	ldc df_div_sym,i2

	ldc 1,i1
	ldc df_ge,c0
	call _SymbolAdd
	ldc df_ge_sym,i2

	ldc 1,i1
	ldc df_lt,c0
	call _SymbolAdd
	ldc df_lt_sym,i2

	ldc 1,i1
	ldc df_mul,c0
	call _SymbolAdd
	ldc df_mul_sym,i2

	ldc 1,i1
	ldc df_norm,c0
	call _SymbolAdd
	ldc df_norm_sym,i2

	ldc 1,i1
	ldc df_sub,c0
	call _SymbolAdd
	ldc df_sub_sym,i2

	ldc 1,i1
	ldc df_to_int,c0
	call _SymbolAdd
	ldc df_to_int_sym,i2

	ldc 1,i1
	ldc df_to_short,c0
	call _SymbolAdd
	ldc df_to_short_sym,i2

	ldc 1,i1
	ldc df_to_uint,c0
	call _SymbolAdd
	ldc df_to_uint_sym,i2

	ldc 1,i1
	ldc df_to_ushort,c0
	call _SymbolAdd
	ldc df_to_ushort_sym,i2

	ldc 1,i1
	ldc divide16signed,c0
	call _SymbolAdd
	ldc divide16signed_sym,i2

	ldc 1,i1
	ldc divide16unsigned,c0
	call _SymbolAdd
	ldc divide16unsigned_sym,i2

	ldc 1,i1
	ldc divide32signed,c0
	call _SymbolAdd
	ldc divide32signed_sym,i2

	ldc 1,i1
	ldc divide32unsigned,c0
	call _SymbolAdd
	ldc divide32unsigned_sym,i2

	ldc 1,i1
	ldc int_to_df,c0
	call _SymbolAdd
	ldc int_to_df_sym,i2

	ldc 1,i1
	ldc MonitorBreak,c0
	call _SymbolAdd
	ldc MonitorBreak_sym,i2

	ldc 1,i1
	ldc shiftMultipliers,c0
	call _SymbolAdd
	ldc shiftMultipliers_sym,i2

	ldc 1,i1
	ldc short_to_df,c0
	call _SymbolAdd
	ldc short_to_df_sym,i2

	ldc 1,i1
	ldc uint_to_df,c0
	call _SymbolAdd
	ldc uint_to_df_sym,i2

	ldc 1,i1
	ldc ushort_to_df,c0
	call _SymbolAdd
	ldc ushort_to_df_sym,i2

	ldx (I6)-1,d0 ; ldy (I6),d1
	ldx (I6)-1,c0 ; ldy (I6),c1
	ldx (I6)-1,b0 ; ldy (I6),b1
	ldx (I6)-1,a0 ; ldy (I6),a1
	ldx (I6)-1,i4 ; ldy (I6),lr0
	ldx (I6)-1,i2 ; ldy (I6),i3
	jr
	ldx (I6)-1,i0 ; ldy (I6),i1

#endif /* ASM */

	.end

#if 0
	/********* EVERYTHING FROM NOW ON IS PURELY DECORATIONAL!!! **********/

/*
	Generic()

	Inputs:
	  i0: data (16-bit)
	  a0: chan

	Outputs:
	  -
*/
	.sect code,Generic
	.export _Generic
_Generic:
	stx mr0,(I6)+1 ; sty i7,(I6)
	stx lr0,(I6)+1 ; sty i5,(I6)	// return address
	stx i0,(I6)+1 ; sty i1,(I6)
	stx i2,(I6)+1 ; sty i3,(I6)
	stx i4,(I6)+1 ; sty i5,(I6)
	stx a0,(I6)+1 ; sty a1,(I6)
	stx b0,(I6)+1 ; sty b1,(I6)
	stx c0,(I6)+1 ; sty c1,(I6)
	stx d0,(I6)+1 ; sty d1,(I6)
	stx le,(I6)+1 ; sty ls,(I6)
	stx lc,(I6)

	ldc MR_INT,mr0

	ldx (I6)-1,lc
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,d0 ; ldy (I6),d1
	ldx (I6)-1,c0 ; ldy (I6),c1
	ldx (I6)-1,b0 ; ldy (I6),b1
	ldx (I6)-1,a0 ; ldy (I6),a1
	ldx (I6)-1,i4 ; ldy (I6),i5
	ldx (I6)-1,i2 ; ldy (I6),i3
	ldx (I6)-1,i0 ; ldy (I6),i1
	ldx (I6)-1,lr0 ; ldy (I6),i5
	jr
	ldx (I6)-1,mr0 ; ldy (I6),i7
#endif
