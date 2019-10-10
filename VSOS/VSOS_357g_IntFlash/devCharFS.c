/// \file devCharFS.c Provide a common filesystem to access character devices as files.
/// This filesystem can be used to provide a simple access to character drvices as if they were
/// normal files.
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy


#include <string.h>
#include "devCharFS.h"
#include "vsos.h"

///Define the port parameter in fileInfo
#define __FI_PORT 0	
#define __FI_MODE1 1	
#define __FI_MODE2 2	
#define __FI_NAME1 3

char *CharacterDeviceFSIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
	return "DeviceFS";
}
	
//ioresult (*Create)(register __i0 DEVICE *dev, char *name, u_int16 *sectorBuffer); ///< Try to start the filesystem for device

ioresult CharacterDeviceFSCreate(register __i0 DEVICE *dev, char *name, u_int16 *sectorBuffer){	
	if (__F_CHARACTER_DEVICE(dev)) {
		return S_OK;
	}
	return S_ERROR;
}

/// This function will convey the call for the file's Open method
/// to the device driver as an Ioctl. 
/// As a convenienve for the device driver developer, the first character of the file name 
/// is copied into the fileinfo struct, as well as two first characters of the mode parameter.
/// Note that the object pointer for the IOCTL_OPEN_DEVICE_FILE is a pointer to the FILE
/// instead of a pointer to the DEVICE.
/// The purpose of this IOCTL is to allow the device driver to set the file's I/O PORT
/// parameter so that the device driver can easily differentiate between different files for input,output etc.
ioresult CharacterDeviceFileOpen(register __i0 VO_FILE *f, const char *name, const char *mode){ ///< Find and open a file
	f->fileInfo[__FI_NAME1] = name[0]; //First character of file name
	f->fileInfo[__FI_MODE1] = mode[0]; //First character of file mode
	f->fileInfo[__FI_MODE2] = mode[1]; //Second character of file mode
	printf("CharDevFileOpen to file %04x device %04x ioctl %04x\n",f,f->dev,f->dev->Ioctl);
	return f->dev->Ioctl((void*)f, IOCTL_OPEN_DEVICE_FILE, name); //To allow device driver to prepare file stream
} 

/// Convey a call to the file's Close method to the device driver.
ioresult	CharacterDeviceFileClose (register __i0 VO_FILE *f){ ///< Flush and close file
	return f->dev->Ioctl((void*)f, IOCTL_CLOSE_DEVICE_FILE, 0); //To allow device driver to close file stream
}

u_int16 CharacterDeviceFileRead(register __i0 VO_FILE *f, void *buf, u_int16 destinationIndex, u_int16 bytes){ ///< Read bytes to *buf, at byte index
	u_int16 bytesTransferred;
	if (!f->dev->Read) {
		return 0;
	}
	bytesTransferred = f->dev->Read(f->dev, buf, destinationIndex, bytes);
	f->pos += bytesTransferred;
	return bytesTransferred;
}

// Convert calls to the file's Write method into one or more calls to the device driver's Output method.
u_int16 CharacterDeviceFileWrite(register __i0 VO_FILE *f, void *vbuf, u_int16 sourceIndex, u_int16 bytes){ ///< Write bytes from *buf at byte index
	u_int16 bytesTransferred;
	if (!f->dev->Write) {
		return 0;
	}
	bytesTransferred = f->dev->Write(f->dev, vbuf, sourceIndex, bytes);
	f->pos += bytesTransferred;
	return bytesTransferred;
}


ioresult CharacterDeviceFileIoctl(register __i0 VO_FILE *f, s_int16 request, char *argp){ ///< For extensions, not used.
	//printf("CharacterDeviceIoctl Relay IOCTL %d to %04x\n",request,f->dev->Ioctl);
	return f->dev->Ioctl((void*)f, request, argp);
}

const FILEOPS characterDeviceFileOperations = {
	CharacterDeviceFileOpen, //ioresult (*Open) (register __i0 VO_FILE *f, const char *name, const char *mode); ///< Find and open a file
	CharacterDeviceFileClose, //ioresult (*Close)(register __i0 VO_FILE *f); ///< Flush and close file
	CharacterDeviceFileIoctl,//ioresult (*Ioctl)(register __i0 VO_FILE *f, s_int16 request, char *argp); ///< For extensions, not used.
	CharacterDeviceFileRead, //u_int16  (*Read) (register __i0 VO_FILE *f, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	CharacterDeviceFileWrite, //u_int16  (*Write)(register __i0 VO_FILE *f, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
};


const FILESYSTEM characterDeviceFS = {
	__MASK_PRESENT |__MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE, //u_int16  flags; //< present, initialized,...
	CharacterDeviceFSIdentify, //char* (*Identify)(void *obj, char *buf, u_int16 bufsize);
	CharacterDeviceFSCreate, //DevConsoleCreate, // ioresult (*Create)(DEVICE *dev, char *name);
	0,//CommonOkResultFunction, //ioresult (*Delete)(DEVICE *dev);
	0,//CommonOkResultFunction, //ioresult (*Ioctl) (DEVICE *dev);
	&characterDeviceFileOperations, //FILEOPS  *op;	// Opened file inherits this set of file operations.
	//0,//&FatDirOps, //DIROPS   *dir; // Directory operations, if the filesystem has directories		
};
