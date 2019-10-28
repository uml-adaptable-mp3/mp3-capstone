#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Reg {
  int addr;
  char name[80];
  char altName[80];
  int nBits;
  struct Bit *bit[16];
} reg[4096] = {0};
int nReg = 0;

struct Bit {
  int bit;
  char name[80];
  struct Reg *reg;
} bit[65536] = {0};
int nBit = 0;

void MakeReg(int addr, const char *name) {
  reg[nReg].addr = addr;
  strcpy(reg[nReg].name, name);
  if (!strcmp(name, "UART_STATUS")) {
    strcpy(reg[nReg].altName, "UART_ST");
  } else if (!strcmp(name, "SPI0_CF")) {
    strcpy(reg[nReg].altName, "SPI_CF");
  } else if (!strcmp(name, "SPI0_CLKCF")) {
    strcpy(reg[nReg].altName, "SPI_CC");
  } else if (!strcmp(name, "SPI0_STATUS")) {
    strcpy(reg[nReg].altName, "SPI_ST");
  } else if (!strcmp(name, "GPIO0_BIT_CONF")) {
    strcpy(reg[nReg].altName, "GPIO_BE");
  } else if (!strcmp(name, "USB_EP_SEND0")) {
    strcpy(reg[nReg].altName, "USB_EP_SEND"); 
  } else if (!strcmp(name, "USB_EP_ST0")) {
    strcpy(reg[nReg].altName, "USB_EP_ST");
  } else if (!strcmp(name, "I2S_STATUS")) {
    strcpy(reg[nReg].altName, "I2S_ST");
  }
  reg[nReg].nBits = 16;
  nReg++;
}

void MakeBit(int bitNo, const char *name) {
  bit[nBit].bit = bitNo;
  strcpy(bit[nBit].name, name);
  nBit++;
}

int CompareRegs(const void *rr1, const void *rr2) {
  const struct Reg *r1 = rr1, *r2 = rr2;
  return r1->addr - r2->addr;
}

int main(void) {
  char s[256];
  char name[256];
  int val;
  int i, j;
  while (fgets(s, 256, stdin)) {
    char *p;
    sscanf(s+2, "%04x %s", &val, name);
    if (s[0] == 'r') {
      MakeReg(val, name);
    } else if (s[0] == 'w') {
      if (strcmp(reg[nReg-1].name, name)) {
	fprintf(stderr, "Warning: 'w' field name mismatch: %s vs %s\n",
		reg[nReg-1].name, name);
      } else {
#if 0
	fprintf(stderr, "Set %d %s width to %d\n", nReg-1, name, val);
#endif
	reg[nReg-1].nBits = val;
      }
    } else if (s[0] == 'b') {
      MakeBit(val, name);
    }
  }
  
  qsort(reg, nReg, sizeof(reg[0]), CompareRegs);
  fprintf(stderr, "%d registers, %d bits\n", nReg, nBit);
  for (i=0; i<nReg; i++) {
    for (j=0; j<nBit; j++) {
      if (!bit[j].reg && !reg[i].bit[bit[j].bit] &&
	  !strncmp(reg[i].name, bit[j].name, strlen(reg[i].name)) ||
	  (strlen(reg[i].altName) &&
	   !strncmp(reg[i].altName, bit[j].name, strlen(reg[i].altName)))) {
	bit[j].reg = reg+i;
	reg[i].bit[bit[j].bit] = bit+j;
      }
    }
  }
  for (i=0; i<nReg; i++) {
    int bits = 1;
    printf("%04X%s\n", reg[i].addr, reg[i].name);
    for (j=reg[i].nBits-1; j>=0; j--) {
      struct Bit *b = reg[i].bit[j];
      if (b) {
	printf("%x%x%s\n", j, bits-1, b->name);
	bits = 0;
      }
      bits++;
    }
  }
  for (i=0; i<nBit; i++) {
    if (!bit[i].reg) {
      fprintf(stderr, "Warning: Unused bit %2d \"%s\"\n",
	      bit[i].bit, bit[i].name);
    }
  }
  printf("FFFFend\n");
  return EXIT_SUCCESS;
}
