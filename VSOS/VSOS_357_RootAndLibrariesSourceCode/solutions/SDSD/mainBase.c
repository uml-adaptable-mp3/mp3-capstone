/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// DL3 files require VSOS3 kernel version 0.3x to run.

// If you add the libary name to S:CONFIG.TXT, and put the library 
// to S:SYS, it will be loaded and run during boot-up. Use 8.3 file names.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <devSdSd.h>
#include <ctype.h>
#include <kernel.h>

s_int16 __mem_y sd_sysdevice = -1; //Default drive letter
void *original_device_pointer = NULL;
SD_DEVICE sdCard = {0};

ioresult main(char *parameters);

ioresult init(const char *parameters) {
  int newDevice = 'D'-'A';

  while (*parameters) {
    char c = toupper(*parameters);
    if (c>='A' && c<='Z') {
      newDevice = c-'A';
    }      
    parameters++;
  }

  DevSdSdCreate((DEVICE *)(&sdCard), NULL, 0); /* Always succeeds */

  sd_sysdevice = newDevice;
  SysReport("%c: %s", 'A'+sd_sysdevice, DevSdSdIdentify((DEVICE *)(&sdCard), NULL, 0));
  original_device_pointer = vo_pdevices[sd_sysdevice];
  vo_pdevices[sd_sysdevice] = (DEVICE *)(&sdCard);
}

// Library or device driver main code
#if 0
ioresult main(char *parameters) {
  return S_OK;
}
#endif


// Library finalization code. This is called when a call to DropLibrary
// causes the reference count to reach 0. Fini() is called and all memory
// is released after the call.
void fini(void) {
  printf("SDSD: Close device \'%c\'\n", sd_sysdevice+'A');
  vo_pdevices[sd_sysdevice] = original_device_pointer;
  DevSdSdDelete((DEVICE *)(&sdCard));
}
