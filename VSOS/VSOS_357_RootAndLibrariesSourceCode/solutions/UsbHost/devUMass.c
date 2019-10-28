/* 
 * USB mass storage $Id: devUMass.c,v 1.5 2013-07-19 16:23:32+03 erkki Exp erkki $ 
 */

/// \file devUMass.c USB Host Mass Storage (USB memory stick) driver for VSOS3
/// \author Erkki Ritoniemi, adapted to VSOS3 by Panu-Kristian Poiksalo
/// \todo This code is highly experimental and should only be used with USB memory sticks that have been verified to work with this code.

#include <stdlib.h>
#include <string.h>
#include <vo_stdio.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <usblowlib.h>
#include <vsos.h>
#include "devUMass.h"
#include "devUsb.h"
#include "scsi_host.h"
#include "usb_host.h"
#include "timers.h"
#include <hwLocks.h>

void CheckSOF(u_int16 flag);

// How many times try soft failed scsi-command
#define SCSI_CMD_TRY_LIMIT 10


#define USB_EP_LOADED 0x8000
#define USB_SCSI_CMD_LEN 0x1F
#define SCSI_DIR_IN  0x8000
#define SCSI_DIR_OUT 0x0000
#define SCSI_CSW_LEN 13
void LcdDelay(u_int32 n);

//#define debug_printf(...) /* (...) */
//#define debug_printf(...) LcdDelay(1000)
//#define debug_printf printf

#if 1
#define debug_printf4(x,y,z,w) ;
#define debug_printf3(x,y,z) ;
#define debug_printf2(x,y) ;
#define debug_printf1(x) ;
#else
#define debug_printf4(x,y,z,w) printf(x,y,z,w)
#define debug_printf3(x,y,z) printf(x,y,z)
#define debug_printf2(x,y) printf(x,y)
#define debug_printf1(x) printf(x)
#endif

extern u_int16 buffer[256];
u_int16 scsiDataBuffer[32];
u_int16 scsiCSW[8];
u_int16 scsiCMD[17];
static u_int16 tag;

typedef s_int16 ioresult;

typedef struct devUMassHardwareInfoStruct {
  u_int16 lun;
  EpInfo *ep0;
  EpInfo *epIn;
  EpInfo *epOut;
} devUMassHwInfo;


UsbDeviceInfo usbDevInfo;



s_int16 scsi_request(u_int16 opcode,EpInfo *epin,EpInfo *epout, u_int16 *buf,
		     u_int16 len, u_int16 lun,ScsiRequestParameters scp);

EpInfo  *OpenEndpoint(UsbDeviceInfo *d, u_int16 n);
ioresult OpenUsbDevice(UsbDeviceInfo *);

// From devUsb.c
void enable_autosofs(u_int16);
ioresult usb_bulk_only_reset_recovery(UsbDeviceInfo *udi,int flag);
u_int16 getbyte(u_int16 off,u_int16 *p);
void setbyte(u_int16 *p,u_int16 off,u_int16 byte);


char *sk_table[14] = {
  "SK_NO_SENSE",
  "SK_RECOVERED_ERROR",
  "SK_NOT_READY",
  "SK_MEDIUM_ERROR",
  "SK_HARDWARE_ERROR",
  "SK_ILLEGAL_REQUEST",
  "SK_UNIT_ATTENTION",
  "SK_DATA_PROTECT",
  "SK_BLANK_CHECK",
  "SK_VENDOR_SPECIFIC",
  "SK_COPY_ABORTED",
  "SK_ABORTED_COMMAND",
  "SK_EQUAL",
  "SK_VOLUME_OVERFLOW"
};


void dump_devsc(UsbDeviceInfo *usbdev);

#define VO_MAX_FILES 6 

extern u_int16 vo_max_num_files;
extern void *vo_pfiles;

/// Invalidate all file handles that point to this device
void InvalidateFileHandles(register __i0 DEVICE *dev) {
	FILE *fp = vo_pfiles;
	u_int16 i;
	for (i=0; i<vo_max_num_files; i++) {
		//printf("file %p flags %p\n",fp,fp->flags);
		if (fp->dev == dev) {
			//printf("invalidate\n");
			fp->flags |= __MASK_ERROR;
		}
		fp++;
	}
}

ioresult DevUMassBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;
	EpInfo *epin = hw->epIn;
	EpInfo *epout = hw->epOut;
	ScsiRequestParameters scp; 
	s_int16 st;
	u_int16 i;
	ioresult result = S_ERROR;

	scp.u.lba = firstBlock;

	ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_USB);

	debug_printf2("Write to lba %ld\n",scp.u.lba);

	for (i=0; i < SCSI_CMD_TRY_LIMIT ; i++) {
	  
	  st = scsi_request(SCSI_WRITE_10,epin,epout,data,blocks*512,hw->lun,scp);
	  
	  if (st == E_AGAIN) {
	    // Try same request again 
	    debug_printf1("GOT E_AGAIN\n");
	    continue;
	    
	  } else if (st == S_ERROR) {
		printf("W_Reinit\n");
	    // Try reconfigure usb_bus	  
	    if (DevUMassProbe() == 0) {//(DevUMassProbe() == S_ERROR) {
	    	printf("W_Reinit Failed\n");
	    	InvalidateFileHandles(dev);
	      dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
	      result = S_ERROR;
	      goto finally;	 
	      printf("Success Failed\n");     
	    }
	    
	  } else {
	    // scsi request completed without errors
	    result = S_OK;
	    goto finally;
	  }	    
	}
	debug_printf1("PERMANENT ERROR\n");
	dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
	result = S_ERROR;
	
	finally:
	ReleaseHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_USB);
	return result;
	
}






// Read block from usb mass storage
ioresult DevUMassBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;
	EpInfo *epin = hw->epIn;
	EpInfo *epout = hw->epOut;
	ScsiRequestParameters scp; 
	s_int16 st;
	u_int16 i;
	ioresult result = S_ERROR;
	
	ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_USB);
	
	scp.u.lba = firstBlock;

	for (i=0; i < SCSI_CMD_TRY_LIMIT ; i++) {
	  
	  st = scsi_request(SCSI_READ_10,epin,epout,data,blocks*512,hw->lun,scp);
	  
	  if (st == E_AGAIN) {
	    // Try same request again 
	    debug_printf1("GOT E_AGAIN\n");
	    continue;
	    
	  } else if (st == S_ERROR) {
		printf("R_Reinit\n");
	    // Try reconfigure usb_bus	  
	    if (DevUMassProbe() == 0) {//(DevUMassProbe() == S_ERROR) {
	    printf("R_Reinit Failedt\n");
		  InvalidateFileHandles(dev);

	      dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
	      result = S_ERROR;
	      goto finally;
	    }

	  } else {
	    // scsi request completed without errors
	    result = S_OK;
	    goto finally;
	  }	    
	}
	debug_printf1("PERMANENT ERROR\n");
	dev->flags = __MASK_PRESENT | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE; //not open
	

	result = S_ERROR;
	
	finally:
	ReleaseHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_USB);
	return result;
}


const char* DevUMassIdentify(register __i0 void *self, char *buf, u_int16 bufsize){ ///< Return some kind of decorative name for this object.
  return "USB Flash";
}

ioresult DevUMassCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;
	memset(dev, 0, sizeof(*dev));
	dev->Identify = DevUMassIdentify;
	dev->BlockRead = DevUMassBlockRead;
	dev->BlockWrite = DevUMassBlockWrite;
	dev->Ioctl = DevUMassIoctl;
	dev->deviceInstance = __nextDeviceInstance++;
	memcpy (hw, name, sizeof(devUMassHwInfo));
	hw->lun = extraInfo;
	debug_printf2("DevUMass: Trying to create UMass device at lun %d.\n",hw->lun);
	{
		ScsiRequestParameters scp;
		u_int16 i;
		scsi_request(SCSI_TEST_UNIT_READY,hw->epIn,hw->epOut,buffer,0,hw->lun,scp); //Sometimes this makes them feel good
		scsi_request(SCSI_READ_CAPACITY_10,hw->epIn,hw->epOut,buffer,8,hw->lun,scp);
				if (scsi_request(SCSI_READ_CAPACITY_10,hw->epIn,hw->epOut,buffer,8,hw->lun,scp) != S_OK) {
			return S_ERROR; //Does not want to tell its capacity -> >:(
		}
		if (buffer[3] != 0x0200) {
			printf("Unsupported blocksize %d not 512.\n",buffer[3]);
			return S_ERROR; //Has some weird block size -> >:(
		}
	}
	
	return dev->Ioctl(dev, IOCTL_RESTART, 0);
}


// Createn kutsu:
//DEVICE umassLun0;
//DEVICE umassLun1;

/// This function tries to create two logical disks in the USB 
/// mass storage device: LUNs 0 and 1.
/// If you want to support other LUNs, edit this code and recompile.
/*
void CreateUMasses(EpInfo *ep0, EpInfo *epin, EpInfo *epout) {
  
  devUMassHwInfo umassInfo;
  umassInfo.ep0 = ep0;
  umassInfo.epIn = epin;
  umassInfo.epOut = epout;

  if (DevUMassCreate(&umassLun0, &umassInfo, 0)) {
    SysReport("Cannot create umass device at lun 0");
  } else {
    vo_pdevices['U'-'A'] = &umassLun0;
  }
  
  if (DevUMassCreate(&umassLun1, &umassInfo, 1)) {
    SysReport("Cannot create umass device at lun 1");
  } else {
    vo_pdevices['V'-'A'] = &umassLun1;
  }  
}    
*/

ioresult CreateUsbFlashDisk(DEVICE *usbFlash, u_int16 lun) {
  devUMassHwInfo umassInfo;
  umassInfo.ep0 = &usbDevInfo.ep[0];
  umassInfo.epIn = &usbDevInfo.ep[1];
  umassInfo.epOut = &usbDevInfo.ep[2];
	

  if (DevUMassCreate(usbFlash, &umassInfo, lun)) {
   // fprintf(vo_stderr,"Lun %d: No disk.\n",lun);
    return S_ERROR;
  } 
  return S_OK;
}    

void UsbSetupReadOperation(register EpInfo *ep, register char *opString, 
			   register void *inBuffer, register u_int16 inBytes);


// Probe USB bus and create mass storage disks if possible.
// Returns the number of LUNs (disk drives) that are found in the USB device.
u_int16 DevUMassProbe() {

  debug_printf1("DevUmassProbe()\n");

  if (OpenUsbDevice(&usbDevInfo) == S_OK) {
    u_int16 i;
    int err;
		ScsiRequestParameters scp;
    EpInfo *ep0      = OpenEndpoint(&usbDevInfo, EP_0);
    EpInfo *epin     = OpenEndpoint(&usbDevInfo, EP_IN);
    EpInfo *epout    = OpenEndpoint(&usbDevInfo, EP_OUT);
    

    scp.u.lba=0;

#if 0
    // Wait until unit becomes ready
    i = err = 0;
    while (1) {
      if (scsi_request(SCSI_TEST_UNIT_READY,epin,epout,buffer,0,0,scp) != S_OK) {
	CheckSOF(1);
	err++;
      } else {
	break;
      }

      if (i > 1000) 
	return 0;
    }
#endif

    // GetMaxLun
    UsbWriteToEndpoint(ep0,"\p\xA1\xFE\x00\x00\x00\x00\x01\x00" , 8, 1);
    if (UsbReadFromEndpoint(ep0, buffer, 1) > 0) {
      UsbWriteToEndpoint(ep0, NULL, 0, 0);
      usbDevInfo.maxlun = SwapWord(buffer[0]);
    } else {
      usbDevInfo.maxlun = 0;
    }
    //printf("GetMaxLun %d\n",usbDevInfo.maxlun);    
    //Enumerate luns and create disks
    //CreateUMasses(ep0,epin,epout);
    return  usbDevInfo.maxlun+1;

  } else {
    memset(&usbDevInfo,0,sizeof(usbDevInfo));
    return 0;
  }
}


ioresult DevUMassIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
  devUMassHwInfo* hw=(devUMassHwInfo*)dev->hardwareInfo;
  //  DEVICE *ph = hw->ph;
  u_int16 i, n, cmd;
  
  switch (request) {
    
  case IOCTL_RESTART:

    // Initialize lun number hw->lun
        
    // Successful init
    dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;

	dev->BlockRead(dev, 0, 1, buffer); 
    dev->fs = StartFileSystem (dev, "0"); // Find a filesystem, which understands this device
    if (dev->fs) {
      debug_printf1("DevUMass: Created.\n");
      return S_OK;
    } else {
      //Set device to be present, but not open, because no filesystem can handle it.
      debug_printf1("DevUMass: Cannot create because no filesystem can handle it.\n");
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


/*
 * Scsi request handler
 */
s_int16 scsi_request(u_int16 opcode,EpInfo *epin,EpInfo *epout, u_int16 *buf,
		     u_int16 len, u_int16 lun, ScsiRequestParameters scp) {
  ScsiCmd cmd;
  u_int16 i,dir;
  s_int16 st;
  //  u_int16 *p = buf;
  int    status;
  int    bytes;
  int    sensePhase;

  dir = 0;
  sensePhase = 0;

sense:

  cmd.opcode = opcode;
  memset(scsiCMD,0,16);
  
  //printf(" SCSI%02x ",opcode);  
  switch (opcode) {
  case SCSI_WRITE_10:
  case SCSI_READ_10:
    cmd.oplen  = 10;
    if (opcode == SCSI_READ_10)
      dir = SCSI_DIR_IN;    
    {
      u_int16 lo = (u_int16)scp.u.lba;
      u_int16 hi = scp.u.lba>>16;
      scsiCMD[8]  = (hi>>8);
      scsiCMD[9]  = ((hi&0xFF)<<8)|(lo>>8);
      scsiCMD[10] = ((lo&0xFF)<<8)|0x0000;
      scsiCMD[11] = len>>9; // Number of blocks
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
    debug_printf2("Unknown scsi opcode %X\n",opcode);
    return S_ERROR;
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
  scsiCMD[6] = dir|lun;
  scsiCMD[7] = (cmd.oplen<<8)|cmd.opcode;

  //  printf("SREAD LUN %d cmd %04x %04x\n",lun,scsiCMD[7],scsiCMD[8]);


  // CMD write 
  // if STALL at cmd phase -> Unknown Error
  if (UsbWriteToEndpoint(epout, scsiCMD, USB_SCSI_CMD_LEN, 0) == S_ERROR) {
    return S_ERROR;
  }


  // Should we read or write data
  if (len && dir == SCSI_DIR_IN) {
    bytes = UsbReadFromEndpoint(epin, buf, len);
    if (bytes == S_ERROR)
      return S_ERROR;
  } else if (len) {
    bytes = UsbWriteToEndpoint(epout, buf, len,0);
    if (bytes == S_ERROR)
      return S_ERROR;    
  }

  // Stall at data phase
  if (bytes == E_STALL) {
    if (dir)
      status = usb_bulk_only_reset_recovery(&usbDevInfo,EP_IN);
    else
      status = usb_bulk_only_reset_recovery(&usbDevInfo,EP_OUT);    
    // Clear endpoint failure
    if (status < 0) {
      return S_ERROR;
    }
  }

  // Get scsi command status wrapper
  status = UsbReadFromEndpoint(epin, scsiCSW,SCSI_CSW_LEN);

  // Stall at csw phase
  if (status == E_STALL) {
    status = usb_bulk_only_reset_recovery(&usbDevInfo,EP_IN);
    if (status < 0) {
      return S_ERROR;
    }
    status = UsbReadFromEndpoint(epin, scsiCSW,SCSI_CSW_LEN);
  }

  // Check if CSW is valid and meaningful
  // signature error
  if (scsiCSW[0] != 0x5553 && scsiCSW[1] != 0x4253) {
    debug_printf3("Wrong signature %x%x\n",scsiCSW[0],scsiCSW[1]);    
    return E_AGAIN;
  }
  
  // tag error 
  if (SwapWord(scsiCSW[2]) != tag) {
    debug_printf3("Wrong tag %d != %d\n",scsiCSW[2],tag);
    return E_AGAIN;
  }

  {    
    u_int32 residue = ((u_int32)SwapWord(scsiCSW[5])<<16)|SwapWord(scsiCSW[4]);
    u_int16 status = scsiCSW[6]>>8;
    if (residue != 0) {
      debug_printf3("RESIDUE %ld, status %d\n",residue,status);
    }
      
    // Command failed
    if (status == 1) {
      if (sensePhase == 0) {
	// Try get sense data
	len = 18;
	opcode = SCSI_REQUEST_SENSE;
	sensePhase = 1;
	//CheckSOF(1);
	goto sense;
      } else {
	// Sense failed, Umass reset recovery procedure
	status = usb_bulk_only_reset_recovery(&usbDevInfo,0);
	if (status == S_OK)
	  return E_AGAIN;
	else
	  return S_ERROR;
      }
    } else if (status == 2) {
      // Phase errorUmass reset recovery procedure      
      debug_printf1("RESET USB BULK SCSI\n");
      status = usb_bulk_only_reset_recovery(&usbDevInfo,0);
      if (status == S_OK)
	return E_AGAIN;
      else
	return S_ERROR;
    }
  }

  if (sensePhase == 1) {
    u_int16 sense_key    = getbyte(2,buffer);
    u_int16 sense_subkey = buffer[6];

    debug_printf4("SENSE KEY %02X, sense subkey %04X %s\n",sense_key,sense_subkey,sk_table[sense_key]);
    switch (sense_key) {
    case SK_NO_SENSE:      
    case SK_RECOVERED_ERROR:
    case SK_NOT_READY:
    case SK_UNIT_ATTENTION:
      return E_AGAIN;
    }
    
    // Is sense_key permanenent or transient    
    return S_ERROR;
  }
  
  // Check residue (should be zero if amount >= len)
  //debug_printf("Successfully done %d\n",bytes);
  return S_OK;
}


// umass reset recovery 
ioresult usb_bulk_only_reset_recovery(UsbDeviceInfo *udi,int flag) {
  s_int16 err,status;
  
  if (!flag) {
    // Mass storage reset
    status = UsbWriteToEndpoint(&udi->ep[EP_0], "\p\x21\xff\x00\x00\x00\x00\x00\x00", 8, 1);
    status |= UsbReadFromEndpoint(&udi->ep[EP_0], buffer, 1);
    if (status != S_OK)
      return S_ERROR;
  }
  
  {
    EpInfo *ep0      = OpenEndpoint(udi, EP_0);    
    if (!flag || flag == EP_IN) {
      memcpy(buffer,"\p\x02\x01\x00\x00\x00\x00\x00\x00",8);
      setbyte(buffer, 4 , 0x80|udi->ep[EP_IN].num);
      status  = UsbWriteToEndpoint(ep0, buffer, 8, 1);
      status |= UsbReadFromEndpoint(ep0, buffer, 1);
    
      udi->ep[EP_IN].dev_toggle  = 0;
      udi->ep[EP_IN].host_toggle  = 0;
      
      if (status != S_OK)
	return S_ERROR;

    }
    
    if (!flag || flag == EP_OUT) {    
      memcpy(buffer,"\p\x02\x01\x00\x00\x00\x00\x00\x00",8);
      setbyte(buffer, 4 , udi->ep[EP_OUT].num);    
      status  = UsbWriteToEndpoint(ep0, buffer, 8, 1);
      status |= UsbReadFromEndpoint(ep0, buffer, 1);
      udi->ep[EP_OUT].dev_toggle  = 0;
      udi->ep[EP_OUT].host_toggle  = 0;

      if (status != S_OK)
	return S_ERROR;
    }
  } 
  
  
  return S_OK;
}
		
