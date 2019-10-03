///  \file kernel_abs.s Fixed memory locations for VSOS application API. 
/// If you modify this file, you must MANUALLY edit vsos02.abs file 
/// so that it corresponds with this file (all symbols have correct addresses)
/// NOTE: VSOS02.ABS HAS ALSO OTHER SYMBOLS (rom hooks) that don't come from this file.

	.sect code, ZeroPtrCall
	.import _ZeroPtrCall
	.org 0
	mv lr0,i1
	jmpi _ZeroPtrCall
	
	
	.sect code,kernelVectors
	

	.import _ioctl //_ioctl=32714:FCN/VOID
	.import _SetHandler//=32715:FCN/VOID
	.import _GpioSetAsPeripheral//=32716:FCN/VOID
	.import _GpioSetAsInput//=32717:FCN/VOID
	.import _GpioReadPinDelay//=32718:FCN/VOID
	.import _GpioSetPin//=32719:FCN/VOID

	.import _ObtainHwLocksBIP//=32711:FCN/VOID
	.import _AttemptHwLocksBIP//=32712:FCN/VOID
	.import _ReleaseHwLocksBIP//=32713:FCN/VOID


	.export  _ResourceLoadHook
	.org 32710
_ResourceLoadHook: jmpi _CommonOkResultFunction//_VOHandleResourceLoad
	
	jmpi _ObtainHwLocksBIP//=32711:FCN/VOID
	jmpi _AttemptHwLocksBIP//=32712:FCN/VOID
	jmpi _ReleaseHwLocksBIP//=32713:FCN/VOID

	jmpi _ioctl //_ioctl=32714:FCN/VOID
	jmpi _SetHandler//=32715:FCN/VOID
	jmpi _GpioSetAsPeripheral//=32716:FCN/VOID
	jmpi _GpioSetAsInput//=32717:FCN/VOID
	jmpi _GpioReadPinDelay//=32718:FCN/VOID
	jmpi _GpioSetPin//=32719:FCN/VOID


	.import _RenderStdButton, _CreateStdButton, _GetStdButtonPress
	.import _Forbid, _Permit, _Yield, _StartTask, _Delay
	.import _CodMpgCreate, _OpenStdAudioOut, _CloseStdAudioOut
	.import _KernelCodMpgCreate
	.import _RemovedInterface
	.import _AutoVoidNull
	.import _LoadLibrary, _DropLibrary
	.import _SystemUiMessageReceiver
	.import _VOGetSymbolAddress, _SymbolAdd
	.export _GetSymbolAddress, _AddSymbol

	.org 0x7fd0
	jmpi _Forbid //7fd0 32720
	jmpi _Permit //7fd1
	jmpi _Yield //7fd2
	jmpi _StartTask //7fd3
	jmpi _Delay //7fd4
	jmpi _KernelCodMpgCreate //7fd5
	jmpi _LoadLibrary //jmpi _RemovedInterface //was jmpi _OpenStdAudioOut //7fd6
	jmpi _DropLibrary //jmpi _CommonOkResultFunction // now ModelCallbackHook was jmpi _CloseStdAudioOut //7fd7	
	jmpi _RenderStdButton//=32728:FCN/VOID	 //jmpi _AutoVoidNull // jmpi _RenderStdButton//=32728:FCN/VOID
	jmpi _CreateStdButton//=32729:FCN/VOID //jmpi _AutoVoidNull // jmpi _CreateStdButton//=32729:FCN/VOID
	.org 32730
	jmpi _AutoVoidNull // jmpi _SetVirtualResolution//=32730:FCN/VOID	
	jmpi _GetStdButtonPress//=32731:FCN/VOID //jmpi _AutoVoidNull // jmpi _GetStdButtonPress//=32731:FCN/VOID
	jmpi _AutoVoidNull // jmpi _SetClippingRectangleToButton//=32732:FCN/VOID
	jmpi _SystemUiMessageReceiver// = 32733
	_GetSymbolAddress: jmpi _VOGetSymbolAddress //=32734
	_AddSymbol: jmpi _SymbolAdd //32735 ==END=OF=VECTORS==

	.import _vo_fopen,_vo_fread,_vo_fseek,_vo_fwrite,_vo_ftell
	.import _vo_fflush,_vo_fclose,_DefaultSysError,_DefaultSysReport,_vo_kernel_init
	.export _SysError, _SysReport
	.import _vo_feof,_vo_ferror,_CommonOkResultFunction
	.import _CommonErrorResultFunction,_StartFileSystem
	.import _vo_fgetc,_vo_fgets,_vo_fputc,_vo_fputs,_vo_ungetc
	.import _PrintFmt, _FatFindFirst, _FatFindNext
	.import _GetTouchLocation, ___InitMemAlloc
	.import _KernelService
	.import _ReadTimeCount
	.import _SetUartSpeed, _SetClockSpeed
	.import _DefaultSetPower, _RunLibraryFunction 
	.export _SetPower
	
	.org 0x7fe0
	jmpi _vo_fopen   // 7fe0 
	jmpi _vo_fread	 // 7fe1  
	jmpi _vo_fseek	 // 7fe2 
	jmpi _vo_fwrite	 // 7fe3 
	jmpi _vo_ftell	 // 7fe4 
	jmpi _vo_fflush	 // 7fe5 todo
	jmpi _vo_fclose	 // 7fe6 
_SysError:		jmpi _DefaultSysError	 // 7fe7
_SysReport:	jmpi _DefaultSysReport  // 7fe8 
	jmpi _vo_kernel_init // 7fe9 
	jmpi _vo_feof	 // 7fea 
	jmpi _vo_ferror  // 7feb 
	jmpi _CommonOkResultFunction    // 7fec 
	jmpi _CommonErrorResultFunction // 7fed 
	jmpi _StartFileSystem           // 7fee 
	jmpi _vo_fgetc	// 7fef 
	jmpi _vo_fgets	// 7ff0
	jmpi _vo_fputc	// 7ff1
	jmpi _vo_fputs	// 7ff2
	jmpi _vo_ungetc	// 7ff3
	jmpi _PrintFmt // 7ff4
	jmpi _FatFindFirst //7ff5
	jmpi _FatFindNext //7ff6
	jmpi _CommonOkResultFunction // jmpi _GetTouchLocation //7ff7
	jmpi ___InitMemAlloc //7ff8
	jmpi _ReadTimeCount //7ff9
	jmpi _SetUartSpeed //7ffa
_SetPower:	jmpi _DefaultSetPower //7ffb
	jmpi _SetClockSpeed //7ffc
	jmpi _CommonErrorResultFunction // jmpi _CallKernelModule //7ffd
	jmpi _RunLibraryFunction//jmpi _CommonErrorResultFunction // jmpi _SysCall //7ffe
	.org 0x7fff
	jmpi _AutoVoidNull // jmpi _KernelService // 7fff		

	.sect code,biosVectors
	.org 0x7c
	.import _LcdInit, _LcdFilledRectangle, _LcdTextOutXY, _BiosService	
	
	jmpi _CommonErrorResultFunction // jmpi _LcdInit	 //0x7c
	jmpi _CommonErrorResultFunction // jmpi _LcdFilledRectangle //0x7d
	jmpi _CommonErrorResultFunction // jmpi _LcdTextOutXY	 //0x7e
	jmpi _BiosService	 //0x7f
	 



	.sect data_x,os_if_vars
	.org 0x870
	.import _consoleFile, _vo_files, _vo_sys_error_hook
	.import _vo_sys_report_hook, _vo_get_time_from_rtc_hook
	.import _FatFileSystem, _characterDeviceFS, _sysTasks
	_lcd0: .export _lcd0			
		.bss 16
		.org 0x880
_vo_stdin: .export _vo_stdin
		.word _consoleFile
_vo_stdout: .export _vo_stdout
		.word _consoleFile
_vo_stderr: .export _vo_stderr
		.word _consoleFile
_appFile: .export _appFile
		.bss 1
		
#if 0
_sys_error_hook_ptr: .export _sys_error_hook_ptr
		.word _vo_sys_error_hook
_sys_report_hook_ptr: .export _sys_report_hook_ptr
		.word _vo_sys_report_hook
_get_time_from_rtc_hook_ptr: .export _get_time_from_rtc_hook_ptr
		.word _vo_get_time_from_rtc_hook
#else
		.word 0
		.word 0
		.word _vo_get_time_from_rtc_hook	
#endif	

_lastErrorMessagePtr: .export _lastErrorMessagePtr
		.bss 1
_vo_osMemoryStart: .export _vo_osMemoryStart
		.bss 1
_vo_osMemorySize: .export _vo_osMemorySize
		.bss 1
_vo_max_num_files: .export _vo_max_num_files
		.bss 1
_vo_pfiles: .export _vo_pfiles
		.word _vo_files
_vo_pdevices: .export _vo_pdevices
		.bss 26 
_currentTime: .export _currentTime
		.word 0, 0, 0, 1, 0, 100, 6, 0, 0
_console: .export _console	
		.bss 33		
_vo_filesystems:	.export _vo_filesystems
		.word _FatFileSystem
		.word 0 //	.word _characterDeviceFS
		.word 0 
		.word 0
		.word 0
		.word 0
_pSysTasks: .export _pSysTasks 
		.word _sysTasks
_vo_fat_allocationSizeClusters: .export _vo_fat_allocationSizeClusters
		.word 128
___nextDeviceInstance: .export ___nextDeviceInstance
		.word 1
_stdaudioin: .export _stdaudioin
		.bss 1
_stdaudioout: .export _stdaudioout
		.bss 1			
//		.import _colorScheme, _touchInfo
//_pColors: .export _pColors 
		.word 0 //.word _colorScheme
//_pTouchCalib: .export _pTouchInfo
		.word 0 //.word _touchInfo
_osVersion: .export _osVersion
		.word 355
_loadedLibs: .export _loadedLibs 
		.bss 1
_loadedLib: .export _loadedLib
		.bss 16
_kernelDebugLevel: .export _kernelDebugLevel
		.word 0
_pIFlist:
		.import _iFlist
		.word _iFlist		
_appParameters: .export _appParameters
		.import _bootrc4ks //buffer of 258 words which is not used by ROM after boot
		.word _bootrc4ks
_lastResult: .export _lastResult
		.word 0
_modelCallbacks: .export _modelCallbacks
		.bss 5
		

#if 0
#define YMEMPOOLSIZE 1024
.sect data_y, ymemPool
	.export _ymemPool, _ymemPoolEnd
	.org 0x2000-YMEMPOOLSIZE
_ymemPool:
	.bss YMEMPOOLSIZE
_ymemPoolEnd:
#endif	


		.end
