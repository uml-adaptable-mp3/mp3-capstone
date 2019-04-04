#ifndef __COPY_FILE_H__
#define __COPY_FILE_H__

#include <vo_stdio.h>
#include <vsos.h>

ioresult CopyFile(const char *destination, const char *source);
ioresult CopyFileRecVerbose(const char *destination, const char *source, FILE *fp);
ioresult CopyFileRec(const char *destination, const char *source);

#endif /* !COPY_H */
