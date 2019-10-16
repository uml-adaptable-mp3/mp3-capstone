#ifndef KERNEL_H
#define KERNEL_H

// Prototypes for importing exported kernel symbols
#include <vo_gpio.h>
#include <volink.h>
#include <mutex.h>
#include <vo_fat.h>

#include <vs1005h_import.h>

/* Symbols offered by VSOS 3.50 but not VS1005h ROM */
DLLIMPORT(VoFatNameFromDir)
DLLIMPORT(Wait)
DLLIMPORT(Signal)
DLLIMPORT(ZPC)
DLLIMPORT(CoarseSine)
DLLIMPORT(GetDivider)
DLLIMPORT(timeCountAdd)
/* Synonym to mp3CrcTable */
DLLIMPORT(crc_table)
DLLIMPORT(crc32Tab)
DLLIMPORT(appIXYStart)

ioresult FatOpenEntry(register __i0 VO_FILE *f, DirectoryEntry *de);
char *FatNameFromDir(const u_int16 *dirEntry, char *longName);
ioresult SetNextApp(const char *appFileName, const char *parameters);
s_int16 CoarseSine(u_int16 ph);

#endif
