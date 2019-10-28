#ifndef __RC4_H__
#define __RC4_H__

#include <vstypes.h>

struct RC4State {
    int i, j;
    unsigned char S[256];
};

void RC4Init(struct RC4State *st, unsigned char *key, unsigned int key_length);
unsigned char RC4Output(struct RC4State *st);
void RC4Cipher(struct RC4State *st, unsigned char *data, unsigned int data_length);
void RC4CipherPacked(struct RC4State *st, u_int16 *data, unsigned int data_length);
unsigned char RC4Output(struct RC4State *st);

#ifdef __VSDSP__
void RC4CipherGeneral(struct RC4State *st, u_int16 *data, unsigned int data_length, int packed);
#define RC4Cipher(a,b,c) RC4CipherGeneral(a,b,c,0)
#define RC4CipherPacked(a,b,c) RC4CipherGeneral(a,b,c,1)
#else
void RC4CipherGeneral(struct RC4State *st, u_int16 *data, unsigned int data_length, int packed);
#endif


#endif/*!__RC4_H__*/
