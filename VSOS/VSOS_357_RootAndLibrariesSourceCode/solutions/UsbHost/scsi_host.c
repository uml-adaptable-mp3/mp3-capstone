#include <stdlib.h>
#include <string.h>
#include <vo_stdio.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <usblowlib.h>
#include <vsos.h>

#include "scsi_host.h"
#include "usb_host.h"
#include "devUsb.h"

extern u_int16 buffer[256];
u_int16 scsiDataBuffer[32];
u_int16 scsiCSW[7];
static u_int16 tag;
u_int16 scsiCMD[16];

typedef s_int16 ioresult;

//ioresult UsbWriteToEndpoint(EpInfo *ep, int *buf, u_int16 bytes, u_int16 isSetup);
//ioresult UsbReadFromEndpoint(EpInfo *ep, int *buf, u_int16 bytes);

s_int16 scsi_request(u_int16 opcode,EpInfo *epin,EpInfo *epout, u_int16 *buf,
		     u_int16 len, u_int16 lun,ScsiReqPar scp);


//EpInfo  OpenEndpoint(UsbDeviceInfo *d, u_int16 n);
UsbDeviceInfo OpenUsbDevice();


// How many times send cwb on case of nack
#define SCSI_CMD_TRY_LIMIT 100
#if 0
#define SCSI_READ_6 0x08
#define SCSI_READ_10 0x28
#define SCSI_REQUEST_SENSE 0x03
#define SCSI_MODE_SENSE_6 0x1a
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_INQUIRY 0x12
#define SCSI_READ_CAPACITY 0x25
#endif

#define USB_EP_LOADED 0x8000
#define USB_SCSI_CMD_LEN 0x1F
#define SCSI_DIR_IN  0x8000
#define SCSI_DIR_OUT 0x8000
#define SCSI_CSW_LEN 13


typedef struct devSdSpiHardwareInfoStruct {
  u_int16 lun;
  EpInfo *ep0;
  EpInfo *epIn;
  EpInfo *epOut;
} devUMassHwInfo;


ioresult DevUMassBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;
	EpInfo *epin = hw->epIn;
	EpInfo *epout = hw->epOut;

}


ioresult DevUMassCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;	
	memset(dev, 0, sizeof(*dev));	
	dev->BlockRead = DevUMassBlockRead;
	dev->Ioctl = DevUMassIoctl;
	dev->deviceInstance = __nextDeviceInstance++;	
	memcpy (hw, name, sizeof(devUMassHwInfo));
	hw->lun = extraInfo;
	//debug_printf("DevUMass: Trying to create UMass device at lun %d.\n",hw->lun);

	return dev->Ioctl(dev, IOCTL_RESTART, 0);
}



UsbProbe() {
  

}

// Createn kutsu:

DEVICE umassLun0;
DEVICE umassLun1;

void CreateUMasses(EpInfo *ep0, EpInfo *epin, EpInfo *epout) {
  devUMassHwInfo umassInfo = {
    0, //lun
    ep0,
    epin,
    epout
  };

  {
    if (DevUMassCreate(&umassLun0, &umassInfo, 0)) {
      SysReport("Cannot create umass device at lun 0");
    } else {
      vo_pdevices['C'-'A'] = &umassLun0;
    }
  }

  {
    if (DevUMassCreate(&umassLun1, &umassInfo, 1)) {
      SysReport("Cannot create umass device at lun 1");
    } else {
      vo_pdevices['D'-'A'] = &umassLun1;
    }
  }
}    




ioresult DevUMassIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
  devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;
  DEVICE *ph = hw->ph;
  u_int16 i, n, cmd;
  
  
  //printf(" SwSpiIoctl:%d ",request);
  switch (request) {
  case IOCTL_START_FRAME:			
    break;
    
  case IOCTL_END_FRAME:
    break;
    
  case IOCTL_RESTART:

    // Initialize lun number hw->lun

    if (hw->lun > max_lun) {
      dev->flags = 0;
      return S_ERROR;
    }
    

    
    // Successful init
    dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;

    dev->fs = StartFileSystem (dev, "0"); // Find a filesystem, which understands this device
    if (dev->fs) {
      //printf("DevUMass: Created.\n");
      return S_OK;
    } else {
      //Set device to be present, but not open, because no filesystem can handle it.
      //printf("DevSdSpi: Cannot create because no filesystem can handle it.\n");
      dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
      return S_ERROR; //Filesystem not initialized
    }
    break;
    
    
    
  default:
    return S_ERROR;
    break;
  }			
  return S_OK;
}





typedef struct {
  u_int16 opcode;
  u_int16 oplen;
} ScsiCmd;


s_int16 init_scsi() {
 UsbDeviceInfo d = OpenUsbDevice();
 EpInfo epin     = OpenEndpoint(&d, EP_IN);
 EpInfo epout    = OpenEndpoint(&d, EP_OUT);
 ScsiReqPar scp; 
 int err;
 u_int16 buf[2];
 EpInfo *ep0;
 tag = 0;

 printf("TAALLA %d %d\n",epin.size,epout.num);

#if 1 
  memset(&scp,0,sizeof(scp));

  strcpy(scsiCMD,"\p\xA1\xFE\x00\x00\x00\x00\x01\x00");

  // GetMaxLun
  ep0 = &epin.dev->ep[0];

  UsbWriteToEndpoint(ep0,"\p\xA1\xFE\x00\x00\x00\x00\x01\x00" , 8, 1);
  err = UsbReadFromEndpoint(ep0, buf, 1);
  UsbWriteToEndpoint(ep0,NULL,0,0);


  printf("GetMaxLun %d %d\n",err,buf[0]);


  //  send_get_std_desc(addr,0xA0|RCPT_INTERFACE,GET_MAX_LUN,0,0,1);
  err = scsi_request(SCSI_TEST_UNIT_READY,&epin,&epout,buffer,0,0,scp);
  if (err < 0) {
    scsi_request(SCSI_REQUEST_SENSE,&epin,&epout,buffer,32,0,scp);
  }
  
  err = scsi_request(SCSI_READ_CAPACITY_10,&epin,&epout,buffer,8,0,scp);
  {
    u_int32 capasity,blocksize;
    printf("Capasity  = %ld\n",((u_int32)buffer[0]<<16)|buffer[1]);
    printf("BlockSize = %ld\n",((u_int32)buffer[2]<<16)|buffer[3]);
  }
  scsi_request(SCSI_REQUEST_SENSE,&epin,&epout,buffer,32,0,scp);
#endif

}

int do_scsi_command_wrapper(ScsiCmd *cmd, 
			    u_int16 dir,
			    u_int16 *buf,
			    u_int32 len) {
  tag++;
  // 16 first bytes are standard
  // 17-31 are command specific
  // Create USB CBW wrapper
  buf[0]   = 0x5553;
  buf[1] = 0x4243;
  buf[2] = ((tag&0xff)<<8)|(tag>>8);
  buf[3] = 0x0000; // Tag high word
  buf[4] = SwapWord(len&0xffff);
  buf[5] = SwapWord(len>>16);
  // MSB is Dir bit
  buf[6] = dir;
  buf[7] = (cmd->oplen<<8)|cmd->opcode;
}

void setbyte(u_int16 *p,u_int16 off,u_int16 byte) {
  u_int16 woff = (off>>1);
  if (off&1) {
    p[woff] = p[woff]&0xFF00|byte;
  }
  p[woff] = p[woff]&0x00FF|(byte<<8);
}


/*
 * Scsi request handler
 */
s_int16 scsi_request(u_int16 opcode,EpInfo *epin,EpInfo *epout, u_int16 *buf,
		     u_int16 len, u_int16 lun,ScsiReqPar scp) {
  ScsiCmd cmd;
  u_int16 i,dir;
  s_int16 st;
  u_int16 *p = buf;
  u_int16 amount;

  cmd.opcode = opcode;
  memset(scsiCMD,0,16);

  switch (opcode) {
  case SCSI_READ_10:
    cmd.oplen  = 10;
    dir = SCSI_DIR_IN;
    
    {
      u_int16 lo = (u_int16)scp.u.lba;
      u_int16 hi = scp.u.lba>>16;

      // Fill read10 data to cbw
      scsiCMD[8]  = (hi>>8);
      scsiCMD[9]  = ((hi&0xFF)<<8)|(lo>>8);
      scsiCMD[10] = ((lo&0xFF)<<8)|0x0000;
      scsiCMD[11] = 1; // Number of blocks
    }

    break;

  case SCSI_REQUEST_SENSE:
    scsiCMD[9]  = 0x0000|len;
    cmd.oplen  = 6;
    dir = SCSI_DIR_IN;
    break;

  case SCSI_READ_CAPACITY_10:
    cmd.oplen  = 10;
    dir = SCSI_DIR_IN;
    break;

  case SCSI_INQUIRY:
    // length 36 bytes
    scsiCMD[8]  = scp.u.page;
    scsiCMD[9]  = len;
    dir = SCSI_DIR_IN;
    cmd.oplen  = 6;
    break;

  case SCSI_TEST_UNIT_READY:
    //dir = SCSI_DIR_IN;
    cmd.oplen  = 6;
    len = 0;
    break;
  
  default:
    printf("Unknown scsi opcode %X\n",opcode);
    return E_FATAL;
  }

  scsiCMD[8]  |= (lun<<13);
  
  // Create scsi command wrapper
  tag++;
  scsiCMD[0] = 0x5553;
  scsiCMD[1] = 0x4243;
  scsiCMD[2] = ((tag&0xff)<<8)|(tag>>8);
  scsiCMD[3] = 0x0000; // Tag high word
  scsiCMD[4] = SwapWord(len&0xffff);
  scsiCMD[5] = SwapWord(len>>16);
  scsiCMD[6] = dir;
  scsiCMD[7] = (cmd.oplen<<8)|cmd.opcode;

  UsbWriteToEndpoint(epout, scsiCMD, USB_SCSI_CMD_LEN, 0);

  // Should we read data
  if (dir != 0) {
    amount = UsbReadFromEndpoint(epin, buf, len);
  }

  if (amount == E_STALL) {
    EpInfo ep0      = OpenEndpoint(epin->dev, EP_0);    
    printf("EP0->dev %p, EPIN->dev %p,EPOUT->dev %p\n",ep0.dev,epin->dev,epout->dev);

    strcpy(scsiCMD,"\p\x02\x01\x00\x00\x82\x00\x00\x00");
    setbyte(scsiCMD, 4 , 0x80|epin->num);    
    UsbWriteToEndpoint(&ep0, scsiCMD, 8, 1);
    epin->dev_toggle=0;
    epin->host_toggle=0;
    amount = UsbReadFromEndpoint(&ep0, buf, 0);

    if (amount < 0) {
      return E_FATAL;
    }
  }

  amount = UsbReadFromEndpoint(epin, &scsiCSW,SCSI_CSW_LEN);

  if (amount == E_STALL) {
    EpInfo ep0      = OpenEndpoint(epin->dev, EP_0);    
    printf("EP0->dev %p, EPIN->dev %p,EPOUT->dev %p\n",ep0.dev,epin->dev,epout->dev);
    UsbWriteToEndpoint(&ep0, "\p\x02\x01\x00\x00\x82\x00\x00\x00", 8, 1);
    amount = UsbReadFromEndpoint(epin, buf, 1);
    UsbWriteToEndpoint(&ep0, NULL, 0, 0);
    amount = UsbReadFromEndpoint(epin, &scsiCSW,SCSI_CSW_LEN);
  }



  // signature error
  if (scsiCSW[0] != 0x5553 && scsiCSW[1] != 0x4253) {
    printf("Wrong signature %x%x\n",scsiCSW[0],scsiCSW[1]);
    return -1;
  }
  
  // tag error 
  if (SwapWord(scsiCSW[2]) != tag) {
    printf("Wrong tag %d != %d\n",scsiCSW[2],tag);
    return -1;
  }
  {    
    u_int32 residue = ((u_int32)SwapWord(scsiCSW[5])<<16)|SwapWord(scsiCSW[4]);
    u_int16 status = scsiCSW[6]>>8;
    if (residue != 0) {
      printf("RESIDUE %ld, status %d\n",residue,status);
    }
      
    // phase error
    if (status != 0)
      return -1;
  }


  // Check residue (should be zero if amount >= len)
  return amount;
}
