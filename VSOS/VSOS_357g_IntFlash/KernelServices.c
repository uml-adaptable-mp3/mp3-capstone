#include <vstypes.h>
#include <vsos.h>
#include <vs1005g.h>
#include <stdarg.h>
#include <usb.h>
#include <usblowlib.h>
#include <kernelservices.h>
#include <mapper.h>
#include <scsi.h>
#include <clockspeed.h>

extern struct FsMapper *map;

ioresult VoMassStorage() {
	//printf("VoMassStorage\n");	
	//printf("Map:%p, version:%04x\n",map,map->version);
	//printf("blocksize:%d, blocks:%ld, cacheBlocks:%d\n",map->blockSize,map->blocks,map->cacheBlocks);
	
	//printf("ScsiReset=%p\n",ScsiReset);
	//printf("ScsiState=%p\n",ScsiState);
	//printf("ScsiTaskHandler=%p\n",ScsiTaskHandler);

	ScsiReset();
	PowerSetVoltages(&voltages[voltCoreUSB]);
	BusyWait10();
	SetClockSpeed(SET_CLOCK_USB);
	InitUSB(1);
	while(1) {
		USBHandler();
		if (USBWantsSuspend()) {
			if (USBIsDetached()) {
				break;
			}
			//Suspend requested
		}
	}
	printf("End of msc\n");
	PERIP(USB_CF) = 0x8000U;
	PERIP(ANA_CF2) &= ~ANA_CF2_UTM_ENA;		
	return 123;
}



auto ioresult KernelService (u_int16 service,...){
	u_int16 i;
	va_list ap;
	va_start(ap, service);
	/*
	printf("hop\n");
	i = va_arg(ap, u_int16); printf("vararg:%d\n",i);	
	i = va_arg(ap, u_int16); printf("vararg:%d\n",i);	
	i = va_arg(ap, u_int16); printf("vararg:%d\n",i);	
	i = va_arg(ap, u_int16); printf("vararg:%d\n",i);	
	i = va_arg(ap, u_int16); printf("vararg:%d\n",i);	
	*/
	switch (service) {
	case KERNEL_GET_USB_STRUCT_ADDRESS:
		return &USB;
		break;
	case KERNEL_USB_MASS_STORAGE:
		map = va_arg(ap, void*);
		return VoMassStorage();
		break;
	case KERNEL_SET_FOPEN_RETRIES:
		fopen_retries = va_arg(ap, u_int16);
		break;						
	default:
		return S_ERROR;
	}
}