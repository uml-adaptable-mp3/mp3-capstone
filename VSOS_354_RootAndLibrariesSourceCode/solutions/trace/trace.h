#ifndef TRACE_H
#define TRACE_H


typedef struct offend_info {
	u_int16 *lib;
	u_int16 page;
	u_int16 address;
	u_int16 segment;
	u_int16 offset;
	u_int32 libfileptr;
	char libname[9];
};

ioresult find_err(register struct offend_info *offend);

#endif
