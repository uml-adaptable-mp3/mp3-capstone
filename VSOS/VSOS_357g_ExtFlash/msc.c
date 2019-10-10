
#include <vo_stdio.h>
#include <vsos.h>
#include <vs1005g.h>
#include <usb.h>
#include <usblowlib.h>
#include <kernelservices.h>

#include <string.h>
#include <scsi.h>
#include <clockspeed.h>
#include <stdbuttons.h>
#include "devSdSd.h"
#include "msc.h"

extern StdButton mscButtons[];
extern u_int16 last_scsi_command;

DEVICE *usbDisk;
DiskGeometry geometry;
extern struct USBVARS USB;
struct USBVARS *usb = NULL;

void MyInitUSBDescriptors(u_int16 initDescriptors);
u_int16 ScsiIdle;

enum SCSIStageEnum ScsiState(void);


const /*__y*/ u_int16 myDeviceDescriptor_MassStorage[] = { 
  //Device Descriptor	
  0x1201,   // bLength | bDescriptorType
  0x1001,   // bcd USB Version (01.10) Low | high
  0x0000,   // bDeviceClass:    Defined in Interface Descriptor
  0x00|ENDPOINT_SIZE_0,   // bDeviceProtocol: -''-
  0xfb19,   // idVendorL VLSI 0x19fb
  0x02ee,   // idProductL VS1000: 0x00.0x02 (0x02=MassStorage)
  0x0000,   // bcdDeviceL
  0x0102,   // iManufacturerString
  0x0301    // SerialNumber
};

#define USBMASS_CONFDESCSIZE 32

const /*__y*/ u_int16 myConfigurationDescriptor_MassStorage[] = {
  // Prime Configuration Descriptor        
  0x0902,   //Length
  0x2000,   //Lo(Total length of configuration descriptor (32))
  0x0101,   //Number of interfaces supported (at least bulk only)
  0x0080,   //String index to this configuration
  0x8009,   //Power usage: 32 milliamperes | Length
  
  // Interface Descriptor 1 of 1                 
  0x0400,   //Type: Interface Descriptor
  0x0002,   //This is Alternate Setting 0
  0x0806,   //Device Class: Mass Storage (Class 8)
  0x5000,   //Interface Protocol: Bulk Only Transport (0x50)
  
  // Endpoint Descriptor 1 of 2
  // This Endpoint transfers data IN to PC from our device    
  0x0705,   //Length
  ((0x80U|MSC_BULK_IN_ENDPOINT)<<8)|02,
  0x4000,   //Lo(Maximum packet size (64 bytes))
  0x0007,   //Polling Interval (0=Not Applicable for bulk endpoints)                
  
  // Endpoint Descriptor 2 of 2
  // This Endpoint transfers data OUT from PC to our device  
  (0x05U<<8)|MSC_BULK_OUT_ENDPOINT, //Type: Endpoint Descriptor : 
  //Endpoint IN(0x03) 
  0x0240,   //This is a bulk endpoint
  0x0000,   //Hi(-''-)  
};




#define D_SIZE 4 
const /*__y*/ u_int16 myLanguageStr[] = {
  (D_SIZE<<8)|03, //length
  0x0904, //UNICODE English
};
#undef D_SIZE

#define VENDOR_NAME_LENGTH 4
const u_int16 myVendorNameStr[] = {
  ((VENDOR_NAME_LENGTH * 2 + 2)<<8) | 0x03,
  'V'<<8,
  'l'<<8,
  's'<<8,
  'i'<<8};

#define MODEL_NAME_LENGTH 8
const u_int16 myModelNameStr[] = {
  ((MODEL_NAME_LENGTH * 2 + 2)<<8) | 0x03,
  'V'<<8,
  'S'<<8,
  '1'<<8,
  '0'<<8,
  '0'<<8,
  '5'<<8,
  'G'<<8,
  '1'<<8};

#define SERIAL_NUMBER_LENGTH 12
u_int16 mySerialNumberStr[] = {
  ((SERIAL_NUMBER_LENGTH * 2 + 2)<<8) | 0x03,
  '0'<<8, // You should put your own serial number here
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '0'<<8,
  '1'<<8, // Serial number should be unique for each unit
};


u_int16 OkResultFunction(void) {
	return 0;
}


struct FsMapper *UsbMapCreate(struct FsPhysical *physical, u_int16 cacheSize) {
  //printf("CREATE");
  return &usbMapper;
}

s_int16 UsbMapRead(struct FsMapper *map, u_int32 firstBlock, u_int16 blocks, u_int16 *data) {
  ioresult err = usbDisk->BlockRead(usbDisk, firstBlock, blocks, data);
  return blocks;
}

s_int16 UsbMapWrite(struct FsMapper *map, u_int32 firstBlock, u_int16 blocks, u_int16 *data) {
  s_int16 bl = 0;
  static s_int16 dataa[256];

  while (bl < blocks) {           
    // Write firstBlock, *data
    usbDisk->BlockWrite(usbDisk, firstBlock, blocks, data);
    data += 256;
    firstBlock++;
    bl++;
  }
  return bl;
}


s_int16 UsbMapFlush(struct FsMapper *map, u_int16 hard){
	usbDisk->BlockRead(usbDisk, 0, 0, NULL); //Flush
	return 0;	
}


struct FsMapper usbMapper = {
    0x010c, /*version*/
    256,    /*blocksize*/
    1986560,
    0,      /*cacheBlocks*/
    UsbMapCreate,
    OkResultFunction,//RamMapperDelete,
    UsbMapRead,
    UsbMapWrite,
    0,//RamMapperFree,
    UsbMapFlush,//RamMapperFlush,
    NULL /* no physical */
};


#define DT_LANGUAGES 2
#define DT_VENDOR 3
#define DT_MODEL 4
#define DT_SERIAL 5
#define DT_DEVICE 0
#define DT_CONFIGURATION 1

void MyInitUSBDescriptors(u_int16 initDescriptors){
	if (usb) {
	  
	  usb->descriptorTable[DT_LANGUAGES] = myLanguageStr;
	  usb->descriptorTable[DT_VENDOR] = myVendorNameStr;
	  usb->descriptorTable[DT_MODEL]  = myModelNameStr;
	  usb->descriptorTable[DT_SERIAL] = mySerialNumberStr;
	  usb->descriptorTable[DT_DEVICE] = myDeviceDescriptor_MassStorage;
	  usb->descriptorTable[DT_CONFIGURATION] = myConfigurationDescriptor_MassStorage;
	  usb->configurationDescriptorSize = USBMASS_CONFDESCSIZE;

	}
}
 

void MyMSCPacketFromPC(USBPacket *inPacket);
void MyScsiTaskHandler();
void MyUSBHandler();
void MyScsiReset();
void MyInitUSB(u_int16 initDescriptors);
u_int16 MyUSBReadLines(void);
u_int16 MyUSBWantsSuspend(void);
u_int16 MyUSBIsDetached(void);


/*
void Delay1(u_int16 i) {
  volatile u_int16 x;
  for(x=0; x < i; x++) ;
}
*/

#define REGU_CTRL      0xFED0
#define CLK_CTRL       0xFECE
#define CCL_REGU_CLOCK (1<<3)

void PowerSetVoltages2(u_int16 volt[3]) {
    register u_int16 v;

    v = (volt[voltCorePlayer] & 31) |
        ((volt[voltIoPlayer] & 31) << 5) |
        ((volt[voltAnaPlayer] & 31) << 10);
    PERIP(REGU_CTRL) = (PERIP(REGU_CTRL) & 0x8000U) | v;

    /* enable regulator, if not already */
    //USEX(SCI_STATUS) &= ~SCISTF_REGU_SHUTDOWN; -- No, reset clears only!
    PERIP(CLK_CTRL) |=  CCL_REGU_CLOCK;    /* clk up */
    PERIP(CLK_CTRL) &= ~CCL_REGU_CLOCK;    /* clk down */
}


ioresult VoMassStorage(char deviceLetterUppercase) {
  SD_HWINFO* hw=(SD_HWINFO*)usbDisk->hardwareInfo;
  u_int16 i;
  
  usbDisk = vo_pdevices[deviceLetterUppercase-'A'];
  usb = &USB;

  // Get card size before starting SCSI
  // Write size to usbMapper structure (in blocks)
  //	size = ioct(Usbdisk,GETSIZE);
  {
	//u_int32 size = hw->size * 2;
	u_int16 i;
#if 0	
	u_int32 size = (((u_int32)usbDisk->hardwareInfo[3] << 16) + (u_int32)usbDisk->hardwareInfo[2]) * 2;
	printf("S disk driver is %s.\n",usbDisk->Identify(usbDisk,0,0));
	if ((osVersion != 241) || strcmp(usbDisk->Identify(usbDisk,0,0),"SD/SD Card")){
		printf("Cannot get real disk size.\n");
		printf("(This version of USB software only gets real disk size for vsos v0.241 and the SD/SD driver.)\n");
	} else {
		usbMapper.blocks = size;
	}
#else
	printf("USB publishing disk: %s.\n",usbDisk->Identify(usbDisk,0,0));
	ioctl(usbDisk,IOCTL_GET_GEOMETRY,&geometry);
	usbMapper.blocks = geometry.totalSectors;
#endif
	//printf("Sectors: %ld (%ld). ",usbMapper.blocks,geometry.totalSectors);
	printf("Size %0.1f MB.\n",((float)(usbMapper.blocks))/2048); //2048 512-byte sectors per mebibyte.  	 
}

  printf("SCSI START\n");
  MyScsiReset();
  ScsiIdle = 0;
  
  {
    u_int16 cf0,cf1;
    cf0 = PERIP(ANA_CF0);
    cf1 = PERIP(ANA_CF1);  
    SetClockSpeed(SET_CLOCK_USB);
    PERIP(ANA_CF0) = cf0;
    PERIP(ANA_CF1) = cf1;
  }

#if 1
  {
    static u_int16 v[3]= {
      31, IO_3V3, 31 /* 1.8 3.3 3.6 USB */
    };
    PowerSetVoltages2(v);
  }
#endif

#if 0
  PERIP(PWM_PULSE) = 0;
  PERIP(PWM_FRAME) = 0;
#endif

  MyInitUSB(1);

  USEY(UTM_TRIMRES)   = 0x8002;

  PERIP(GPIO2_MODE)  &= ~3;
  PERIP(GPIO2_DDR)   |=  3;
  
  // GPIO2[4] = clk_out
  PERIP(GPIO2_MODE)  |=0x10;
  PERIP(CLK_CF)      |= 0x80;


  while(1) {
    static u_int16 phase = 0;
    

	PERIP(ANA_CF1) |= ANA_CF1_BTNDIS; //Disable power button reset
	
	MyUSBHandler();
	if (PERIP(UART_DATA) == 0x03) { //Ctrl-C
		PERIP(WDOG_CF) = 1;
		PERIP(WDOG_KEY) = 0x4ea9;
	}


#if 0
    if (phase++>60000) {
      if (GetStdButtonPress(mscButtons)) {
	break;
      }
      phase = 0;
    }
#endif

#if 1   
    // ScsiIdle==20 means that PC has ejected us. 
    // We should exit after we have sent CSW to the Allow Medium Removal CMD.
    if (ScsiIdle == 20  && ScsiState() == SCSI_READY_FOR_COMMAND && 
	(PERIP(USB_EP_ST0 + MSC_BULK_IN_ENDPOINT) & USB_STF_IN_EMPTY)) {
      break;
    }
#endif
	{
		char c = PERIP(UART_DATA);
		if ((c == 3) || (c == 239)) { //ctrl-c or monitor connect
			printf("Ctrl-C Reset\n");
			PERIP(WDOG_CF) = 1;
			PERIP(WDOG_KEY) = 0x4ea9;
			while (1) {
				//Wait for watchdog reset
			}
		}
	}

   // if (MyUSBIsDetached()) {
   //     printf("Detached.\n");
   // 	goto finally;
//	}

#if 0

    // If cable is yanked off -> end program
    if (MyUSBWantsSuspend()) {
      volatile u_int16 ii;
      
      printf("\r< WantSuspend %d ",ScsiIdle);

      ii = 0;
      while (ii <= 5000) {
	if (PERIP(USB_STATUS) & 0x0080) {
	  PERIP(USB_STATUS) = 0x0080;
	  //printf(".. RESUME >\n");
	  break;
	}
	Delay1(1500);
	if (++ii > 5000) {
	  goto finally;
	}
      }
    }
#endif      
  }
  


finally:
  PERIP(USB_CF) = 0x8000U;
  PERIP(ANA_CF2) &= ~ANA_CF2_UTM_ENA;		
  return 123;
}

