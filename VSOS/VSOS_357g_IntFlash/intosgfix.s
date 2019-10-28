/*
   Fix to VS1005g's VSDSP core bug/feature with mr0/P.
*/
	.sect code,intOSgfix
	.export _intOSgfix
_intOSgfix:
/*0x90bb*/	STX I7,(I6)+1 ; STY LR0,(I6)
/*0x90bc*/	STX IPR1,(I6)+1 ; STY LR1,(I6)
/*0x90bd*/	STX LE,(I6)+1 ; STY LC,(I6)
/*0x90be*/	LDC 0x0,LC
/*0x90bf*/	LDC 0xffff,LE
/*0x90c0*/	STX MR0,(I6)+1 ; STY I5,(I6)
/*0x90c1*/	STX I0,(I6)+1 ; STY I1,(I6)
/*0x90c2*/	STX A2,(I6)+1 ; STY B2,(I6)
/*0x90c3*/	STX C2,(I6)+1 ; STY D2,(I6)
/*0x90c4*/	//LDC 0x200,MR0	//BUG!!!! Not before add null,p,a
/*0x90c5*/	LDC 0xfc06,I7
/*0x90c6*/	LDY (I7),I0
/*0x90c7*/	LDC 0x10,I1      /* _intVectors */
/*0x90c8*/	STX A0,(I6) ; STY A1,(I6)+1
/*0x90c9*/	AND A,NULL,A ; LDY (I1)*,NULL
/*0x90ca*/	MVY A1,I5 ; MVX A0,I7
/*0x90cb*/	ADD NULL,P,A ; LDY (I1),LR0
		j 0x90cc
		LDC 0x200,MR0

	.end
