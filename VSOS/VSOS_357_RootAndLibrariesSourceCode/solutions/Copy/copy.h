#ifndef __COPY_H__
#define __COPY_H__

#include <vo_stdio.h>
#include <vsos.h>

ioresult CopyFile(const char *destination, const char *source);
ioresult CopyRecVerbose(const char *destination, const char *source, FILE *fp);
ioresult CopyRec(const char *destination, const char *source);

#endif /* !COPY_H */
