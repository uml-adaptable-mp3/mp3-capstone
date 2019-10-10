#ifndef FM_INIT_H
#define FM_INIT_H

void InitFM(void);
void SimplePS(void);

u_int16 SetFMFreq(register u_int32 fmband);
u_int16 CheckRFLck(register u_int16 printena);
u_int32 FmNext(register u_int32 oldFreq, register u_int16 backwards);
void SweepIQrms(u_int16 run_n);
#endif
