#ifndef KERNELSERVICES_H
#define KERNELSERVICES_H

#define KERNEL_GET_USB_STRUCT_ADDRESS 1
#define KERNEL_USB_MASS_STORAGE 2
#define KERNEL_SET_FOPEN_RETRIES 3

auto ioresult KernelService (u_int16 service,...);

#endif