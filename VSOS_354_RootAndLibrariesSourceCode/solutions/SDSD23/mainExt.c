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
#include <kernel.h>

#if 0
u_int16 sd_sysdevice = 'D'-'A'; //Default drive letter
void *original_device_pointer;
DEVICE sdcard;
#endif

#if 0
// Library or device driver main code
ioresult main(char *parameters) {
  return S_OK;
}
#endif


// Library finalization code. This is called when a call to DropLibrary
// causes the reference count to reach 0. Fini() is called and all memory
// is released after the call.
#if 0
void fini(void) {

}
#endif
