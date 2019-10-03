#include <vo_stdio.h>
#include <ByteManipulation.h>

#if 0
auto u_int16 GetBE8(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return (p[byteOffset>>1] >> ((1-(byteOffset&1))*8)) & 0xFF;
}

auto u_int16 GetLE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetBE8(p, byteOffset) | (GetBE8(p, byteOffset+1)<<8);
}

auto u_int16 GetBE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetBE8(p, byteOffset+1) | (GetBE8(p, byteOffset)<<8);
}

auto u_int32 GetLE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetLE16(p, byteOffset) | ((u_int32)GetLE16(p, byteOffset+2)<<16);
}


auto u_int32 GetBE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetBE16(p, byteOffset+2) | ((u_int32)GetBE16(p, byteOffset)<<16);
}

auto u_int16 BitReverse8(register __d0 u_int16 x) {
  x = ((x >> 1) & 0x55) | ((x & 0x55) << 1);
  x = ((x >> 2) & 0x33) | ((x & 0x33) << 2);
  x = ((x >> 4) & 0x0f) | ((x & 0x0f) << 4);
  return x;
}

auto u_int16 BitReverse16(register __d0 u_int16 x) {
  x = ((x >> 8) & 0x00ff) | ((x & 0x00ff) << 8);
  x = ((x >> 4) & 0x0f0f) | ((x & 0x0f0f) << 4);
  x = ((x >> 2) & 0x3333) | ((x & 0x3333) << 2);
  x = ((x >> 1) & 0x5555) | ((x & 0x5555) << 1);
  return x;
}

auto u_int32 BitReverse32(register __reg_d u_int32 x) {
  x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
  x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
  x = ((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4);
  x = ((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8);
  x = (x >> 16) | (x << 16);
  return x;
}


u_int32 BitReverseN32(register __reg_d u_int32 x, register __c0 u_int16 n) {
  return BitReverse32(x<<(32-n));
}

u_int16 BitReverseN16(register __d0 u_int16 x, register __c0 u_int16 n) {
  return BitReverse16(x<<(16-n));
}

void SetBE8(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data) {
  u_int16 d1 = data;
  MemCopyPackedBigEndian(p, byteOffset, &d1, 1, 1);
}

void SetBE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data) {
  u_int16 d1 = data;
  MemCopyPackedBigEndian(p, byteOffset, &d1, 0, 2);
}

void SetLE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data) {
  SetBE16(p, byteOffset, Swap16(data));
}

void SetBE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data) {
  u_int16 d1[2];
  d1[0] = (u_int16)(data>>16);
  d1[1] = (u_int16)data;
  MemCopyPackedBigEndian(p, byteOffset, d1, 0, 4);
}

void SetLE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data) {
  SetBE32(p, byteOffset, Swap32(data));
}
#endif


void SetCHS(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data) {
  int cylinder, sector, head;
  int cylinder, sector, head;
#if 0
  printf("C/H/S %ld (0x%lx) -> ", data, data);
#endif
  if (data >= 1024L*63L*255L) {
    data=1024L*63L*255L-1;
  }
  sector = (int)(data % 63)+1;
  data /= 63;
  head = (int)(data % 255);
  data /= 255;
  cylinder = (int)data;
#if 0
  printf("%d/%d/%d -> ", cylinder, head, sector);
#endif
  SetBE8(p, 0, head&0xFF);
  SetBE8(p, 1, ((cylinder>>8)<<6)|sector);
  SetBE8(p, 2, cylinder&0xFF);
#if 0
  printf("result 0x%06lx\n", GetBE32(p, 0) >> 8);
#endif
}
