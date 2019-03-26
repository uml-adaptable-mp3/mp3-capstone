#ifndef BUTTONS_H
#define BUTTONS_H

#define BUTTON1  (0x1E)
#define BUTTON2  (0x25)
#define BUTTON3  (0x27)
#define BUTTON4  (0x0D)
#define BUTTON5  (0x20)
#define BUTTON6  (0x21)
#define BUTTON7  (0x22)
#define BUTTON8  (0x23)
#define BUTTON9  (0x24)

#define BUTTON10 (0x1A)
#define BUTTON11 (0x1B)
#define BUTTON12 (0x1C)
#define BUTTON13 (0x1D)

#define CBUTTON1 (0x18)
#define CBUTTON2 (0x19)
#define CBUTTON3 (0x14)

#define ACTIVEHIGH (1)
#define ACTIVELOW  (0)

#define UP       (1)
#define DOWN     (0)

#define MUTE_VOLUME 200

#define DEFAULT_SEED 19586

u_int16 nextSong(register u_int16 button, u_int16 activeHigh);
u_int16 repeatSong(register u_int16 button, u_int16 activeHigh);
u_int16 playPause(register u_int16 button, u_int16 activeHigh);
u_int16 volumeUp(register u_int16 button, u_int16 activeHigh);
u_int16 volumeDown(register u_int16 button, u_int16 activeHigh);
u_int16 volumeMute(register u_int16 button, u_int16 activeHigh);
u_int16 stopSong(register u_int16 button, u_int16 activeHigh);
u_int16 previousSong(register u_int16 button, u_int16 activeHigh);
u_int16 shuffleSong(register u_int16 button, u_int16 activeHigh);

#endif

