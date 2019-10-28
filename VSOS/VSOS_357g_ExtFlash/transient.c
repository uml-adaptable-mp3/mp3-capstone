#include <vo_stdio.h>
#include <vsos.h>
#include "extSymbols.h"
#include "transient.h"
#include "power.h"
#include "vo_fat.h"
#include <mutex.h>
#include <taskandstack.h>
#include <cyclic.h>
#include <clockspeed.h>
#include <dct.h>
#include <wma.h>
#include <hwLocks.h>
#include <sysmemory.h>

ioresult FatOpenEntry(register __i0 VO_FILE *f, DirectoryEntry *de);
char *FatNameFromDirEntry(const u_int16 *dirEntry, char *longName);
ioresult SetNextApp(const char *appFileName, const char *parameters);
void *LoadLibraryP(const char *filename, void *parameters);
void ZeroPtrCall(register __i1 u_int16 lr0, register __i0 u_int16 i0);
s_int16 ISine(u_int16 ph);
extern const u_int16 crc_table[256];
extern __mem_y s_int32 lpcsin[2048+1];
auto u_int16 ReadFuseMulti(register __a0 u_int16 loBit, /* no interrupt... */
                           register __a1 u_int16 bits); /*   ...protection */
auto u_int16 CalcFuseCRC(void);

#include "encMMisc.h"
#include "encMBits.h"
#include "encMHuff.h"
#include "encMDct.h"
#include "encMWindow.h"
extern const s_int16 __mem_y encMWinCoeffI[];

#include "encVTables.h"
#include "encVOggCrc.h"
#include "encVMisc.h"
#include "encVMode.h"

#include <parseFileParam.h>

#include <font16a.h>

#include <vsNand.h>
extern struct FsNandPhys fsNandPhys;

#include <ctype.h>

#include <romfont1005e.h>

#include <string.h>

#include <audio.h>
#include <usblowlib.h>

/* ROM: player.c */
extern struct FsPhysical *ph;

/* sysRtos/exec.c */
extern __near struct LIST readyQueue;
extern __near struct LIST timerQueue;
extern __near struct LIST waitQueue;

#include <strings.h>

#include <swap.h>

#include <extSymbols.h>

/* sysRtos/allocy.c */
extern struct LIST xFlist;
extern __mem_y struct LISTY yFlist;
extern const u_int16 __mem_y hwLockOffset[HW_LOCK_GROUPS+1];
extern const u_int16 __mem_y hwLockN[HW_LOCK_GROUPS];

/* Vorbis encoder */
extern const s_int16 evBands08k_64[];
extern const s_int16 evMean08k_64[];
extern const s_int16 evBands08k_256[];
extern const s_int16 evMean08k_256[];
extern const s_int16 evBands16k_128[];
extern const s_int16 evMean16k_128[];
extern const s_int16 evBands16k_512[];
extern const s_int16 evMean16k_512[];
extern const s_int16 evBands32k_128[];
extern const s_int16 evMean32k_128[];
extern const s_int16 evBands32k_1024[];
extern const s_int16 evMean32k_1024[];
extern const s_int16 evBands44k_128[];
extern const s_int16 evMean44k_128[];
extern const s_int16 evBands44k_1024[];
extern const s_int16 evMean44k_1024[];
extern const s_int16 evBands48k_256[];
extern const s_int16 evMean48k_256[];
extern const s_int16 evBands48k_1024[];
extern const s_int16 evMean48k_1024[];

void TransientAddKernelSymbols(void) {

	TransientAddKernelSymbolsAsm();
	// DLLIMPORTs for all these symbols are in kernel.h
	
	SymbolAdd("_ResetSdCard", (void *)1, (int)ResetSdCard);
	SymbolAdd("_FatOpenEntry", (void *)1, (int)FatOpenEntry);
	SymbolAdd("_FatNameFromDir", (void *)1, (int)FatNameFromDirEntry);
	SymbolAdd("_InitMutexN", (void *)1, (int)InitMutexN);
	
	SymbolAdd("_ObtainMutex", (void *)1, (int)ObtainMutex);
	SymbolAdd("_AttemptMutex", (void *)1, (int)AttemptMutex);
	SymbolAdd("_ReleaseMutex", (void *)1, (int)ReleaseMutex);

	SymbolAdd("_SetNextApp", (void *)1, (int)SetNextApp);
	SymbolAdd("_LoadLibraryP", (void *)1, (int)LoadLibraryP);
	SymbolAdd("_CreateTaskAndStack", (void *)1, (int)CreateTaskAndStack); //in transient segment
	SymbolAdd("_AddCyclic", (void *)1, (int)AddCyclic);
	SymbolAdd("_DropCyclic", (void *)1, (int)DropCyclic);

	SymbolAdd("_SymbolDeleteLib", (void *)1, (int)SymbolDeleteLib);
	SymbolAdd("_SymbolDelete", (void *)1, (int)SymbolDelete);
	SymbolAdd("_SymbolFindByCrc", (void *)1, (int)SymbolFindByCrc);
	SymbolAdd("_SymbolCalcCrc32String", (void *)1, (int)SymbolCalcCrc32String);

	SymbolAdd("_Wait",(void *)1, (int)Wait);
	SymbolAdd("_Signal",(void *)1, (int)Signal);
	SymbolAdd("_ZPC",(void *)1, (int)ZeroPtrCall);
	SymbolAdd("_CoarseSine",(void *)1, (int)ISine);
	SymbolAdd("_clockSpeed",(void *)1, (int)(&clockSpeed));
	SymbolAdd("_GetDivider",(void *)1, (int)GetDivider);
	SymbolAdd("_timeCountAdd",(void *)1, (int)(&timeCountAdd));
	SymbolAdd("_thisTask",(void *)1, (int)(&thisTask));
	SymbolAdd("_appIXYStart",(void *)1, (int)(appIXYStart));

	/* For compatibility with VS1005h */
	SymbolAdd("_AddTask",(void *)1, (int)(&AddTask));
	SymbolAdd("_atoi",(void *)1, (int)(&atoi));
	SymbolAdd("_BootFromX", (void *)1, (int)BootFromX);
	SymbolAdd("_CalcClockSpeed", (void *)1, (int)CalcClockSpeed);
	SymbolAdd("_crc_table",(void *)1, (int)(&crc_table));
	SymbolAdd("_dct", (void *)1, (int)dct);
	SymbolAdd("_Disable", (void *)1, (int)Disable);
	SymbolAdd("_Enable", (void *)1, (int)Enable);
	SymbolAdd("_encDiv256", (void *)1, (int)&encDiv256);
	SymbolAdd("_EncMAlignBitsToByte", (void *)1, (int)EncMAlignBitsToByte);
	SymbolAdd("_encMButrCSCAI", (void *)1, (int)&encMButrCSCAI);
	SymbolAdd("_EncMCountHuff2", (void *)1, (int)EncMCountHuff2);
	SymbolAdd("_EncMCountHuff4", (void *)1, (int)EncMCountHuff4);
	SymbolAdd("_EncMDct18", (void *)1, (int)EncMDct18);
	SymbolAdd("_EncMDct32", (void *)1, (int)EncMDct32);
	SymbolAdd("_EncMGetBitPtr", (void *)1, (int)EncMGetBitPtr);
	SymbolAdd("_EncMHuff2N", (void *)1, (int)EncMHuff2N);
	SymbolAdd("_EncMHuff4", (void *)1, (int)EncMHuff4);
	SymbolAdd("_encMHybridWinFuncI", (void *)1, (int)&encMHybridWinFuncI);
	SymbolAdd("_EncMInitBits", (void *)1, (int)EncMInitBits);
	SymbolAdd("_EncMInitDct", (void *)1, (int)EncMInitDct);
	SymbolAdd("_EncMInsertBits", (void *)1, (int)EncMInsertBits);
	SymbolAdd("_EncMMoveBitBytes", (void *)1, (int)EncMMoveBitBytes);
	SymbolAdd("_EncMOutBits16", (void *)1, (int)EncMOutBits16);
	SymbolAdd("_encMPow34", (void *)1, (int)&encMPow34);
	SymbolAdd("_EncMSetBitBytePtr", (void *)1, (int)EncMSetBitBytePtr);
	SymbolAdd("_encMWinCoeffI", (void *)1, (int)&encMWinCoeffI);
	SymbolAdd("_EncMWriteBitBytes", (void *)1, (int)EncMWriteBitBytes);
	SymbolAdd("_encVDbRatio", (void *)1, (int)&encVDbRatio);
	SymbolAdd("_encVFloorInv", (void *)1, (int)&encVFloorInv);
	SymbolAdd("_encVOggCrc", (void *)1, (int)&encVOggCrc);
	SymbolAdd("_encVOnePer", (void *)1, (int)&encVOnePer);
	SymbolAdd("_EncVSampleClear", (void *)1, (int)EncVSampleClear);
	SymbolAdd("_EncVShiftUpAndCopyToSamp", (void *)1, (int)EncVShiftUpAndCopyToSamp);
	SymbolAdd("_evAudioMode", (void *)1, (int)&evAudioMode); /* Should check if these have changed... */
	SymbolAdd("_extSymbolSearchRom", (void *)1, (int)&extSymbolSearchRom);
	SymbolAdd("_extSymbol", (void *)1, (int)&extSymbol);
	SymbolAdd("_extSymbolRom", (void *)1, (int)0xFFF0); /* Address doesn't matter because size is 0, just see to it that it isn't NULL. */
	SymbolAdd("_extSymbolRomSize", (void *)1, (int)&extSymbolRomSize);
	SymbolAdd("_FileParamInt", (void *)1, (int)FileParamInt);
	SymbolAdd("_FLOOR1_fromdB_LOOKUP_i", (void *)1, (int)&FLOOR1_fromdB_LOOKUP_i);
	SymbolAdd("_font16a", (void *)1, (int)&font16a);
	SymbolAdd("_font16aptrs", (void *)1, (int)&font16aptrs);
	SymbolAdd("_fsNandPhys", (void *)1, (int)&fsNandPhys);
	SymbolAdd("_FsPhNandCreate", (void *)1, (int)FsPhNandCreate);
	SymbolAdd("_GetI6", (void *)1, (int)GetI6);
	SymbolAdd("_GetSizeNodeI", (void *)1, (int)GetSizeNodeI);
	SymbolAdd("_HeadNode", (void *)1, (int)HeadNode);
	SymbolAdd("_HeadNodeI", (void *)1, (int)HeadNodeI);
	SymbolAdd("_HeadNodeY", (void *)1, (int)HeadNodeY);
	SymbolAdd("_huffEnc2", (void *)1, (int)&huffEnc2);
	SymbolAdd("_hwLockOffset", (void *)1, (int)&hwLockOffset);
	SymbolAdd("_hwLockN", (void *)1, (int)&hwLockN);
	SymbolAdd("_hwLocks", (void *)1, (int)&hwLocks);
	SymbolAdd("_isalnum", (void *)1, (int)isalnum);
	SymbolAdd("_isalpha", (void *)1, (int)isalpha);
	SymbolAdd("_iscntrl", (void *)1, (int)iscntrl);
	SymbolAdd("_isdigit", (void *)1, (int)isdigit);
	SymbolAdd("_isgraph", (void *)1, (int)isgraph);
	SymbolAdd("_islower", (void *)1, (int)islower);
	SymbolAdd("_isprint", (void *)1, (int)isprint);
	SymbolAdd("_ispunct", (void *)1, (int)ispunct);
	SymbolAdd("_isspace", (void *)1, (int)isspace);
	SymbolAdd("_isupper", (void *)1, (int)isupper);
	SymbolAdd("_isxdigit", (void *)1, (int)isxdigit);
	SymbolAdd("_latin1", (void *)1, (int)&latin1);
	SymbolAdd("_MegaFatGetByte", (void *)1, (int)MegaFatGetByte);
	SymbolAdd("_MegaFatGetLong", (void *)1, (int)MegaFatGetLong);
	SymbolAdd("_MegaFatGetWord", (void *)1, (int)MegaFatGetWord);
	SymbolAdd("_memcmp", (void *)1, (int)memcmp);
	SymbolAdd("_MemCopyPackedBigEndian", (void *)1, (int)MemCopyPackedBigEndian);
	SymbolAdd("_memcpy", (void *)1, (int)memcpy);
	SymbolAdd("_MemCpySampleT", (void *)1, (int)MemCpySampleT);
	SymbolAdd("_memcpyXY", (void *)1, (int)memcpyXY);
	SymbolAdd("_memcpyYX", (void *)1, (int)memcpyYX);
	SymbolAdd("_memcpyYY", (void *)1, (int)memcpyYY);
	SymbolAdd("_memmove", (void *)1, (int)memmove);
	SymbolAdd("_memset", (void *)1, (int)memset);
	SymbolAdd("_memsetY", (void *)1, (int)memsetY);
	SymbolAdd("__modf", (void *)1, (int)_modf);
	SymbolAdd("_NandGetOctets", (void *)1, (int)NandGetOctets);
	SymbolAdd("_NandGetStatus", (void *)1, (int)NandGetStatus);
	SymbolAdd("_NandPutAddressOctet", (void *)1, (int)NandPutAddressOctet);
	SymbolAdd("_NandPutCommand", (void *)1, (int)NandPutCommand);
	SymbolAdd("_NandPutOctets", (void *)1, (int)NandPutOctets);
	SymbolAdd("_NandWaitIdle", (void *)1, (int)NandWaitIdle);
	SymbolAdd("_NextNode", (void *)1, (int)NextNode);
	SymbolAdd("_NextNodeI", (void *)1, (int)NextNodeI);
	SymbolAdd("_NextNodeY", (void *)1, (int)NextNodeY);
	//	SymbolAdd("_nf", (void *)1, (int)nf);
	SymbolAdd("_oldClockX", (void *)1, (int)&oldClockX);
	SymbolAdd("_oldExtClock4KHz", (void *)1, (int)&oldExtClock4KHz);
	SymbolAdd("_ph", (void *)1, (int)&ph);
	SymbolAdd("_random", (void *)1, (int)random);
	SymbolAdd("_ReadIMem", (void *)1, (int)ReadIMem);
	SymbolAdd("_readyQueue", (void *)1, (int)&readyQueue);
	SymbolAdd("_SpiSendReceive", (void *)1, (int)SpiSendReceive);
	SymbolAdd("_sprintf", (void *)1, (int)sprintf);
	SymbolAdd("_srandom", (void *)1, (int)srandom);
	SymbolAdd("_strcat", (void *)1, (int)strcat);
	SymbolAdd("_strchr", (void *)1, (int)strchr);
	SymbolAdd("_strcpy", (void *)1, (int)strcpy);
	SymbolAdd("_strlen", (void *)1, (int)strlen);
	SymbolAdd("_strncasecmp", (void *)1, (int)strncasecmp);
	SymbolAdd("_strncat", (void *)1, (int)strncat);
	SymbolAdd("_strncmp", (void *)1, (int)strncmp);
	SymbolAdd("_strncpy", (void *)1, (int)strncpy);
	SymbolAdd("_strrchr", (void *)1, (int)strrchr);
	SymbolAdd("_strspn", (void *)1, (int)strspn);
	SymbolAdd("_strstr", (void *)1, (int)strstr);
	SymbolAdd("_strtol", (void *)1, (int)strtol);
	SymbolAdd("_Swap16", (void *)1, (int)Swap16);
	SymbolAdd("_Swap32", (void *)1, (int)Swap32);
	SymbolAdd("_Swap32Mix", (void *)1, (int)Swap32Mix);
	SymbolAdd("_SymbolCrunchStringCalcCrc32", (void *)1, (int)SymbolCrunchStringCalcCrc32);
	SymbolAdd("_SymbolFindRomByCrc", (void *)1, (int)CommonOkResultFunction);
	SymbolAdd("_timeCount", (void *)1, (int)&timeCount);
	SymbolAdd("_timerQueue", (void *)1, (int)&timerQueue);
	SymbolAdd("_tolower", (void *)1, (int)tolower);
	SymbolAdd("_toupper", (void *)1, (int)toupper);
	SymbolAdd("_UartDivider", (void *)1, (int)UartDivider);
	SymbolAdd("_VoFatClusterPos", (void *)1, (int)VoFatClusterPos);
	SymbolAdd("_VoFatReadClusterRecord", (void *)1, (int)VoFatReadClusterRecord);
	SymbolAdd("_vo_fprintf", (void *)1, (int)vo_fprintf);
	SymbolAdd("_volumeReg", (void *)1, (int)&volumeReg);
	SymbolAdd("_vo_printf", (void *)1, (int)vo_printf);
	SymbolAdd("_waitQueue", (void *)1, (int)&waitQueue);
	SymbolAdd("_winPtr", (void *)1, (int)&winPtr);
	SymbolAdd("_WriteIMem", (void *)1, (int)WriteIMem);
	SymbolAdd("_xFlist", (void *)1, (int)&xFlist);
	SymbolAdd("_XpFifoRead", (void *)1, (int)XpFifoRead);
	SymbolAdd("_XpFifoWrite", (void *)1, (int)XpFifoWrite);
	SymbolAdd("_yFlist", (void *)1, (int)&yFlist);
	SymbolAdd("_CountBitsLong", (void *)1, (int)CountBitsLong);

	SymbolAdd("___malloc", (void *)1, (int)__malloc);
	SymbolAdd("___calloc", (void *)1, (int)__calloc);
	SymbolAdd("___realloc", (void *)1, (int)__realloc);
	SymbolAdd("___free", (void *)1, (int)__free);
	SymbolAdd("___mallocy", (void *)1, (int)__mallocy);
	SymbolAdd("___callocy", (void *)1, (int)__callocy);
	SymbolAdd("___reallocy", (void *)1, (int)__reallocy);
	SymbolAdd("___freey", (void *)1, (int)__freey);
	SymbolAdd("_memchr", (void *)1, (int)memchr);
	SymbolAdd("_HuffmanMask", (void *)1, (int)&HuffmanMask);
	SymbolAdd("_HuffmanRLC16Mono", (void *)1, (int)&HuffmanRLC16Mono);
	SymbolAdd("_HuffmanRLC44ODiff", (void *)1, (int)&HuffmanRLC44ODiff);
	SymbolAdd("_i_twiddle_iv", (void *)1, (int)&i_twiddle_iv);
	SymbolAdd("_lpcsin", (void *)1, (int)&lpcsin);

	SymbolAdd("_evBands08k_64", (void *)1, (int)evBands08k_64);
	SymbolAdd("_evMean08k_64", (void *)1, (int)evMean08k_64);
	SymbolAdd("_evBands08k_256", (void *)1, (int)evBands08k_256);
	SymbolAdd("_evMean08k_256", (void *)1, (int)evMean08k_256);
	SymbolAdd("_evBands16k_128", (void *)1, (int)evBands16k_128);
	SymbolAdd("_evMean16k_128", (void *)1, (int)evMean16k_128);
	SymbolAdd("_evBands16k_512", (void *)1, (int)evBands16k_512);
	SymbolAdd("_evMean16k_512", (void *)1, (int)evMean16k_512);
	SymbolAdd("_evBands32k_128", (void *)1, (int)evBands32k_128);
	SymbolAdd("_evMean32k_128", (void *)1, (int)evMean32k_128);
	SymbolAdd("_evBands32k_1024", (void *)1, (int)evBands32k_1024);
	SymbolAdd("_evMean32k_1024", (void *)1, (int)evMean32k_1024);
	SymbolAdd("_evBands44k_128", (void *)1, (int)evBands44k_128);
	SymbolAdd("_evMean44k_128", (void *)1, (int)evMean44k_128);
	SymbolAdd("_evBands44k_1024", (void *)1, (int)evBands44k_1024);
	SymbolAdd("_evMean44k_1024", (void *)1, (int)evMean44k_1024);
	SymbolAdd("_evBands48k_256", (void *)1, (int)evBands48k_256);
	SymbolAdd("_evMean48k_256", (void *)1, (int)evMean48k_256);
	SymbolAdd("_evBands48k_1024", (void *)1, (int)evBands48k_1024);
	SymbolAdd("_evMean48k_1024", (void *)1, (int)evMean48k_1024);
	SymbolAdd("_ReadFuseMulti", (void *)1, (int)ReadFuseMulti);
	SymbolAdd("_CalcFuseCRC", (void *)1, (int)CalcFuseCRC);
	SymbolAdd("_RingBufCopyXfromY", (void *)1, (int)RingBufCopyXfromY);

	//SymbolAdd("_", (void *)1, (int) );
}
