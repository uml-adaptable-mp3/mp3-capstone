/// \file lcd-tft-177.c Driver for Shenzhen Feigeda FGD177P1402 1.77" TFT LCD display, using ST7735B controller.
/** 
	- Uses a (hardware) SPI port driver to access the TFT LCD display
	- Define the actual pin connections in your PCB in the LcdInit function
	- With 'v' option start in vertical mode.
	- Display size is 160x128 pixels.
	- Horizontal: 22x16 = 352 characters.
	- Vertical:   18x20 = 360 characters.
	
	LCD NOTES
	- By default, TFT displays in VS1005 systems are kept in 16 bits per pixel, RGB565 color mode.

*/
#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vstypes.h>
#include <kernel.h>
#include <vsos.h>
//#include "lcdisplay.h"
#include <vs1005h.h>
#include <romfont1005e.h>
#include <vo_gpio.h>
#include <devHwSpi.h>
#include "lcd7735.h"
#include <rgb565.h>
#include <lcd.h>
#include <power.h>
#include <clockspeed.h>

#define Output(p,v,b,n) Write(p,b,v,n)
#define Input(p,v,b,n) Read(p,b,v,n)

DEVICE devLcdHw;

/* Width & height in horizontal mode */
#define __LCD_LOGICAL_WIDTH 160
#define __LCD_LOGICAL_HEIGHT 128
/*
	lcd0.width = 160;
	lcd0.height = 128;
	lcd0.textColor = COLOR_WHITE;
	lcd0.backgroundColor = COLOR_NAVY;		
*/	

#define MAX(a,b) (((a)>(b))?(a):(b))

extern lcdInfo lcd0;

#define LCD_SET_COMMAND_MODE() {PERIP(GPIO0_CLEAR_MASK) = (1<<0x0f);}
#define LCD_SET_DATA_MODE() {PERIP(GPIO0_SET_MASK) = (1<<0x0f);}

//#define CS_SET_HIGH(){PERIP(GPIO1_SET_MASK) = (1 << 0xf);}
//#define CS_SET_LOW(){PERIP(GPIO1_CLEAR_MASK) = (1 << 0xf);}
#define CS_SET_HIGH(){PERIP(GPIO1_SET_MASK) = (1 << 0xf);}
#define CS_SET_LOW(){PERIP(GPIO1_CLEAR_MASK) = (1 << 0xf);}
u_int16 bkcolor[MAX(__LCD_LOGICAL_HEIGHT,__LCD_LOGICAL_WIDTH)];

void LcdStartCommand(u_int16 opcode) {
	devLcdHw.Ioctl(&devLcdHw, IOCTL_END_FRAME, 0);
	LCD_SET_COMMAND_MODE();		
	devLcdHw.Ioctl(&devLcdHw, IOCTL_START_FRAME, 0);
	devLcdHw.Output(&devLcdHw, opcode, 0, 1);
	devLcdHw.Ioctl(&devLcdHw, IOCTL_WAIT_UNTIL_TX_IDLE, 0);
	LCD_SET_DATA_MODE();
}



//SPI config value 0x0baf=fifo,master,8bit,idle_hi
//#define __HWSPI_8BIT_MASTER 0x3af
//#define __HWSPI_16BIT_MASTER 0x3bf
#define __HWSPI_8BIT_MASTER 0x1af
#define __HWSPI_16BIT_MASTER 0x1bf
#define __HWSPI_WAIT_UNTIL_TX_IDLE() {	while (hw->regs->status & SPI_ST_TXRUNNING); } 
#define __HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL() {	while ((hw->regs->status & SPI_ST_TXFIFOFULL)){/*printf("ST=%04x ",hw->regs->status);*/} }
//#define __HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL() {	while ((PERIP(SPI0_STATUS)& SPI_ST_TXFIFOFULL)){/*printf("ST=%04x ",hw->regs->status);*/} }


u_int16 FastSpiWriteWords(register __i0 DEVICE *dev, register u_int16 *buf, register u_int16 words, register u_int16 compressed) {
  devHwSpiHwInfo* hw=(devHwSpiHwInfo*)dev->hardwareInfo;
  u_int16 w;
  u_int16 rle = 0, runLeft = 1;

  if (!compressed) {
    runLeft = 0xFFFF;
  }

#if 0
  if (hw->regs->config != __HWSPI_16BIT_MASTER) {
    /*Must wait until end of transmission to switch from 8-bit to 16-bit mode*/
#endif
    __HWSPI_WAIT_UNTIL_TX_IDLE();
    hw->regs->config = __HWSPI_16BIT_MASTER;
#if 0
  }
#endif

  for (w=0; w<words; w++) {
    u_int16 d;

    if (!--runLeft) {
      buf += rle;
      rle = *buf >> 15;
      runLeft = *buf++ & 0x7FFF;
    }
    d = *buf;
    if (!rle) {
      buf++;
    }
    __HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL();
    hw->regs->data = d;
  }

  return S_OK;
}

u_int16 MyLcdFilledRectangle(u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color) {
  static u_int16 buff[MAX(__LCD_LOGICAL_HEIGHT,__LCD_LOGICAL_WIDTH)];
  u_int16 n = x2-x1+1;
  u_int16 w = n;
  u_int16 y;
  u_int16 xOffset = 2, yOffset = 1;
  if (lcd0.width > lcd0.height) {
    /* Horizontal mode */
    xOffset = 1; yOffset = 2;
  }

  y=y1;
  buff[0] = x1+xOffset;
  buff[1] = x2+xOffset;
  LcdStartCommand(LCD_CASET);
  devLcdHw.Output(&devLcdHw,0,buff,4);
  buff[0] = y1+yOffset;
  buff[1] = y2+yOffset;
  LcdStartCommand(LCD_RASET);
  devLcdHw.Output(&devLcdHw,0,buff,4);
		
  /* Replace potential background color with color gradient. */
  if (texture && color != COLOR_COMPRESSED_TEXTURE) {
    u_int16 *k = texture;
    while (y <= y2) {
      for (x1=0; x1<w; x1++) {
	if (*k == 0xfffe) *k = bkcolor[y];
	k++;
      }
      y++;
    }
  }

  LcdStartCommand(LCD_RAMWR);
  if (texture) {
    //		devLcdHw.Output(&devLcdHw,0,texture,n*2);
    FastSpiWriteWords(&devLcdHw,texture,n*(y2-y1+1),color==COLOR_COMPRESSED_TEXTURE);
  } else {
    memset(buff, color, w);
    while (y<=y2) {
      if (color == 0xfffe) memset(buff,bkcolor[y],w);
      FastSpiWriteWords(&devLcdHw,buff,w,0);
      y++;
    }
  }
  devLcdHw.Ioctl(&devLcdHw, IOCTL_END_FRAME, 0);
}


u_int16 MyLcdTextOutXY(u_int16 x1, u_int16 y1, const char *s) {
  static u_int16 buff[7*8];
  while (*s) { //Foreach character *s do:
    __y u_int16 *p = &latin1[(*s)*3]; //Get a pointer to ROM font table
    u_int16 x,y;		
    memset(buff,lcd0.backgroundColor,sizeof(buff)); //fill the texture with bk color
    for (x=0; x<3; x++) { //light some texture pixels with text color based on font
      for (y=0; y<8; y++) { 
	if (p[x]&(1 << (y+8))) buff[y*7+x*2] = lcd0.textColor;
	if (p[x]&(1 << (y))) buff[y*7+x*2+1] = lcd0.textColor;
      }
    }
    MyLcdFilledRectangle(x1,y1,x1+6,y1+7,buff,0); //Draw a textured rectangle
    x1 += 7;
    s++;		
  }
}

void SetPWMLevel(int level) {
  PERIP(PWM_FRAMELEN) = 255; // pulse end position        	
  PERIP(PWM_PULSELEN) = level;  // pulse start position 0,1 disable
}

u_int16 main(char *parameters) {
  s_int16 i = 0;
  s_int16 horizontal = 1;
  static const devHwSpiHwInfo hwInfo = {
    (void *)SPI1_CF, // register base; spiRegisters *regs;
    0x1f, // CS pin 0x1f
    6, // io channel number (74HC138 address decoder select pattern)
    0, //speed (divider)
  };

  if (*parameters == 'v') {
    horizontal = 0;
  }

  SetPWMLevel(0);

  DevHwSpiCreate(&devLcdHw, &hwInfo, 0);

  GpioSetPin(0x2b, 0); //RESET
  Delay(1);	/* ST7735 Datasheet: reset pulse >= 10 microsec. */
  GpioSetPin(0x2b, 1); //NOT RESET
  Delay(120);	/* ST7735 Datasheet: allow 120 ms for "Reset cancel". */

  memset(&lcd0,0,sizeof(lcd0));
  if (horizontal) {
    lcd0.width = __LCD_LOGICAL_WIDTH;
    lcd0.height = __LCD_LOGICAL_HEIGHT;
  } else {
    lcd0.width = __LCD_LOGICAL_HEIGHT;
    lcd0.height = __LCD_LOGICAL_WIDTH;
  }

  {
    u_int16 *k = bkcolor, *k2 = bkcolor+lcd0.height-1;		
    int loops = lcd0.height-128;
    for (i=0; i<64; i++) *k++ = *k2-- = (i>>1);
    memset(k, 31, loops);
  }

  GpioSetPin(0x0e,0); /* CLE, reset LCD. */

  lcd0.buttonFaceColor = lcd0.defaultTextColor = lcd0.textColor = __RGB565RGB(240,240,240);
  lcd0.buttonTextColor = COLOR_BLACK;
  lcd0.defaultBackgroundColor = lcd0.backgroundColor = 0xfffe;
  lcd0.shadowColor = __RGB565RGB(100,100,100);
  lcd0.highlightColor = COLOR_WHITE;
#if 0
  lcd0.clipx1 = 0;
#endif
  lcd0.clipx2 = lcd0.width-1;
#if 0
  lcd0.clipy1 = 0;
#endif
  lcd0.clipy2 = lcd0.height-1;

  GpioSetPin(0x0f,1); /* ALE */
  GpioSetPin(0x0e,1); /* CLE, unreset LCD */

  Delay(130);

#if 0
  /* No need to do SW reset after earlier HW reset */
  LcdStartCommand(LCD_SWRESET);
  Delay(130);
#endif
  LcdStartCommand(LCD_SLPOUT);
  Delay(10);
  LcdStartCommand(LCD_DISPON);
  Delay(130); /* ST7735 Datasheet: DISPON may take up to 130 ms */
  LcdStartCommand(LCD_COLMOD);
  devLcdHw.Output(&devLcdHw, LCD_PIXEL_FORMAT_16BPP, 0, 1);

  LcdStartCommand(LCD_MADCTL);
  devLcdHw.Output(&devLcdHw, horizontal ? 0x68 : 0xc8, 0, 1);
  LcdStartCommand(LCD_GASET);
  devLcdHw.Output(&devLcdHw, 4, 0, 1);

  MyLcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,0,lcd0.backgroundColor);

#if 0
  /* These four code lines plot a white dot in each corner of the LCD.
     They can be used to check whether the X and Y offsets and display
     sizes are correct. */
  MyLcdFilledRectangle(0,0,0,0,0,0xFFFF);
  MyLcdFilledRectangle(0,lcd0.height-1,0,lcd0.height-1,0,0xFFFF);			
  MyLcdFilledRectangle(lcd0.width-1,0,lcd0.width-1,0,0,0xFFFF);			
  MyLcdFilledRectangle(lcd0.width-1,lcd0.height-1,lcd0.width-1,lcd0.height-1,0,0xFFFF);			
#endif

  MyLcdTextOutXY(lcd0.width/2-45,lcd0.height/2-4,"VLSI SOLUTION");

  /* For some reason the display still isn't quite ready, so wait for
     a while to avoid a flash when turning it on. */
  Delay(50);
  SetPWMLevel(128);
	
  SetHandler(LcdFilledRectangle, MyLcdFilledRectangle);	
  SetHandler(LcdTextOutXY, MyLcdTextOutXY);	

  return S_OK;
}
