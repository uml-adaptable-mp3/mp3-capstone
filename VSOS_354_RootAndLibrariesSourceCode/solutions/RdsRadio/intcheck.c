#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  int largest = 0, smallest = 32767;
  int last, new;
  int line = 2;
  scanf("%x", &last);
  scanf("%x", &last);
  while (scanf("%x", &new) == 1) {
    line++;
    if ((unsigned short)(last-new) < smallest) {
      smallest = (unsigned short)(last-new);
      printf("line %d: last 0x%04x, new 0x%04x, smallest 0x%x\n",
	     line, last, new, smallest);
    }
    if ((unsigned short)(last-new) > largest) {
      largest = (unsigned short)(last-new);
      printf("line %d: last 0x%04x, new 0x%04x,\t\t largest 0x%x\n",
	     line, last, new, largest);
    }
    last = new;
  }
  printf("%d lines\n", line);
  return EXIT_SUCCESS;
}
