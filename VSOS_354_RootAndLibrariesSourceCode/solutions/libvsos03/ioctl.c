#include <vsos.h>

auto int ioctl(register void *p, register int request, register void *argp) {
	int res = 0;
	if (!p) {
		return S_ERROR;
	}

	if (__F_FILE((VO_FILE*)p)) {
		VO_FILE *fp = p;
		if (!fp->op || !fp->op->Ioctl) {
			//printf(" error\n");
			return S_ERROR;
		}
		res = fp->op->Ioctl(fp, request, argp);
	} else {
		DEVICE *dp = p;
		if (!dp->Ioctl) {
			return S_ERROR;
		}
		res = dp->Ioctl(dp, request, argp);	
	}
  	
	 
	return res;
}
