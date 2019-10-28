/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <string.h>
#include <strings.h>
#include <consolestate.h>
#include <ctype.h>
#include <stdlib.h>
#include "disassemble.h"
#include "printbits.h"


const char tss[2][3] = {"", "; "};

const char *Get6BitReg(register u_int16 reg) {
  static const char *reg6Str[64] = {
    "A0", "A1", "B0", "B1", "C0", "C1", "D0", "D1",
    "LR0", "LR1", "MR0", "<res>", "NULL", "LC", "LS", "LE",
    "I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7",
    "<res>","<res>","<res>","<res>","<res>","<res>","<res>","<res>",
    "A2", "B2", "C2", "D2", "NOP", "<res>", "<res>", "<res>",
    "<res>","<res>","<res>","<res>","<res>","<res>","<res>","<res>",
    "<res>","<res>","<res>","<res>","<res>","<res>","<res>","<res>",
    "<res>","<res>","<res>","<res>","<res>","<res>","IPR0","IPR1",
  };
  return reg6Str[reg&0x3f];
}

const char *Get3BitAluReg(register u_int16 reg, register u_int16 wide) {
  static const char *alu3Regs[2][8] = {
    {"A0", "A1", "B0", "B1", "C0", "C1", "D0", "D1"},
    {"<res>","A","<res>","B","<res>","C","<res>","D"},
  };
  return alu3Regs[wide][reg&7];
}

const char *Get4BitAluReg(register u_int16 reg, register u_int16 *wide) {
  static const char *aluRegs[16] = {
    "A0", "A1", "B0", "B1", "C0", "C1", "D0", "D1",
    "NULL", "ONES", "<res>", "P", "A", "B", "C", "D"
  };
  reg &= 0xf;
  if (reg >= 11) *wide = 1;
  return aluRegs[reg];
}

const char *Get3BitIReg(register u_int16 d) {
  static const char iRegs[8][3] = {
    "I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7",
  };
  return iRegs[d&7];
}

const char *GetCtlReg(register u_int32 d) {
  static const char iRegs[8][5] = {
    "(I0)", "(I1)", "(I2)", "(I3)", "(I4)", "(I5)", "(I6)", "(I7)",
  };
  if (d & (1L<<23)) {
    return iRegs[((u_int16)d>>6)&7];
  }
  return "";
}

const char *GetPostMod(register u_int16 d) {
  static const char postMod[16][3] = {
    "", "+1", "+2", "+3", "+4", "+5", "+6", "+7",
    "*", "-7", "-6", "-5", "-4", "-3", "-2", "-1",
  };
  return postMod[d&0xf];
}

int PrintShortMove(register u_int16 bus, register u_int16 d, register u_int16 hasPrinted) {
  const char *modS = (d&0x8) ? "*" : "", busChar = 'X'+bus;
  const char *iReg = Get3BitIReg(d>>4);
  const char *aReg = Get3BitAluReg(d, 0);

  if (d & (1<<7)) {
    printf("; ST%c %s,(%s)%s", busChar, aReg, iReg, modS);
  } else {
    printf("; LD%c (%s)%s,%s", busChar, iReg, modS, aReg);
  }

  return 1;
}

int PrintFullMove(register u_int16 bus, register u_int16 d, register u_int16 hasPrinted) {
  int busChar = 'X'+bus;
  const char *iReg = Get3BitIReg(d>>10);
  const char *modS = GetPostMod(d>>6);
  const char *aReg = Get6BitReg(d);
  const char *ts = tss[hasPrinted];

  d &= 0x3FFF;
  if (d == 0x0024) { /* NOP */
    return hasPrinted;
  }
  if (d & (1<<13)) {
    printf("%sST%c %s,(%s)%s", ts, busChar, aReg, iReg, modS);
  } else {
    printf("%sLD%c (%s)%s,%s", ts, busChar, iReg, modS, aReg);
  }
  return 1;
}



int PrintParallelMove(register u_int32 d, register u_int16 hasPrinted) {
  if ((u_int16)(d>>16) & 0x1) {
    /* Parallel short moves */
    PrintShortMove(0, (u_int16)d>>8, hasPrinted);
    hasPrinted = PrintShortMove(1, (u_int16)d>>0, 1);
  } else if (!((u_int16)d & 0x4000)) {
    /* Full move */
    hasPrinted = PrintFullMove(((u_int16)d>>15), (u_int16)d, hasPrinted);
  } else if (!((u_int16)d & 0x3000)) {
    /* reg-to-reg move (Y bus) */
    printf("%sMV %s,%s",
	   tss[hasPrinted],
	   Get6BitReg((u_int16)d>>6), Get6BitReg((u_int16)d>>0));
    hasPrinted = 1;
  } else if (((u_int16)d & 0x3C00) == 0x1000) {
    /* long-X move */
    printf("; MVLONGX");
    hasPrinted = 1;
  } else if (((u_int16)d & 0x3C00) == 0x1400) {
    /* I-bus move */
    u_int16 regChar = 'A'+((u_int16)d&3);
    const char *iReg = Get3BitIReg((u_int16)d>>6);
    const char *mod = GetPostMod((u_int16)d>>2);
    const char *ts = tss[hasPrinted];

    if ((u_int16)d & (1<<9)) {
      printf("%sSTI %c,(%s)%s", ts, regChar, iReg, mod);
    } else {
      printf("%sLDI (%s)%s,%c", ts, iReg, mod, regChar);
    }
    hasPrinted = 1;
  }
  return hasPrinted;
}


void PrintDisassembled(register u_int16 addr, register u_int32 d, u_int16 fast) {
  static const char mulStr[4][3] = {
    "", "SU", "US", "UU"
  };
  u_int16 cnst = (u_int16)(d>>6);
  u_int16 dHi = (u_int16)(d>>16);
  u_int16 opCode = (u_int16)(dHi>>(28-16));

  printf("I:0x%04x=0x%08lx // ", addr, d);

  if (opCode < 2) { /* LDC */
    /* Is destination register Move NOP? */
    if (((u_int16)d & 0x003f) == 0x0024) {
      printf("NOP");
    } else {
      const char *s = Get6BitReg((u_int16)d);
      printf("LDC 0x%04x,%s", cnst, s);
      if (*s == 'I' && cnst >= 0x10 && !fast) {
	if (cnst < 0xfc00) {
	  printf("    // X:");
	  RunLibraryFunction("TRACE", ENTRY_4, cnst);
	  printf(", Y:");
	  RunLibraryFunction("TRACE", ENTRY_5, cnst);
	} else {
	  printf("    // PERIP:");
	  PrintBitsY(NULL, cnst, cnst, 0, -1);
	}
      }
    }
  } else if (opCode == 2) { /* Control */
    static const char *ctlIns[16] = {
      "JR", "RETI", "RESP", "res", "LOOP", "LOOP", "LOOP", "LOOP",
      "J", "CALL", "JMPI", "MV", "res", "HALT", "res", "res"
    };
    u_int16 ctl = ((u_int16)(dHi>>(24-16)))&0xf;
    int doTrace = 0;
    printf("%s", ctlIns[ctl]);
    /*      JR               J                CALL */
    if (ctl == 0b0000 || ctl == 0b1000 || ctl == 0b1001) {
      static const char *jConds[64] = {
	"", "CS", "ES", "VS", "NS", "ZS", "", "",
	"LT", "LE", "", "", "", "", "", "",
	"", "CC", "EC", "VC", "NC", "ZC", "", "",
	"GE", "GT", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
      };
      printf(jConds[(u_int16)d&0x3f]);
      if (ctl == 0b0000) {
	printf(GetCtlReg(d));
      }
    }
    printf(" ");

    /*          J, CALL, JMPI            */
    if (ctl >= 0b1000 && ctl <= 0b1010) {
      printf("0x%04x", cnst);
      doTrace = 1;
    }

    /*      RETI */
    if (ctl == 0b0001) {
      printf(GetCtlReg(d));
    }

    /*      RESP */
    if (ctl == 0b0010) {
      printf("%s,%s",
	     Get3BitAluReg((u_int16)(dHi>>(17-16)), 0),
	     Get3BitAluReg((u_int16)(dHi>>(20-16)), 0));
    }
    /*               LOOP */ 
    if (ctl >= 0b0100 && ctl <= 0b0111) {
      printf("%s,0x%04x", Get6BitReg((u_int16)d & 0x1f), cnst);
      doTrace = 1;
    }
    /*    JMPI */
    if (ctl == 0b1010) {
      u_int16 mod = ((u_int16)d>>3)&3;
      if (mod == 1 || mod == 3) {
	printf(",(%s)%+d", Get3BitIReg((u_int16)d), 2-mod);
      }
    }
    /*    MVX/MVY */
    if (ctl == 0b1011) {
      printf("%s,%s; MV %s,%s",
	     Get6BitReg((u_int16)(dHi>>(18-16))), Get6BitReg((u_int16)(d>>12)),
	     Get6BitReg((u_int16)d>>6), Get6BitReg((u_int16)d>>0));
    }
    if (doTrace && !fast) {
      printf("    // ");
      RunLibraryFunction("TRACE", ENTRY_1, cnst);
    }
  } else if (opCode == 3) { /* Double move */
    int hasPrinted = PrintFullMove(0, (u_int16)(d>>14), 0);
    PrintFullMove(1, (u_int16)(d>>0), hasPrinted);
  } else if (opCode == 15) { /* Single operand instruction + MUL */
    static const char singleOpStr[16][5] = {
      "ABS", "ASR", "LSR", "LSRC", "NOP", "EXP", "SAT", "RND",
      "RES", "RES", "RES", "RES", "RES", "RES", "MUL", "MUL"
    };
    u_int16 singleOp = (u_int16)(dHi>>(24-16))&0xf;
    int hasPrinted = 0;
    static const char *nop[2] = {"NOP",""};
    if (singleOp >= 14) {
      printf("MUL%s %s,%s",
	     mulStr[((u_int16)(dHi>>(23-16))&3)],
	     Get3BitAluReg((u_int16)(dHi>>(17-16)), 0),
	     Get3BitAluReg((u_int16)(dHi>>(20-16)), 0));
      hasPrinted = 1;
    } else if (singleOp != 4) {
      u_int16 wide = 0;
      printf("%s %s,",
	     singleOpStr[singleOp], Get4BitAluReg((u_int16)(dHi>>(20-16)), &wide));
      /*         EXP              RND */
      if (singleOp == 0x5 || singleOp == 0x7) wide = 0;
      printf(Get3BitAluReg((u_int16)(dHi>>(17-16)), wide));
      hasPrinted = 1;
    }
    hasPrinted = PrintParallelMove(d, hasPrinted);
    printf(nop[hasPrinted]);
  } else {
    static const char *opCodeStr[16] = {
      "LDC", "LDC", "Ctrl", "DblMove", "ADD", "MAC", "SUB", "MSU",
      "ADDC", "SUBC", "ASHL", "AND", "OR", "XOR", "Extension", "Single"
    };
    printf(opCodeStr[opCode]);
    /*         MAC              MSU           */
    if (opCode == 0b0101 || opCode == 0b0111) {
      printf("%s %s,%s,%s",
	     mulStr[((u_int16)(dHi>>(23-16))&3)],
	     Get3BitAluReg((u_int16)(dHi>>(25-16)), 0),
	     Get3BitAluReg((u_int16)(dHi>>(20-16)), 0),
	     Get3BitAluReg((u_int16)(dHi>>(17-16)), 1));
    } else {
      u_int16 wide = 0;
      const char *l = Get4BitAluReg((u_int16)(dHi>>(24-16)), &wide);
      const char *r = Get4BitAluReg((u_int16)(dHi>>(20-16)), &wide);
      /* l & r need to be evaluated first to get correct value into var wide */
      printf(" %s,%s,%s", l, r, Get3BitAluReg((u_int16)(dHi>>(17-16)), wide));
    }
    PrintParallelMove(d, 1);
  }

  printf("\n");
}
