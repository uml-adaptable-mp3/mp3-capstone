#ifndef SIMPLE_DEVICE
#define SIMPLE_DEVICE struct simple_device_descriptor

struct simple_device_descriptor { // must have CHARACTER DEVICE flag set, should not have filesystem.
	vo_flags   flags; ///< VSOS Flags 
	const char*    (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	ioresult (*Open)(register __i0 SIMPLE_FILE *self, const char *name, const char *mode); ///< Start device, populate descriptor, find and start filesystem.
	ioresult (*Close)(register __i0 SIMPLE_FILE *self); ///< Flush and close file
	IOCTL_RESULT (*Ioctl)(register __i0 SIMPLE_FILE *self, s_int16 request, IOCTL_ARGUMENT arg); ///< For extensions, used e.g. for special operations on device files.
	u_int16  (*Read) (register __i0 SIMPLE_FILE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	u_int16  (*Write)(register __i0 SIMPLE_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
	ioresult (*BlockRead)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Read block at LBA
	ioresult (*BlockWrite)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Write block at LBA
	FILESYSTEM *fs; ///< pointer to the filesystem driver for this device
	u_int16  deviceInstance; ///< identifier to detect change of SD card etc
};

#endif