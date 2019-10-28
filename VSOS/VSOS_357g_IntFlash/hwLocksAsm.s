/** \file hwLocksAsm.s Release hardware lock associated with XPERIP. */

#include <vs1005g.h>
#include <hwLocks.h>


#if 0
	.import _GetHwLocksBIP

	.sect code,ObtainHwLocksBIP
	.export _ObtainHwLocksBIP
_ObtainHwLocksBIP:
	j _GetHwLocksBIP
	ldc 1,a0	// waitForLocks


	.sect code,AttemptHwLocksBIP
	.export _AttemptHwLocksBIP
_AttemptHwLocksBIP:
	j _GetHwLocksBIP
	ldc 0,a0	// waitForLocks
#endif


/*
	s_int16 MsbSetU32(register __reg_a u_int32 n);
	Returns -1 if no bits are set, otherwise the number of the
	highest bit that is set.
*/
	.sect code,MSBSetU32
	.export _MSBSetU32
_MSBSetU32:
	and d,d,d
	ldc 0,d2
	jrzs
	add null,ones,a0
	
	exp d,a0
	ldc 38,d0
	jr
	sub d0,a0,a0









	.end
