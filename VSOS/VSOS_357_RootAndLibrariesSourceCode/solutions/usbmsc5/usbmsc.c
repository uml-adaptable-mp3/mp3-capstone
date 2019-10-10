/// \file usbmsc.c A decently size-optimized USB MSC handler for VSOS/VS1005
/// Implements USB Chapter 9, MSC, SCSI, Ramdisk and remote code execution for VS1005
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy 2016

#define VS1005

#include <vo_stdio.h>
#ifdef VS1005
#include <vs1005h.h>
#include <consolestate.h>
#else
#include <vs1010b.h>
#endif
#include <string.h>
#include <usblowlib.h>
#include <clockspeed.h>
#include <vo_gpio.h>
#include "usbinit.h"
#include <volink.h>
#include <vsos.h>

#ifdef VS1005
__mem_y u_int16 diskSurfaceY[8192];
#endif

DEVICE *usbDisk = NULL;
u_int16 blockSizeShift = 0;

#ifdef ENDPOINT_SIZE_MAX
#undef ENDPOINT_SIZE_MAX
#endif
#define ENDPOINT_SIZE_MAX 64

#define USB_MEM(a) USEY(a)
#define PREFIX_SETUP (1<<14)

typedef struct usbpkt2 {
	s_int16 length;
	u_int16 ep;
	u_int16 prefix;
	union {
		u_int16 data[(ENDPOINT_SIZE_MAX+1)>>1];
		struct {
			u_int16 operation;
			u_int16 value;
			u_int16 index;
			u_int16 swappedLength;
		} setup;
	} d;
} USBPacket2;
USBPacket2 usp;


auto void RingBufCopyXfromY(register __i2 u_int16 *d, register __i0 __mem_y const u_int16 *s, register __a0 u_int16 n);
	
struct UsbSender usbsender;

USB_MEM_TYPE u_int16 * const usb_xmit_bufptr[4] ={
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM),
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM+64),
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM+128),
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM+192)
};


auto void SetAddress(void) {
	PERIP(USB_CF) |= (usp.d.setup.value >> 8) & 0x7f;
}
auto void ResetDataToggles(void) {
	u_int16 u = PERIP(USB_CF);
	PERIP(USB_CF) = u | USB_CF_DDTOG;
	PERIP(USB_CF) = u;
}
#pragma msg 139 off // temporarily remove warning "int converted to ptr"

__mem_y u_int16 csw[7];
__mem_y u_int16 capacity[4];
const __mem_y u_int16 tenzeros[10] = {0};
const __mem_y void* const setAddress[] = {SetAddress}; //Zero length packet side-effect function
const __mem_y void* const resetDataToggles[] = {ResetDataToggles}; //Zero length packet side-effect function
const __mem_y u_int16 deviceDescriptor[] = { 
  //Device Descriptor	
  0x1201,   // bLength | bDescriptorType
  0x1001,   // bcd USB Version (01.10) Low | high
  0x0000,   // bDeviceClass:    Defined in Interface Descriptor
  0x0040,   // bDeviceProtocol: -''-, Endpoint 0 size 64
  0xfb19,   // idVendorL VLSI 0x19fb
  0x0205,   // idProductL VS1000: 0x00.0x02 (0x02=MassStorage)
  0x0000,   // bcdDeviceL
  0x0102,   // iManufacturerString
  0x0301    // SerialNumber
};

const __mem_y u_int16 configurationDescriptor[] = {
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
  // Endpoint Descriptor 1 of 2, data IN to PC from our device    
  0x0705,   //Length, Endpoint descriptor
  0x8202,   //EP 2 IN, Bulk
  0x4000,   //Lo(Maximum packet size (64 bytes))
  0x0007,   //Polling Interval (0=Not Applicable for bulk endpoints)                  
  // Endpoint Descriptor 2 of 2, data OUT from PC to our device  
  0x0503,   //Endpoint descriptor, EP 3 OUT
  0x0240,   //This is a bulk endpoint
  0x0000,   //Hi(-''-)  
};
const __mem_y u_int16 languageDescriptor[] = {
  0x0403,0x0904, //UNICODE English
};
#define VENDOR_NAME_LENGTH 4
const __mem_y u_int16 vendorStringDescriptor[] = {
  ((VENDOR_NAME_LENGTH * 2 + 2)<<8) | 0x03,
  'V'<<8,'L'<<8,'S'<<8,'I'<<8};
#define MODEL_NAME_LENGTH 6
const __mem_y u_int16 productStringDescriptor[] = {
  ((MODEL_NAME_LENGTH * 2 + 2)<<8) | 0x03,
  'V'<<8,'S'<<8,'1'<<8,'0'<<8,'0'<<8,'5'<<8};
#define SERIAL_NUMBER_LENGTH 12
const __mem_y u_int16 serialNumberDescriptor[] = {
  ((SERIAL_NUMBER_LENGTH * 2 + 2)<<8) | 0x03,
  '0'<<8, // You should put your own serial number here
  '0'<<8,'0'<<8,'0'<<8,'0'<<8,'0'<<8,
  '0'<<8,'0'<<8,'0'<<8,'0'<<8,'0'<<8,
  '1'<<8, // Serial number should be unique for each unit
};
const __mem_y void* const chapter9[] = { //USB chapter 9 protocol
	6, 1, 0x0005, 0, 0, setAddress, //Set address (zero reply, special)
	7, 2, 0x8006, 0x0001,  0, 18, deviceDescriptor, //Get device descriptor
	7, 2, 0x8006, 0x0002,  0, 32, configurationDescriptor, //Get configuration descriptor
	7, 2, 0x8006, 0x0003,  0, 4, languageDescriptor,
	7, 2, 0x8006, 0x0103,  0, 10, vendorStringDescriptor,
	7, 2, 0x8006, 0x0203,  0, 14, productStringDescriptor,
	7, 2, 0x8006, 0x0303,  0, 26, serialNumberDescriptor,
	7, 2, 0x0009, 0x0100, 0, 0, resetDataToggles, //Set configuration 1 (zero reply)
	7, 2, 0x0201, 0x0000, 0, 0, resetDataToggles, //Clear endpoint halt (zero reply)
	6, 1, 0xa1fe, 0, 1, tenzeros, //get max lun
	0
};

// ------- SCSI replies -------
const __mem_y u_int16 inquiry1[18] = { //-----this must be first SCSI packet-----
	128, 1, 31<<8, 0, ('V'<<8)|'L',('S'<<8)|'I', 0, 0,
	('V'<<8)|'S',('1'<<8)|'0',('0'<<8)|'5',('H'<<8), 0, 0, 0, 0,
	('1'<<8)|'.',('4'<<8)|' '};
const __mem_y u_int16 sense[9] = {0x7000, 0}; //no_sense
__mem_y u_int16 capacity[4] = {0, 31, 0x0000, 512}; //Read Capacity
__mem_y u_int16 atapiCapacities[6] = {0, 8, 0, 31, 0x0200, 512}; //ATAPI read format capacities
const __mem_y u_int16 diskData[1] = {0};
const __mem_y u_int16 csw1[7] = {0x5553, 0x4253, 0, 0, 0, 0, 0}; //-----this must be after all SCSI packets-----

const __mem_y void* const scsi[] = {
	6, 1, 0x0612, 2, 36, inquiry1, //inquiry
	6, 1, 0x0c12, 2, 36, inquiry1, //inquiry
	6, 1, 0x0c03, 2, 18, sense, //request sense
	6, 1, 0x0a25, 2, 8, capacity, //read capacity
	6, 1, 0x0a28, 2, -1, diskData, //read 10
	6, 1, 0x0a2a, 2, -2, csw, //write 10
	6, 1, 0x0a23, 2, 12, atapiCapacities, //ATAPI read format capacities
	6, 1, 0x061a, 2, 4, tenzeros,
	6, 1, 0x0a5a, 2, 4, tenzeros,
	5, 0, 2, 13, csw, //send csw ok to all unknown commands - don't worry, be happy.
	0 
};





const __mem_y u_int16 diskData0Y[] = {
/* 0-1 */ 0xeb34,
/* 2-3 */ 0x9000|'M',
/* 4-5 */ ('S'<<8)|'D',
/* 6-7 */ ('O'<<8)|'S',
/* 8-9 */ ('4'<<8)|'.',
/* 10-11 */ '0'<<8|0x00,
/* 11-13 */ 0x0201, //bytespersector | sectorspercluster
/* 14-15 */ 0x0100, //reserved sectors
/* 16-17 */ 0x0110, //numfats | nrootentlo
/* 18-19 */ 0x0040, //nrootenthi | totsec16lo
/* 20-21 */ 0x00f8, //totsec16hi|media
/* 22-23 */ 0x0100, //fat size
/* 24-25 */ 0x4000, //sectors per track;
/* 26-27 */ 0x0100, //number of tracks;
/* 28-29 */ 0x0000, //HiddSec32
/* 30-31 */ 0x0000, //Hiddsec32
/* 32-33 */ 0x0000, //0x0a00; //totsec32;
/* 34-35 */ 0x0000, //totsec32;
/* 36-37 */ 0x0000, //logicaldrivenum | reserved
/* 38-39 */ 0x2801, //extendedsig | sernum
/* 40-41 */ 0x0203, //
/* 42-43 */ 0x0400|'V',
/* 44-45 */ 'L'<<8|'S',
/* 46-47 */ 'I'<<8|'_',
/* 48-49 */ 'B'<<8|'O',
/* 50-51 */ 'O'<<8|'T',
/* 52-53 */ '_'<<8|'5',
/* 54-55 */ 'F'<<8|'A',
/* 56-57 */ 'T'<<8|'1',
/* 58-59 */ '2'<<8|' ',
/* 60-61 */ ' '<<8|' '
};
const __mem_y u_int16 diskData1Y[] = {0x55AA, 0xf8ff, 0xffff, 0x0f00};
/*
const __mem_y u_int16 diskData2Y[] = {
	0x5653,0x3130,0x3130,0x425f,0x5241,0x4d08,0x0000,0x0000,0x0000,0x0000,0x0000,0x284c,0x7b47,0x0000,0x0000,0x0000,
	0x4252,0x0061,0x006d,0x0064,0x0069,0x000f,0x0031,0x7300,0x6b00,0x0000,0xffff,0xffff,0xffff,0x0000,0xffff,0xffff,
	0x0156,0x0053,0x0031,0x0030,0x0030,0x000f,0x0031,0x3500,0x4800,0x2000,0x4200,0x6f00,0x6f00,0x0000,0x7400,0x2000,
	0x5653,0x3130,0x3130,0x4231,0x2020,0x2020,0x00bc,0x3d4c,0x7b47,0x7b47,0x0000,0x7049,0x7b47,0x0000,0x0000,0x0000,
	0x5653,0x494e,0x464f,0x2020,0x5458,0x5420,0x003c,0x444c,0x7b47,0x7b47,0x0000,0xe249,0x7b47,0x0200,0x0200,0x0000
};
*/
const __mem_y u_int16 diskData2Y[] = {
	0x5653,0x3130,0x3130,0x425f,0x5241,0x4d08,0x0000,0x0000,0x0000,0x0000,0x0000,0x4c92,0x9647,0x0000,0x0000,0x0000,
	0x4252,0x0061,0x006d,0x0064,0x0069,0x000f,0x003d,0x7300,0x6b00,0x0000,0xffff,0xffff,0xffff,0x0000,0xffff,0xffff,
	0x0156,0x0053,0x0031,0x0030,0x0031,0x000f,0x003d,0x3000,0x4200,0x2000,0x4200,0x6f00,0x6f00,0x0000,0x7400,0x2000,
	0x5653,0x3130,0x3130,0x7e31,0x2020,0x2020,0x00bc,0x3d4c,0x7b47,0x7b47,0x0000,0x7049,0x7b47,0x0000,0x0000,0x0000,
	0x5653,0x494e,0x464f,0x2020,0x5458,0x5420,0x003c,0x444c,0x7b47,0x7b47,0x0000,0xe249,0x7b47,0x0200,0x0200,0x0000
};

const __mem_y u_int16 diskData3Y[] = {0x3548};
const __mem_y void* const diskInitTab[] = { // Block copy operations to initialize disk surface
	&diskSurfaceY[0*256], diskData0Y, sizeof(diskData0Y),
	&diskSurfaceY[1*256-1], diskData1Y, sizeof(diskData1Y),
	&diskSurfaceY[2*256], diskData2Y, sizeof(diskData2Y),
	&diskSurfaceY[3*256], diskData3Y, sizeof(diskData3Y),
	csw, csw1, sizeof(csw),
	//capacity, capacity1, sizeof(capacity),	
	0,0,0
};




#pragma msg 139 on
u_int16 usbuf[256];

void GetSector(){

	if (usbDisk) {
		usbDisk->BlockRead(usbDisk, usbsender.lba, 1, usbuf);
		memcpyXY(diskSurfaceY, usbuf, 256);
		usbsender.buf = &diskSurfaceY[0];
	} else {
		usbsender.buf = &diskSurfaceY[(u_int16)usbsender.lba*256]; //Read disk data
	}
	usbsender.lba++;
	usbsender.bytes = 512;
	usbsender.readSectors--;

}


void PutSector() {

	if (usbDisk) {
		usbDisk->BlockWrite(usbDisk, usbsender.lba, 1, usbuf);
	} else {
		if ((usbuf[0] == (('v'<<8) | ('B'))) && (usbuf[1] == (('0' << 8) | ('t')))) {
			BootFromX(&usbuf[3]); //Execute BootFromX record (max. 512-6 bytes)
		}
		memcpyXY(&diskSurfaceY[usbsender.lba*256],usbuf,256);
	}
	
	usbsender.lba++;
	usbsender.writeSectors--;
	usbsender.diskDataPtr = &usbuf[0];
	if (usbsender.writeSectors) {
		usbsender.expectedDiskPackets = 8;
	} else {
		usbsender.buf = csw;
		usbsender.bytes = 13;
	}
}


s_int16 GetUsbPacket() {
	u_int16 tmp,lenW;	
	u_int16 rdptr = PERIP(USB_RDPTR);	
	if ((PERIP(USB_ST) & USB_ST_BRST)) { //Handle bus reset
		int cnt=0;
		while ((PERIP(USB_UTMIR) & 0xC000) == 0) {
		  	if (++cnt >= 32) {
				//printf(" ### BUS RESET ### \n");
				ResetUsbHardware();
				return -2;
			}
		}
		PERIP(USB_ST) = USB_ST_BRST;	
	}
	if (rdptr == PERIP(USB_WRPTR)) return -1; //No new packets in buffer
	usp.prefix = usp.length = USB_MEM(rdptr + USB_RECV_MEM);
	usp.ep = (s_int16)(usp.length & 0x3C00) >> 10;
	usp.length &= 0x03FF;
	lenW = (usp.length + 1) >> 1;
	tmp = (rdptr + 1) & 0x03FF;
	if (lenW <= (ENDPOINT_SIZE_MAX+1)>>1) {
		if (usbsender.expectedDiskPackets && (usp.ep == 3)) {
			RingBufCopyXfromY(usbsender.diskDataPtr, (USB_MEM_TYPE void *)(tmp + USB_RECV_MEM), lenW);
			usbsender.diskDataPtr += lenW;
			usbsender.expectedDiskPackets--;
			usp.length = -1;
			if (usbsender.expectedDiskPackets == 0) {
				PutSector();
			}
		} else {
			RingBufCopyXfromY(usp.d.data, (USB_MEM_TYPE void *)(tmp + USB_RECV_MEM), lenW);
		}
		PERIP(USB_RDPTR) = (lenW + tmp) & 0x03FF;
	}
	PERIP(USB_ST) = (USB_ST_RX /*| SETUP_INFO*/);
	return usp.length;
}

void HandlePacket(register __mem_y u_int16 *p, register u_int16 *data) { //Protocol, Data
	__mem_y u_int16 *np = p;
	u_int16 *d = data;
	u_int16 len = 32767;
	static u_int16 swappedLba[2];
	u_int32 lba;
	if (p==(__mem_y void*)chapter9) { //for chapter 9 replies return only requested amount
		len = usp.d.setup.swappedLength >> 8;
	}

	MemCopyPackedBigEndian(swappedLba, 0, &usp.d.data[8], 1, 4);
	lba = ((u_int32)swappedLba[0] << 16) | (swappedLba[1]);

	while (*p) {
		s_int16 n;
		np = p + *p;
		p++;			
		n = *p++;
		while (n--) {
			if (*d++ != *p++) goto next_record;
		}
		usbsender.ep = *p++;
		n = *p++;
		if (n == -1) { //read disk data
			usbsender.lba = lba<<blockSizeShift;
			usbsender.bytes = 0;
			usbsender.readSectors = usp.d.data[11]<<blockSizeShift;
			return;			
		}
		if (n == -2) { //write disk data
			usbsender.lba = lba<<blockSizeShift;
			usbsender.writeSectors = usp.d.data[11]<<blockSizeShift;
			usbsender.expectedDiskPackets = 8;
			//usbsender.writePtrY = &diskSurfaceY[lba*256];
			return;
		}
		if (n > len) n = len;
		usbsender.bytes = n;
		usbsender.buf = (void*)*p;

		return; 
		next_record: 
		d = data;
		p = np;
	}
	//Should stall now, but let's not need to
}


void UsbSender() {
	u_int16 n;
	u_int16 w;
	u_int16 EP_STATUS = USB_EP_ST0 + usbsender.ep;

	if (!usbsender.bytes && usbsender.readSectors) {
		GetSector();
	}
	
	if (!usbsender.buf || (!(PERIP(EP_STATUS) & USB_EP_ST_IXMIT_EMP))) return;
	//We have bufptr and ep is not busy

	n = usbsender.bytes;
	if (n>ENDPOINT_SIZE_MAX) n = ENDPOINT_SIZE_MAX;
	usbsender.bytes -= n;
	w = ((n+1) >> 1);

	
	memcpyYY(usb_xmit_bufptr[usbsender.ep], usbsender.buf, w);
	usbsender.buf += w;

	PERIP(USB_EP_SEND0 + usbsender.ep) = (USB_EP_SEND_TXR | (usbsender.ep << USB_EP_SEND_ADDR_B) | n);
	if (!n) { //Handle zero length packet side-effects such as SetAddress
		auto void (*Function)(void) = (void*)(usbsender.buf[0]);
		while (!(PERIP(EP_STATUS) & USB_EP_ST_IXMIT_EMP));
		//printf("F%04x",(u_int16)Function);
		Function();
	}

	if (usbsender.bytes == 0) { //for any EP2 (MSC IN; Device->PC) data, autoappend csw
		if ((usbsender.ep == 2) && (usbsender.readSectors == 0) && (usbsender.buf != &csw[sizeof(csw)])) {
			usbsender.buf = csw;
			usbsender.bytes = 13;
		} else {
			usbsender.buf = NULL;
		}		
	}	
}



void UsbMainLoop(void) {
	static DiskGeometry g;	
	

	InitUsbHardware();
	ResetUsbHardware();

	memsetY(diskSurfaceY,0,sizeof(diskSurfaceY));
	{ 	// Init disk surface
		__mem_y u_int16 *p = (void*)&diskInitTab[0];
		while (*p) {			
			u_int16 d,s,w;
			d = *p++;
			s = *p++;
			w = (u_int16)*p++;
			memcpyYY((__mem_y void*)d, (__mem_y void*)s, w);			
		}
	}
		
	if (usbDisk) {
		ioctl(usbDisk, IOCTL_GET_GEOMETRY, (void*)(&g));
		while (g.sectorsPerBlock > 1) {
			blockSizeShift++;
			g.sectorsPerBlock >>= 1;
		}

		capacity[0] = atapiCapacities[2] = g.totalSectors >> 16;
		capacity[1] = atapiCapacities[3] = g.totalSectors & 0xffff;
		capacity[3] = atapiCapacities[5] = 512 << blockSizeShift;
		printf("USB publishing disk %s.\n",usbDisk->Identify(usbDisk,NULL,0));
	} else {
		printf("No disk, USB publishing ramdisk.\n");
	}
	printf("[Ctrl-C] to exit.\n");

	while (!(appFlags & APP_FLAG_QUIT)) {

		if (GetUsbPacket() >= 0) {
			if (usp.prefix & PREFIX_SETUP) {
				HandlePacket((__mem_y void*)chapter9, usp.d.data); //Setup packet -> handle with chapter 9 protocol
			} else if ((usp.ep == 3) && (usp.d.data[0] == 0x5553)) { //MSC out (PC->Device) endpoint && SCSI_REQUEST
					csw[2] = usp.d.data[2]; //Get copy of transaction tag
					csw[3] = usp.d.data[3];	
					HandlePacket((__mem_y void*)scsi, &usp.d.data[7]); //It's a SCSI protocol packet
			}
		}
		UsbSender();
	}
}


